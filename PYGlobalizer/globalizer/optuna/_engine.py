"""Ядро: пространство поиска Optuna -> задача Globalizer, метод в отдельном потоке.

Optuna устроена как «спроси-сообщи» (ask/tell): пользователь сам вызывает
целевую функцию. Globalizer, наоборот, сам вызывает функции задачи. Чтобы
совместить эти схемы, метод (PYDGlobalizer.IterativeSolver) работает в
отдельном потоке; функция задачи, которую он вызывает, передаёт точку в
очередь и ждёт ответа:

    поток метода:  f(y) -> askq.put(точка) ... tellq.get() -> значение
    study.ask():   askq.get()  -> Trial с предложенными параметрами
    study.tell():  tellq.put(значение)

IterativeSolver отпускает GIL на время итерации, так что поток метода не
мешает основному. Уже вычисленные точки (кэш исследования) метод получает
сразу, без обращения к пользователю: так перезапуск метода (новое
пространство поиска, меньшая точность) повторяет прежний путь бесплатно.

Кодирование параметров (переменные Globalizer):
  * float                    -> непрерывная на [low, high];
  * float, log=True          -> непрерывная на [ln low, ln high], значение exp(y);
  * float со step, int       -> непрерывная с округлением до узла сетки;
  * int, log=True            -> непрерывная на [ln low, ln(high+1)], значение floor(exp(y));
  * categorical              -> дискретная переменная Globalizer (номер значения),
                                пока число сочетаний категорий не больше
                                max_discrete_combinations; иначе непрерывная
                                с округлением номера;
  * распределение с одним значением -> фиксировано, в поиске не участвует.
"""

import atexit
import math
import os
import queue
import tempfile
import threading

from .distributions import CategoricalDistribution, FloatDistribution, IntDistribution
from ..optimize import _native

_STOP = object()
_active_lock = threading.RLock()
_active_engine = None


class SearchSpace:
    """Отображение вектора Globalizer y <-> параметры испытания."""

    def __init__(self, distributions, max_discrete_combinations=64):
        self.names = list(distributions)
        self.dists = dict(distributions)
        self.fixed = {}
        free = []
        for name in self.names:
            d = self.dists[name]
            if d.single():
                self.fixed[name] = _single_value(d)
            else:
                free.append(name)
        self.free = free

        # Категориальные - дискретными переменными Globalizer, начиная с
        # самых маленьких, пока произведение размеров не превысит лимит.
        cats = sorted((n for n in free if isinstance(self.dists[n], CategoricalDistribution)),
                      key=lambda n: len(self.dists[n].choices))
        self.disc, product = [], 1
        for n in cats:
            k = len(self.dists[n].choices)
            if product * k <= max_discrete_combinations:
                self.disc.append(n)
                product *= k
        disc_set = set(self.disc)
        self.cont = [n for n in free if n not in disc_set]
        self.disc = [n for n in free if n in disc_set]          # порядок объявления

        self.lower, self.upper = [], []
        for n in self.cont:
            lo, hi = _encoded_range(self.dists[n])
            self.lower.append(lo)
            self.upper.append(hi)
        # Ядро не решает задачи только с дискретными переменными.
        self.dummy = not self.cont and bool(self.disc)
        if self.dummy:
            self.lower.append(0.0)
            self.upper.append(1.0)
        self.nc = len(self.lower)
        self.disc_values = [[str(i) for i in range(len(self.dists[n].choices))]
                            for n in self.disc]

    @property
    def empty(self):
        return not self.free

    def to_params(self, y):
        params = dict(self.fixed)
        for i, n in enumerate(self.cont):
            params[n] = _decode(self.dists[n], float(y[i]))
        for j, n in enumerate(self.disc):
            d = self.dists[n]
            idx = min(max(int(round(float(y[self.nc + j]))), 0), len(d.choices) - 1)
            params[n] = d.choices[idx]
        return {n: params[n] for n in self.names}

    def key(self, params):
        return make_key(self.dists, params)

    def center(self):
        return {n: default_value(self.dists[n]) for n in self.names}

    def sample(self, rng):
        params = {}
        for n in self.names:
            d = self.dists[n]
            if isinstance(d, CategoricalDistribution):
                params[n] = d.choices[rng.integers(len(d.choices))]
            else:
                lo, hi = _encoded_range(d)
                params[n] = _decode(d, float(rng.uniform(lo, hi)))
        return params

    def describe(self):
        return (f"{len(self.cont)} continuous + {len(self.disc)} discrete Globalizer variables "
                f"({len(self.fixed)} fixed parameters)")


def encode_value(d, v):
    """Хешируемое значение параметра для ключа кэша (None - значения нет в choices)."""
    if isinstance(d, CategoricalDistribution):
        try:
            return ("c", d.index_of(v))
        except ValueError:
            return None
    return float(v)


def make_key(dists, params):
    """Ключ кэша: значения всех параметров пространства (None, если какого-то нет)."""
    out = []
    for n, d in dists.items():
        if n not in params:
            return None
        v = encode_value(d, params[n])
        if v is None:
            return None
        out.append((n, v))
    return tuple(out)


def _single_value(d):
    if isinstance(d, CategoricalDistribution):
        return d.choices[0]
    if isinstance(d, IntDistribution):
        return int(d.low)
    return float(d.low)


def default_value(d):
    """Значение параметра, для которого у метода ещё нет точки (центр диапазона)."""
    if isinstance(d, CategoricalDistribution):
        return d.choices[0]
    lo, hi = _encoded_range(d)
    return _decode(d, 0.5 * (lo + hi))


