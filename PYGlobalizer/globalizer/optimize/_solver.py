"""Общее ядро: перевод задачи в формате scipy в PYProblem и запуск Globalizer.

Соответствие понятий:
  * переменные с integrality=True -> дискретные переменные Globalizer
    (идут последними, значения - все целые из [lb, ub]);
  * переменные с lb == ub -> фиксированы и в поиске не участвуют;
  * ограничения -> функции g_j(x) <= 0, идут ПЕРЕД целевой функцией
    (порядок, принятый в ядре Globalizer);
  * результат - лучшая ДОПУСТИМАЯ точка среди всех вычислений целевой
    функции (их отслеживает обёртка), а не только итог метода.

Исключения из пользовательских функций и запрос остановки из callback ядро
Globalizer прервать не умеет (Task::CalculateFuncs превращает исключение в
MaxDouble). Поэтому обёртка запоминает исключение / запрос остановки, перестаёт
вызывать пользовательский код (дальше функции возвращают константу, метод
быстро дорабатывает), а после возврата из решателя поднимает исходное
исключение или формирует результат по уже найденной лучшей точке.
"""

import math
import os
import tempfile
import threading
import warnings

import numpy as np

from ._constraints import normalize_bounds, normalize_constraints
from ._optimize import OptimizeResult, OptimizeWarning
from . import _native

#: Лимит вычислений по умолчанию, если метод scipy не задаёт его явно, и
#: верхняя граница для «бесконечных» лимитов scipy (maxfun=1e7 и т.п.).
#: Каждое испытание Globalizer хранится в памяти, поэтому лимит ограничен.
DEFAULT_MAXFEV = 100_000
_MAXFEV_LIMIT = 10_000_000
#: Допуск для равенств h(x) = 0: индексный метод ищет точки в полосе
#: |h(x)| <= eq_tol; слишком узкую полосу он может не найти вовсе.
DEFAULT_EQ_TOL = 1e-2

_MESSAGES = {
    0: "Optimization terminated successfully.",
    1: "Maximum number of function evaluations has been exceeded.",
    2: "`callback` function requested stop early.",
    3: "No feasible point was found.",
}

#: Ключи словаря options, которые понимает Globalizer.
GLOBALIZER_OPTIONS = ("r", "eps", "maxfev", "local_search", "local_iterations",
                      "eq_tol", "threads", "disp")


def _clamp_maxfev(value):
    value = int(min(max(value, 10), _MAXFEV_LIMIT))
    return value


def resolve_workers(workers):
    """workers scipy -> число потоков Globalizer."""
    if callable(workers):
        warnings.warn("Globalizer does not use a map-like `workers`; "
                      "the problem is solved in one thread.", OptimizeWarning, stacklevel=3)
        return 1
    workers = int(workers)
    if workers == -1:
        return os.cpu_count() or 1
    return max(1, workers)


