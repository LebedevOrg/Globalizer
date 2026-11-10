"""Исследование (Study) - как в optuna.study; точки испытаний выбирает Globalizer."""

import copy
import datetime
import enum
import math
import time
import uuid
import warnings

import numpy as np

from . import logging as _log
from ._engine import Engine, SearchSpace, encode_value, make_key
from .distributions import check_distribution_compatibility, convert
from .exceptions import TrialPruned
from .pruners import BasePruner, MedianPruner
from .samplers import as_globalizer_sampler
from .trial import FrozenTrial, Trial, TrialState, _validate

__all__ = ["Study", "StudyDirection", "create_study", "load_study", "delete_study",
           "copy_study", "get_all_study_names", "get_all_study_summaries"]

_logger = _log.get_logger(__name__)


class StudyDirection(enum.IntEnum):
    NOT_SET = 0
    MINIMIZE = 1
    MAXIMIZE = 2

    def __repr__(self):
        return f"StudyDirection.{self.name}"


def _to_direction(d):
    if isinstance(d, StudyDirection):
        return d
    if getattr(type(d), "__name__", "") == "StudyDirection":       # объект самой Optuna
        return StudyDirection(int(d))
    if d == "minimize":
        return StudyDirection.MINIMIZE
    if d == "maximize":
        return StudyDirection.MAXIMIZE
    raise ValueError(f"Please set either 'minimize' or 'maximize' to direction. You specified {d}.")