def _encoded_range(d):
    if isinstance(d, FloatDistribution):
        if d.log:
            return math.log(d.low), math.log(d.high)
        if d.step is not None:
            return d.low - 0.5 * d.step, d.high + 0.5 * d.step
        return d.low, d.high
    if isinstance(d, IntDistribution):
        if d.log:
            return math.log(d.low), math.log(d.high + 1)
        return d.low - 0.5 * d.step, d.high + 0.5 * d.step
    if isinstance(d, CategoricalDistribution):
        return -0.5, len(d.choices) - 0.5
    raise TypeError(f"Unsupported distribution {d!r}")


def _decode(d, y):
    if isinstance(d, FloatDistribution):
        if d.log:
            return min(max(math.exp(y), d.low), d.high)
        if d.step is not None:
            k = min(max(int(round((y - d.low) / d.step)), 0), d.n_values() - 1)
            # round(): low + k*step без «хвостов» вида 0.30000000000000004
            return float(round(d.low + k * d.step, 12))
        return min(max(y, d.low), d.high)
    if isinstance(d, IntDistribution):
        if d.log:
            return int(min(max(math.floor(math.exp(y)), d.low), d.high))
        k = min(max(int(round((y - d.low) / d.step)), 0), d.n_values() - 1)
        return int(d.low + k * d.step)
    idx = min(max(int(round(y)), 0), len(d.choices) - 1)
    return d.choices[idx]


class Engine:
    """Один запуск индексного метода над фиксированным пространством поиска."""

    def __init__(self, space, n_constraints, sampler, eps, lookup):
        self.space = space
        self.n_constraints = n_constraints
        self.sampler = sampler
        self.eps = eps
        self.lookup = lookup            # key -> (constraints, value) или None
        self.new_points = 0             # точек, отданных пользователю
        self.killed = False             # остановлен извне (не дошёл до конца сам)
        self._stop = False
        self._askq = queue.Queue()
        self._tellq = queue.Queue()
        self._waiting = False
        self._thread = None

    # --- основной поток ----------------------------------------------------
    def start(self):
        global _active_engine
        with _active_lock:
            # В процессе может работать только один решатель Globalizer
            # (глобальные параметры ядра): останавливаем чужой.
            if _active_engine is not None and _active_engine is not self:
                _active_engine.stop()
            _active_engine = self
        self._thread = threading.Thread(target=self._worker, name="globalizer-optuna",
                                        daemon=True)
        self._thread.start()

    @property
    def alive(self):
        return self._thread is not None and self._thread.is_alive() and not self._stop

    def next(self):
        """('point', params, key) | ('finished', None, None) | ('error', exc, None)."""
        return self._askq.get()

    def answer(self, result):
        self._tellq.put(result)

    def stop(self, timeout=30.0):
        global _active_engine
        if self._thread is None:
            return
        if not self._stop:
            self._stop = True
            self.killed = True
            self._tellq.put(_STOP)
        self._thread.join(timeout)
        with _active_lock:
            if _active_engine is self:
                _active_engine = None

    # --- поток метода -------------------------------------------------------
    def _worker(self):
        solver = None
        try:
            native, PYProblem = _native.load()
            sp = self.space
            problem = PYProblem(dimension=sp.nc, numCriterions=1)
            problem.set_bounds(list(sp.lower), list(sp.upper))
            if sp.disc:
                problem.set_discrete_variables([list(v) for v in sp.disc_values],
                                               [f"d_{n}" for n in sp.disc])
            for j in range(self.n_constraints):
                problem.add_function(self._function(j))          # ограничения - первыми
            problem.add_function(self._function(self.n_constraints))   # критерий - последним

            params = native.SolverParameters()
            params.max_iterations = int(self.sampler.max_iterations)
            params.r = self.sampler.r
            params.epsilon = self.eps
            params.num_threads = 1
            params.local_refine = self.sampler.local_search
            params.local_iterations = self.sampler.local_iterations
            # Поле есть только в новых сборках PYDGlobalizer.
            if hasattr(params, "local_refine_mode"):
                params.local_refine_mode = self.sampler.local_refine_mode
            params.verbose = 0
            params.print_result = False
            params.log_prefix = os.path.join(tempfile.gettempdir(), "globalizer_optuna_log")

            solver = native.IterativeSolver(problem, params)
            while not self._stop:
                if solver.do_iteration():
                    break
            self._askq.put(("finished", None, None))
        except BaseException as exc:                # noqa: BLE001 - отдаём в основной поток
            self._askq.put(("error", exc, None))
        finally:
            solver = None

    def _halt(self, j):
        # Остановка: ограничения выполнены, критерий постоянен - метод быстро
        # завершает текущую итерацию, после чего цикл выходит.
        return -1.0 if j < self.n_constraints else 0.0

    def _function(self, j):
        def f(y):
            if self._stop:
                return self._halt(j)
            params = self.space.to_params(y)
            key = self.space.key(params)
            res = self.lookup(key)
            if res is None:
                self.new_points += 1
                self._askq.put(("point", params, key))
                res = self._tellq.get()
                if res is _STOP:
                    return self._halt(j)
            cons, value = res
            if j < self.n_constraints:
                return float(cons[j]) if j < len(cons) else 0.0
            return float(value)
        return f


def _stop_active():
    with _active_lock:
        engine = _active_engine
    if engine is not None:
        engine.stop(timeout=5.0)


atexit.register(_stop_active)