class _Layout:
    """Отображение вектора Globalizer y <-> вектор пользователя x."""

    def __init__(self, lb, ub, integrality):
        n = lb.size
        if integrality is None:
            integ = np.zeros(n, dtype=bool)
        else:
            integ = np.broadcast_to(np.asarray(integrality, dtype=bool), (n,)).copy()
        self.n = n
        self.template = np.empty(n)
        self.cont, self.disc, self.disc_values = [], [], []
        for i in range(n):
            if integ[i]:
                lo, hi = math.ceil(lb[i]), math.floor(ub[i])
                if lo > hi:
                    raise ValueError(
                        "One of the integrality constraints does not have any possible "
                        "integer values between the lower/upper bounds.")
                if lo == hi:
                    self.template[i] = lo
                else:
                    self.disc.append(i)
                    self.disc_values.append(np.arange(lo, hi + 1, dtype=float))
            elif lb[i] == ub[i]:
                self.template[i] = lb[i]
            else:
                self.cont.append(i)
        self.cont = np.array(self.cont, dtype=int)
        self.disc = np.array(self.disc, dtype=int)
        self.lb, self.ub = lb, ub
        # Ядро не умеет задачи только с дискретными переменными -
        # добавляем фиктивную непрерывную переменную на [0, 1].
        self.dummy = self.cont.size == 0 and self.disc.size > 0
        self.nc = self.cont.size + (1 if self.dummy else 0)

    @property
    def n_free(self):
        return self.cont.size + self.disc.size

    def to_x(self, y):
        x = self.template.copy()
        if self.cont.size:
            x[self.cont] = y[:self.cont.size]
        if self.disc.size:
            x[self.disc] = np.rint(y[self.nc:self.nc + self.disc.size])
        return x

    def center(self):
        x = self.template.copy()
        if self.cont.size:
            x[self.cont] = 0.5 * (self.lb[self.cont] + self.ub[self.cont])
        for i, vals in zip(self.disc, self.disc_values):
            x[i] = vals[len(vals) // 2]
        return x

    def clip(self, x):
        """Приводит точку пользователя (например, x0) к границам и целым."""
        x = np.asarray(x, dtype=float).reshape(-1)
        if x.size != self.n:
            raise ValueError(f"x0 has {x.size} elements, but bounds define {self.n} variables")
        x = np.clip(x, self.lb, self.ub)
        if self.disc.size:
            lo = np.array([v[0] for v in self.disc_values])
            hi = np.array([v[-1] for v in self.disc_values])
            x[self.disc] = np.clip(np.rint(x[self.disc]), lo, hi)
        fixed = np.ones(self.n, dtype=bool)
        fixed[self.cont] = False
        fixed[self.disc] = False
        x[fixed] = self.template[fixed]
        return x


class _Evaluator:
    """Обёртки функций для Globalizer + отслеживание лучшей допустимой точки."""

    def __init__(self, layout, func, args, constraints, vectorized, notify):
        self.layout = layout
        self.func = func
        self.args = args if isinstance(args, tuple) else (args,)
        self.constraints = constraints
        self.vectorized = vectorized
        self.notify = notify
        self.nfev = 0
        self.best_x = None
        self.best_f = np.inf
        self.error = None
        self.stopped = False
        self._lock = threading.RLock()
        # Диапазон уже полученных конечных значений. Из него строятся
        # «штрафные» значения для nan/inf и константа после остановки:
        # огромные числа (1e300, MaxDouble) переполняют характеристики метода
        # (исключение "Infinite R!" в ядре).
        self._f_lo = np.inf
        self._f_hi = -np.inf
        self._g_hi = 0.0

    # --- пользовательские вызовы -------------------------------------------
    def call_func(self, x):
        if self.vectorized:
            f = self.func(x[:, None], *self.args)
        else:
            f = self.func(x, *self.args)
        with self._lock:
            self.nfev += 1
        f = np.asarray(f, dtype=float)
        if f.size != 1:
            raise RuntimeError("The user-provided objective function must return a scalar value.")
        return float(f.reshape(-1)[0])

    def is_feasible(self, x):
        return all(c.value(x) <= 0.0 for c in self.constraints)

    def max_violation(self, x):
        if not self.constraints:
            return 0.0
        return float(max(c.violation(x) for c in self.constraints))

    def _fail(self, exc):
        with self._lock:
            if self.error is None:
                self.error = exc

    @property
    def halted(self):
        return self.error is not None or self.stopped

    def _objective_penalty(self):
        if self._f_hi < self._f_lo:
            return 1.0
        return self._f_hi + (self._f_hi - self._f_lo) + 1.0

    def _halt_objective(self):
        return self._f_lo if np.isfinite(self._f_lo) else 0.0

    # --- функции, которые вызывает Globalizer ------------------------------
    def make_constraint(self, c):
        def g(y):
            if self.halted:
                return 1.0                         # константа: метод быстро доработает
            try:
                v = float(c.value(self.layout.to_x(np.asarray(y, dtype=float))))
            except BaseException as exc:          # noqa: BLE001 - вернём пользователю
                self._fail(exc)
                return 1.0
            if not np.isfinite(v):
                return 2.0 * self._g_hi + 1.0
            if v > self._g_hi:
                self._g_hi = v
            return v
        return g

    def objective(self, y):
        if self.halted:
            return self._halt_objective()
        x = self.layout.to_x(np.asarray(y, dtype=float))
        try:
            f = self.call_func(x)
            feasible = self.is_feasible(x) if self.constraints else True
        except BaseException as exc:              # noqa: BLE001
            self._fail(exc)
            return self._halt_objective()
        if not np.isfinite(f):
            return self._objective_penalty()
        with self._lock:
            self._f_lo = min(self._f_lo, f)
            self._f_hi = max(self._f_hi, f)
        if feasible:
            self.consider(x, f)
        return f

    def consider(self, x, f):
        """Учитывает допустимую точку; при улучшении вызывает callback."""
        with self._lock:
            if not f < self.best_f:
                return
            self.best_f = f
            self.best_x = x.copy()
        if self.notify is not None:
            try:
                if self.notify(x.copy(), f):
                    self.stopped = True
            except StopIteration:
                self.stopped = True
            except BaseException as exc:          # noqa: BLE001
                self._fail(exc)


def solve(func, bounds, *, args=(), constraints=(), integrality=None, x0=None,
          vectorized=False, maxfev=DEFAULT_MAXFEV, local_search=True, workers=1,
          notify=None, disp=False, options=None):
    """Решает задачу Globalizer'ом и возвращает OptimizeResult.

    notify(x, f) -> bool : вызывается при каждом улучшении лучшей допустимой
    точки; True означает просьбу остановиться.

    options (dict, ключи Globalizer):
      r                - параметр надёжности метода (> 1, по умолчанию 4.0);
      eps              - точность остановки по координатам в долях диапазона
                         (по умолчанию 0.01);
      maxfev           - лимит испытаний (перекрывает лимит метода scipy);
      local_search     - локальное уточнение решения (Hooke-Jeeves);
      local_iterations - итераций локального метода (по умолчанию 100);
      eq_tol           - допуск для ограничений-равенств (по умолчанию 1e-2);
      threads          - число потоков (перекрывает workers);
      disp             - печать итогов решателя.
    """
    opts = dict(options or {})
    native, PYProblem = _native.load()

    lb, ub = normalize_bounds(bounds)
    layout = _Layout(lb, ub, integrality)
    eq_tol = float(opts.get("eq_tol", DEFAULT_EQ_TOL))
    cons = normalize_constraints(constraints, layout.center(), eq_tol)
    ev = _Evaluator(layout, func, args, cons, vectorized, notify)

    # Стартовая точка пользователя: Globalizer не умеет начинать с неё,
    # но она учитывается как кандидат в решение.
    if x0 is not None:
        x_start = layout.clip(x0)
        f0 = ev.call_func(x_start)
        if np.isfinite(f0) and ev.is_feasible(x_start):
            ev.consider(x_start, f0)
        if ev.error is not None:
            raise ev.error

    nit = 0
    budget = _clamp_maxfev(opts.get("maxfev", maxfev))
    if layout.n_free > 0 and not ev.halted:
        problem = PYProblem(dimension=layout.nc, numCriterions=1)
        lower = [float(v) for v in lb[layout.cont]] + ([0.0] if layout.dummy else [])
        upper = [float(v) for v in ub[layout.cont]] + ([1.0] if layout.dummy else [])
        problem.set_bounds(lower, upper)
        if layout.disc.size:
            problem.set_discrete_variables(
                [[str(int(v)) for v in vals] for vals in layout.disc_values],
                [f"x{i}" for i in layout.disc])
        for c in cons:                         # ограничения - первыми
            problem.add_function(ev.make_constraint(c))
        problem.add_function(ev.objective)     # критерий - последним

        params = native.SolverParameters()
        params.max_iterations = budget
        params.r = float(opts.get("r", 4.0))
        params.epsilon = float(opts.get("eps", 0.01))
        params.num_threads = int(opts.get("threads", workers))
        params.local_refine = bool(opts.get("local_search", local_search))
        params.local_iterations = int(opts.get("local_iterations", 100))
        params.verbose = 0
        # print_result и log_prefix есть только в новых сборках PYDGlobalizer.
        if hasattr(params, "print_result"):
            params.print_result = bool(opts.get("disp", disp))
        if hasattr(params, "log_prefix"):
            params.log_prefix = os.path.join(tempfile.gettempdir(), "globalizer_log")

        raw = native.solve(problem, params)
        if ev.error is not None:
            raise ev.error
        if not raw.get("success", False):
            raise RuntimeError("Globalizer failed: " + str(raw.get("error")))
        nit = int(raw.get("iterations", 0))
        if ev.best_x is None:
            fallback = layout.to_x(np.asarray(raw["best_point"], dtype=float))
        else:
            fallback = None
    else:
        fallback = None if ev.best_x is not None else layout.center()

    # --- результат ----------------------------------------------------------
    if ev.stopped:
        status = 2
    elif ev.best_x is None:
        status = 3
    elif nit >= budget:
        status = 1
    else:
        status = 0

    if ev.best_x is not None:
        x, fun = ev.best_x, ev.best_f
    else:
        x = fallback
        try:
            fun = ev.call_func(x)
        except Exception:                          # noqa: BLE001
            fun = np.inf
        if layout.n_free == 0 and np.isfinite(fun) and ev.is_feasible(x):
            ev.consider(x, fun)
            status = 0

    result = OptimizeResult(
        x=np.asarray(x, dtype=float),
        fun=float(fun),
        success=status in (0, 1) and ev.best_x is not None,
        status=status,
        message=_MESSAGES[status],
        nfev=ev.nfev,
        nit=nit,
    )
    if cons:
        cv = ev.max_violation(result.x)
        result["maxcv"] = cv
        result["constr_violation"] = cv
    return result