class Study:
    """Исследование: набор испытаний одной задачи оптимизации.

    Создаётся функцией :func:`create_study`. Интерфейс совпадает с
    optuna.study.Study (одна цель, хранение в памяти).
    """

    def __init__(self, study_name, direction, sampler, pruner):
        self.study_name = study_name
        self._direction = direction
        self.sampler = sampler
        self.pruner = pruner
        self._trials = []
        self._user_attrs = {}
        self._metric_names = None
        self._stop_flag = False
        self._rng = np.random.default_rng(sampler.seed)

        # Пространство поиска и метод Globalizer.
        self._dists = {}                 # все параметры, встречавшиеся в испытаниях
        self._space = None               # SearchSpace текущего запуска метода
        self._space_dirty = False        # пространство изменилось - нужен перезапуск
        self._engine = None
        self._outstanding = None         # (номер испытания, engine, key) - ждёт tell()
        self._cache = {}                 # key -> (ограничения, значение для метода)
        # Набор реально запрошенных параметров -> {их значения: результат}.
        # Точка метода, в которой все параметры прошлого испытания совпадают,
        # даёт тот же результат (целевая функция зависит только от того, что
        # она запросила): так в условном пространстве не повторяются
        # испытания, отличающиеся лишь неиспользуемыми параметрами.
        self._used_cache = {}
        self._n_constraints = None
        self._eps = sampler.eps
        self._idle_finishes = 0
        self._exhausted = False
        self._f_lo, self._f_hi, self._g_hi = math.inf, -math.inf, 0.0

    # --- свойства -----------------------------------------------------------
    @property
    def direction(self):
        return self._direction

    @property
    def directions(self):
        return [self._direction]

    @property
    def user_attrs(self):
        return copy.deepcopy(self._user_attrs)

    def set_user_attr(self, key, value):
        self._user_attrs[key] = value

    @property
    def metric_names(self):
        return None if self._metric_names is None else list(self._metric_names)

    def set_metric_names(self, metric_names):
        if len(metric_names) != 1:
            raise ValueError("The number of objectives must match the length of the metric names.")
        self._metric_names = list(metric_names)

    @property
    def trials(self):
        return self.get_trials(deepcopy=True)

    def get_trials(self, deepcopy=True, states=None):
        trials = [t for t in self._trials if states is None or t.state in states]
        return copy.deepcopy(trials) if deepcopy else list(trials)

    def _best(self):
        complete = [t for t in self._trials
                    if t.state == TrialState.COMPLETE and t.value is not None]
        if not complete:
            raise ValueError("No trials are completed yet.")
        feasible = [t for t in complete if t.feasible]
        if not feasible:
            raise ValueError("No feasible trials are completed yet.")
        pick = max if self._direction == StudyDirection.MAXIMIZE else min
        return pick(feasible, key=lambda t: t.value)

    @property
    def best_trial(self):
        return copy.deepcopy(self._best())

    @property
    def best_trials(self):
        return [self.best_trial]

    @property
    def best_value(self):
        return self._best().value

    @property
    def best_params(self):
        return dict(self._best().params)

    # --- оптимизация -------------------------------------------------------
    def optimize(self, func, n_trials=None, timeout=None, n_jobs=1, catch=(), callbacks=None,
                 gc_after_trial=False, show_progress_bar=False):
        """Выполняет n_trials испытаний (или до timeout / study.stop()).

        Испытания выполняются последовательно (n_jobs > 1 не поддерживается).
        Оптимизация завершается раньше, если метод Globalizer исчерпал
        пространство поиска (достиг точности eps_min сэмплера или все точки
        уже вычислены) - так же Optuna поступает с GridSampler.
        """
        if n_jobs != 1:
            warnings.warn("Globalizer runs trials sequentially: n_jobs is ignored.",
                          UserWarning, stacklevel=2)
        if isinstance(catch, type):
            catch = (catch,)
        catch = tuple(catch)
        callbacks = list(callbacks or [])
        start = time.time()
        done = 0
        self._stop_flag = False
        try:
            while True:
                if n_trials is not None and done >= n_trials:
                    break
                if timeout is not None and time.time() - start >= timeout:
                    break
                if self._stop_flag:
                    break
                trial = self._ask(stop_if_exhausted=True)
                if trial is None:
                    _logger.info(f"Globalizer finished the search (eps={self._eps:.3g}); "
                                 f"optimize stopped after {done} trials.")
                    break
                frozen = self._run_trial(func, trial, catch)
                done += 1
                for cb in callbacks:
                    cb(self, frozen)
        finally:
            self._stop_flag = False

    def _run_trial(self, func, trial, catch):
        try:
            value = func(trial)
        except TrialPruned:
            return self.tell(trial, state=TrialState.PRUNED)
        except Exception as exc:                   # noqa: BLE001
            frozen = self._tell_fail(trial, exc)
            if isinstance(exc, catch):
                return frozen
            raise
        except BaseException as exc:               # KeyboardInterrupt и т.п.: как в Optuna - FAIL
            self._tell_fail(trial, exc)
            raise
        values, error = _check_value(value)
        if error is not None:
            frozen = self._fail(trial._frozen, error)
            return frozen
        return self.tell(trial, values)

    def stop(self):
        """Останавливает optimize() после текущего испытания."""
        self._stop_flag = True

    # --- ask / tell ---------------------------------------------------------
    def ask(self, fixed_distributions=None):
        """Новое испытание; значения параметров предлагает Globalizer."""
        trial = self._ask(stop_if_exhausted=False)
        for name, dist in (fixed_distributions or {}).items():
            trial._suggest(name, convert(dist))
        return trial

    def _ask(self, stop_if_exhausted):
        now = datetime.datetime.now()
        waiting = next((t for t in self._trials if t.state == TrialState.WAITING), None)
        proposal, fixed, source = {}, {}, "fixed"
        if waiting is not None:
            fixed = dict(waiting.system_attrs.get("fixed_params", {}))
        else:
            proposal, source = self._next_point()
            if source == "exhausted" and stop_if_exhausted:
                return None
        if waiting is not None:
            frozen = waiting
            frozen.state = TrialState.RUNNING
            frozen.datetime_start = now
        else:
            frozen = FrozenTrial(number=len(self._trials), state=TrialState.RUNNING, value=None,
                                 datetime_start=now, datetime_complete=None, params={},
                                 distributions={}, user_attrs={}, system_attrs={},
                                 intermediate_values={}, trial_id=len(self._trials))
            self._trials.append(frozen)
        if source == "engine":
            _, engine, key = self._outstanding
            self._outstanding = (frozen.number, engine, key)
        return Trial(self, frozen, proposal=proposal, fixed=fixed)

    def _next_point(self):
        """(параметры, источник): engine | center | random | exhausted."""
        if self._outstanding is not None:
            # Метод последователен: пока он ждёт tell() предыдущей точки,
            # параллельные испытания получают случайные точки.
            return self._random_point(), "random"
        while True:
            if self._space is None or self._space_dirty:
                self._stop_engine()
                self._space = SearchSpace(self._dists, self.sampler.max_discrete_combinations)
                self._space_dirty = False
                self._eps = self.sampler.eps
                self._idle_finishes = 0
                self._exhausted = False
            if self._space.empty:
                # Параметров ещё нет (первое испытание) или все фиксированы:
                # suggest_* вернёт центр диапазона, новые параметры расширят пространство.
                return {}, "center"
            if self._exhausted:
                return self._random_point(), "exhausted"
            if self._engine is None:
                self._engine = Engine(self._space, self._n_constraints or 0, self.sampler,
                                      self._eps, self._lookup)
                self._engine.start()
            elif not self._engine.alive and self._engine.killed:
                self._engine = None                  # остановлен другим исследованием
                continue
            kind, payload, key = self._engine.next()
            if kind == "point":
                self._outstanding = (None, self._engine, key)
                return payload, "engine"
            engine, self._engine = self._engine, None
            if kind == "error":
                raise RuntimeError(f"Globalizer failed: {payload}") from payload
            if engine.killed:
                continue                             # перезапуск с той же точностью
            # Метод достиг точности: перезапуск с меньшей eps (известные точки
            # берутся из кэша) или конец поиска.
            self._idle_finishes = 0 if engine.new_points else self._idle_finishes + 1
            self._eps *= self.sampler.eps_decay
            if self._eps < self.sampler.eps_min or self._idle_finishes >= 2:
                self._exhausted = True
                _logger.debug("Globalizer search space is exhausted.")
            else:
                _logger.debug(f"Globalizer reached accuracy; restarting with eps={self._eps:.3g}.")

    def _random_point(self):
        if self._space is None:
            return {}
        return self._space.sample(self._rng)

    def _lookup(self, key):
        if key is None:
            return None
        res = self._cache.get(key)
        if res is not None:
            return res
        point = dict(key)
        for names, table in list(self._used_cache.items()):
            if all(n in point for n in names):
                res = table.get(tuple(point[n] for n in names))
                if res is not None:
                    return res
        return None

    def _stop_engine(self):
        if self._engine is not None:
            self._engine.stop()
            self._engine = None
        self._outstanding = None

    def _register_param(self, name, dist):
        old = self._dists.get(name)
        if old is None:
            self._dists[name] = dist
            self._space_dirty = True
        elif old != dist:
            check_distribution_compatibility(old, dist)
            self._dists[name] = dist
            self._space_dirty = True

    def _proposal_valid(self, name, dist):
        return self._space is not None and self._space.dists.get(name) == dist

    def tell(self, trial, values=None, state=None, skip_if_finished=False):
        """Завершает испытание: значение (values) и/или состояние (state)."""
        frozen = self._resolve(trial)
        if frozen.state.is_finished():
            if skip_if_finished:
                _logger.info(f"Skipped telling trial {frozen.number} with values {values} and "
                             f"state {state!r} since trial was already finished.")
                return copy.deepcopy(frozen)
            raise RuntimeError(f"Cannot tell a {frozen.state.name} trial.")
        if state is None:
            state = TrialState.COMPLETE if values is not None else TrialState.FAIL
        if state not in (TrialState.COMPLETE, TrialState.PRUNED, TrialState.FAIL):
            raise ValueError(f"Cannot tell with state {state!r}.")
        if state == TrialState.COMPLETE:
            vals, error = _check_value(values)
            if error is not None:
                return self._fail(frozen, error)
            frozen.values = vals
        elif state == TrialState.PRUNED:
            last = frozen.last_step
            v = frozen.intermediate_values[last] if last is not None else None
            frozen.values = [v] if v is not None and math.isfinite(v) else None
        else:
            frozen.values = None
        if state != TrialState.FAIL and self.sampler.constraints_func is not None:
            done = copy.deepcopy(frozen)
            done.state = state
            cons = self.sampler.constraints_func(done)
            frozen.system_attrs["constraints"] = tuple(float(c) for c in cons)
        frozen.state = state
        frozen.datetime_complete = datetime.datetime.now()
        self._feed_method(frozen)
        self._log_trial(frozen)
        return copy.deepcopy(frozen)

    def _tell_fail(self, trial, exc):
        frozen = trial._frozen
        if frozen.state.is_finished():
            return copy.deepcopy(frozen)
        frozen.state = TrialState.FAIL
        frozen.datetime_complete = datetime.datetime.now()
        frozen.system_attrs["fail_reason"] = repr(exc)
        self._feed_method(frozen)
        _logger.warning(f"Trial {frozen.number} failed with parameters: {frozen.params} because "
                        f"of the following error: {exc!r}.")
        return copy.deepcopy(frozen)

    def _fail(self, frozen, message):
        frozen.state = TrialState.FAIL
        frozen.values = None
        frozen.datetime_complete = datetime.datetime.now()
        frozen.system_attrs["fail_reason"] = message
        self._feed_method(frozen)
        _logger.warning(f"Trial {frozen.number} failed with parameters: {frozen.params} because "
                        f"of the following error: {message}")
        return copy.deepcopy(frozen)

    def _resolve(self, trial):
        if isinstance(trial, Trial):
            if trial.study is not self:
                raise ValueError("The trial belongs to another study.")
            return trial._frozen
        if isinstance(trial, int):
            if not 0 <= trial < len(self._trials):
                raise ValueError(f"Cannot tell for trial with number {trial} since it has not "
                                 "been created.")
            return self._trials[trial]
        raise TypeError("Trial must be a trial object or trial number.")

    # --- связь с методом Globalizer ----------------------------------------
    def _sign(self):
        return -1.0 if self._direction == StudyDirection.MAXIMIZE else 1.0

    def _penalty(self):
        if self._f_hi < self._f_lo:
            return 1.0
        return self._f_hi + (self._f_hi - self._f_lo) + 1.0

    def _feed_method(self, frozen):
        """Значение испытания -> кэш и ответ методу (если точка была его)."""
        cons = [frozen.constraints[k] for k in sorted(frozen.constraints)]
        state = frozen.state
        if state != TrialState.FAIL:
            if self._n_constraints is None:
                self._n_constraints = len(cons)
            elif len(cons) != self._n_constraints:
                self._n_constraints = len(cons)
                self._space_dirty = True
        for g in cons:
            if math.isfinite(g):
                self._g_hi = max(self._g_hi, g)
        value = frozen.values[0] if frozen.values else None
        if state == TrialState.COMPLETE and value is not None and math.isfinite(value):
            f = self._sign() * value
            self._f_lo = min(self._f_lo, f)
            self._f_hi = max(self._f_hi, f)
        elif state == TrialState.PRUNED and value is not None and math.isfinite(value):
            f = self._sign() * value
        else:
            f = self._penalty()
        n = self._n_constraints or 0
        if state == TrialState.FAIL:
            cons = [2.0 * self._g_hi + 1.0] * n
        cons = [g if math.isfinite(g) else 2.0 * self._g_hi + 1.0 for g in cons]
        cons = (cons + [0.0] * n)[:n]
        result = (tuple(cons), float(f))

        # Ключ - по всем известным параметрам: точку получит и метод,
        # запущенный позже (после расширения пространства этим испытанием).
        key = make_key(self._dists, frozen.params)
        if key is not None:
            self._cache.setdefault(key, result)
        if frozen.params:
            names = tuple(sorted(frozen.params))
            values = tuple(encode_value(frozen.distributions[n], frozen.params[n]) for n in names)
            if None not in values:
                self._used_cache.setdefault(names, {}).setdefault(values, result)
        if self._outstanding is not None and self._outstanding[0] == frozen.number:
            _, engine, key = self._outstanding
            self._outstanding = None
            if key is not None:
                self._cache[key] = result
            if engine is self._engine and engine.alive:
                if self._space_dirty:
                    self._stop_engine()
                else:
                    engine.answer(result)

    def _log_trial(self, frozen):
        if frozen.state == TrialState.COMPLETE:
            try:
                best = self._best()
                tail = f" Best is trial {best.number} with value: {best.value}."
            except ValueError:
                tail = ""
            _logger.info(f"Trial {frozen.number} finished with value: {frozen.value} and "
                         f"parameters: {frozen.params}.{tail}")
        elif frozen.state == TrialState.PRUNED:
            _logger.info(f"Trial {frozen.number} pruned. ")
        else:
            _logger.warning(f"Trial {frozen.number} failed with parameters: {frozen.params}.")

    # --- добавление испытаний вручную --------------------------------------
    def enqueue_trial(self, params, user_attrs=None, skip_if_exists=False):
        """Ставит в очередь испытание с заданными параметрами (вычисляется первым)."""
        if skip_if_exists:
            for t in self._trials:
                if t.state in (TrialState.WAITING,) and \
                        t.system_attrs.get("fixed_params") == params:
                    return
                if t.state.is_finished() or t.state == TrialState.RUNNING:
                    if t.params == params:
                        return
        frozen = FrozenTrial(number=len(self._trials), state=TrialState.WAITING, value=None,
                             datetime_start=None, datetime_complete=None, params={},
                             distributions={}, user_attrs=dict(user_attrs or {}),
                             system_attrs={"fixed_params": dict(params)},
                             intermediate_values={}, trial_id=len(self._trials))
        self._trials.append(frozen)

    def add_trial(self, trial):
        """Добавляет готовое испытание (create_trial); его значение получит метод."""
        frozen = copy.deepcopy(trial)
        frozen.distributions = {k: convert(d) for k, d in frozen.distributions.items()}
        _validate(frozen)
        frozen.number = len(self._trials)
        frozen._trial_id = frozen.number
        self._trials.append(frozen)
        for name, d in frozen.distributions.items():
            self._register_param(name, d)
        if frozen.state.is_finished():
            self._feed_method(frozen)

    def add_trials(self, trials):
        for t in trials:
            self.add_trial(t)

    # --- экспорт ------------------------------------------------------------
    def trials_dataframe(self, attrs=("number", "value", "datetime_start", "datetime_complete",
                                      "duration", "params", "user_attrs", "system_attrs",
                                      "state"), multi_index=False):
        try:
            import pandas as pd
        except ImportError:
            raise ImportError("trials_dataframe() requires pandas: pip install pandas") from None
        rows = []
        for t in self._trials:
            row = {}
            for a in attrs:
                if a in ("params", "user_attrs", "system_attrs"):
                    for k, v in getattr(t, a).items():
                        row[(a, k) if multi_index else f"{a}_{k}"] = v
                elif a == "state":
                    row[a] = t.state.name
                else:
                    row[a] = getattr(t, a)
            rows.append(row)
        df = pd.DataFrame(rows)
        if multi_index:
            df.columns = pd.MultiIndex.from_tuples(
                [c if isinstance(c, tuple) else (c, "") for c in df.columns])
        return df

    def __repr__(self):
        return f"Study(study_name={self.study_name!r}, direction={self._direction!r})"


def _check_value(value):
    """(values, None) или (None, сообщение об ошибке) - как проверка в Optuna."""
    if value is None:
        return None, "The objective function returned None."
    if isinstance(value, (list, tuple, np.ndarray)):
        seq = list(np.asarray(value, dtype=object).reshape(-1))
        if len(seq) != 1:
            return None, (f"The number of the values {len(seq)} did not match the number of the "
                          "objectives 1.")
        value = seq[0]
    try:
        v = float(value)
    except (TypeError, ValueError):
        return None, (f"The value {value!r} could not be cast to float.")
    if math.isnan(v):
        return None, f"The value {v} is not acceptable."
    return [v], None


def create_study(*, storage=None, sampler=None, pruner=None, study_name=None, direction=None,
                 load_if_exists=False, directions=None):
    """Создаёт исследование (в памяти). Точки испытаний выбирает Globalizer."""
    if storage is not None:
        raise NotImplementedError("globalizer.optuna keeps studies in memory only: "
                                  "`storage` is not supported.")
    if direction is not None and directions is not None:
        raise ValueError("Specify only one of `direction` and `directions`.")
    if directions is not None:
        if len(directions) != 1:
            raise NotImplementedError("Multi-objective optimization is not supported by "
                                      "globalizer.optuna: use a single direction.")
        direction = directions[0]
    direction = _to_direction(direction if direction is not None else "minimize")
    if pruner is None:
        pruner = MedianPruner()
    elif not isinstance(pruner, BasePruner) and not hasattr(pruner, "prune"):
        raise TypeError(f"Unsupported pruner {pruner!r}.")
    name = study_name or f"no-name-{uuid.uuid4()}"
    study = Study(name, direction, as_globalizer_sampler(sampler), pruner)
    _logger.info(f"A new study created in memory with name: {name}")
    return study


def _no_storage(*args, **kwargs):
    raise NotImplementedError("globalizer.optuna keeps studies in memory only "
                              "(no storage support).")


load_study = delete_study = copy_study = get_all_study_names = get_all_study_summaries = _no_storage
