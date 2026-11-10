"""Испытания - как в optuna.trial: Trial, FrozenTrial, FixedTrial, TrialState, create_trial."""

import copy
import datetime
import enum
import math
import warnings

from .distributions import (BaseDistribution, CategoricalDistribution, FloatDistribution,
                            IntDistribution, check_distribution_compatibility, convert)

__all__ = ["TrialState", "BaseTrial", "Trial", "FrozenTrial", "FixedTrial", "create_trial"]

_CONSTRAINTS_KEY = "constraints"          # constraints_func сэмплера (как в Optuna)
_CONSTRAINT_PREFIX = "constraints:"       # trial.set_constraint(key, value)


class TrialState(enum.IntEnum):
    RUNNING = 0
    COMPLETE = 1
    PRUNED = 2
    FAIL = 3
    WAITING = 4

    def is_finished(self):
        return self not in (TrialState.RUNNING, TrialState.WAITING)

    def __repr__(self):
        return f"TrialState.{self.name}"


def _constraints_of(system_attrs):
    out = {}
    func_values = system_attrs.get(_CONSTRAINTS_KEY)
    if func_values is not None:
        for i, v in enumerate(func_values):
            out[str(i)] = float(v)
    for k, v in system_attrs.items():
        if k.startswith(_CONSTRAINT_PREFIX):
            out[k[len(_CONSTRAINT_PREFIX):]] = float(v)
    return out


class BaseTrial:
    """Общая часть Trial / FrozenTrial / FixedTrial."""

    def suggest_float(self, name, low, high, *, step=None, log=False):
        return self._suggest(name, FloatDistribution(low, high, log=log, step=step))

    def suggest_int(self, name, low, high, *, step=1, log=False):
        return int(self._suggest(name, IntDistribution(low, high, log=log, step=step)))

    def suggest_categorical(self, name, choices):
        return self._suggest(name, CategoricalDistribution(choices))

    # Устаревшие имена Optuna (оставлены в Optuna 5 с предупреждением).
    def suggest_uniform(self, name, low, high):
        return self.suggest_float(name, low, high)

    def suggest_loguniform(self, name, low, high):
        return self.suggest_float(name, low, high, log=True)

    def suggest_discrete_uniform(self, name, low, high, q):
        return self.suggest_float(name, low, high, step=q)

    def _suggest(self, name, distribution):          # pragma: no cover
        raise NotImplementedError


class FrozenTrial(BaseTrial):
    """Завершённое (или ожидающее) испытание: только данные."""

    def __init__(self, number, state, value, datetime_start, datetime_complete, params,
                 distributions, user_attrs, system_attrs, intermediate_values, trial_id, *,
                 values=None):
        if value is not None and values is not None:
            raise ValueError("Specify only one of `value` and `values`.")
        if value is not None:
            values = [float(value)]
        self.number = number
        self.state = state
        self._values = None if values is None else [float(v) for v in values]
        self.datetime_start = datetime_start
        self.datetime_complete = datetime_complete
        self.params = dict(params)
        self.distributions = dict(distributions)
        self.user_attrs = dict(user_attrs)
        self.system_attrs = dict(system_attrs)
        self.intermediate_values = dict(intermediate_values)
        self._trial_id = trial_id

    # --- значения -----------------------------------------------------------
    @property
    def value(self):
        if self._values is None:
            return None
        if len(self._values) > 1:
            raise RuntimeError("This attribute is not available during multi-objective optimization.")
        return self._values[0]

    @value.setter
    def value(self, v):
        self._values = None if v is None else [float(v)]

    @property
    def values(self):
        return None if self._values is None else list(self._values)

    @values.setter
    def values(self, v):
        self._values = None if v is None else [float(x) for x in v]

    @property
    def duration(self):
        if self.datetime_start and self.datetime_complete:
            return self.datetime_complete - self.datetime_start
        return None

    @property
    def last_step(self):
        return max(self.intermediate_values) if self.intermediate_values else None

    @property
    def constraints(self):
        return _constraints_of(self.system_attrs)

    @property
    def feasible(self):
        return all(v <= 0.0 for v in self.constraints.values())

    # --- интерфейс испытания (для повторного вызова целевой функции) -------
    def _suggest(self, name, distribution):
        if name not in self.params:
            raise ValueError(f"The value of the parameter '{name}' is not found. Please set it "
                             "at the construction of the FrozenTrial object.")
        return self.params[name]

    def report(self, value, step):
        pass

    def should_prune(self):
        return False

    def set_user_attr(self, key, value):
        self.user_attrs[key] = value

    def set_constraint(self, key, value):
        self.system_attrs[_CONSTRAINT_PREFIX + str(key)] = float(value)

    def __eq__(self, other):
        return isinstance(other, FrozenTrial) and self.__dict__ == other.__dict__

    def __lt__(self, other):
        return self.number < other.number

    def __hash__(self):
        return hash((self.number, self._trial_id))

    def __repr__(self):
        return (f"FrozenTrial(number={self.number}, state={self.state!r}, values={self._values}, "
                f"datetime_start={self.datetime_start!r}, "
                f"datetime_complete={self.datetime_complete!r}, params={self.params}, "
                f"user_attrs={self.user_attrs}, system_attrs={self.system_attrs}, "
                f"intermediate_values={self.intermediate_values}, "
                f"distributions={self.distributions}, trial_id={self._trial_id})")


class Trial(BaseTrial):
    """Испытание в процессе выполнения (передаётся целевой функции).

    Значения параметров предлагает метод Globalizer; ``suggest_*`` возвращает
    значение предложенной точки. Новый параметр (или изменённое
    распределение) расширяет пространство поиска: метод перезапускается
    после этого испытания, а само испытание получает центр диапазона.
    """

    def __init__(self, study, frozen, proposal=None, fixed=None):
        self.study = study
        self._frozen = frozen                 # запись в study (RUNNING)
        self._proposal = proposal or {}       # точка метода Globalizer
        self._fixed = fixed or {}             # enqueue_trial / ask(fixed_distributions)

    @property
    def number(self):
        return self._frozen.number

    @property
    def params(self):
        return copy.deepcopy(self._frozen.params)

    @property
    def distributions(self):
        return copy.deepcopy(self._frozen.distributions)

    @property
    def user_attrs(self):
        return copy.deepcopy(self._frozen.user_attrs)

    @property
    def system_attrs(self):
        return copy.deepcopy(self._frozen.system_attrs)

    @property
    def datetime_start(self):
        return self._frozen.datetime_start

    @property
    def relative_params(self):
        return dict(self._proposal)

    @property
    def constraints(self):
        return self._frozen.constraints

    def _check_running(self):
        if self._frozen.state != TrialState.RUNNING:
            raise RuntimeError(f"Trial#{self.number} has already finished and can not be updated.")

    def _suggest(self, name, distribution):
        self._check_running()
        fr = self._frozen
        if name in fr.params:
            check_distribution_compatibility(fr.distributions[name], distribution)
            return fr.params[name]
        self.study._register_param(name, distribution)
        if name in self._fixed:
            value = self._fixed[name]
            if not distribution._contains(value):
                warnings.warn(f"Fixed parameter '{name}' with value {value!r} is out of range "
                              f"for distribution {distribution}.", UserWarning, stacklevel=3)
        elif name in self._proposal and self.study._proposal_valid(name, distribution):
            value = self._proposal[name]
        else:
            from ._engine import default_value
            value = default_value(distribution)
        if isinstance(distribution, IntDistribution):
            value = int(value)
        elif isinstance(distribution, FloatDistribution):
            value = float(value)
        fr.params[name] = value
        fr.distributions[name] = distribution
        return value

    def report(self, value, step):
        self._check_running()
        try:
            value = float(value)
        except (TypeError, ValueError):
            raise TypeError(f"The `value` argument is of type '{type(value).__name__}' but "
                            "supposed to be a float.") from None
        if step < 0:
            raise ValueError(f"The `step` argument is {step} but cannot be negative.")
        if step in self._frozen.intermediate_values:
            warnings.warn(f"The reported value is ignored because this `step` {step} is "
                          "already reported.", UserWarning, stacklevel=2)
            return
        self._frozen.intermediate_values[int(step)] = value

    def should_prune(self):
        self._check_running()
        return bool(self.study.pruner.prune(self.study, self._frozen))

    def set_user_attr(self, key, value):
        self._check_running()
        self._frozen.user_attrs[key] = value

    def set_system_attr(self, key, value):
        self._check_running()
        self._frozen.system_attrs[key] = value

    def set_constraint(self, key, value):
        """Ограничение испытания: допустимо, если все значения <= 0 (как в Optuna 5)."""
        self._check_running()
        self._frozen.system_attrs[_CONSTRAINT_PREFIX + str(key)] = float(value)

    def __repr__(self):
        return f"Trial(number={self.number}, params={self._frozen.params})"


class FixedTrial(BaseTrial):
    """Испытание с заданными параметрами - чтобы вызвать целевую функцию вне study."""

    def __init__(self, params, number=0):
        self._params = dict(params)
        self._suggested = {}
        self._distributions = {}
        self._user_attrs = {}
        self._system_attrs = {}
        self._number = number
        self._datetime_start = datetime.datetime.now()

    def _suggest(self, name, distribution):
        if name not in self._params:
            raise ValueError(f"The value of the parameter '{name}' is not found. Please set it "
                             "at the construction of the FixedTrial object.")
        value = self._params[name]
        if not distribution._contains(value):
            warnings.warn(f"The value {value} of the parameter '{name}' is out of the range of "
                          "the distribution.", UserWarning, stacklevel=3)
        self._suggested[name] = value
        self._distributions[name] = distribution
        return value

    def report(self, value, step):
        pass

    def should_prune(self):
        return False

    def set_user_attr(self, key, value):
        self._user_attrs[key] = value

    def set_constraint(self, key, value):
        self._system_attrs[_CONSTRAINT_PREFIX + str(key)] = float(value)

    @property
    def params(self):
        return dict(self._suggested)

    @property
    def distributions(self):
        return dict(self._distributions)

    @property
    def user_attrs(self):
        return dict(self._user_attrs)

    @property
    def system_attrs(self):
        return dict(self._system_attrs)

    @property
    def constraints(self):
        return _constraints_of(self._system_attrs)

    @property
    def number(self):
        return self._number

    @property
    def datetime_start(self):
        return self._datetime_start


def create_trial(*, state=TrialState.COMPLETE, value=None, values=None, params=None,
                 distributions=None, user_attrs=None, system_attrs=None,
                 intermediate_values=None, constraints=None):
    """Создаёт FrozenTrial (например, для study.add_trial)."""
    now = datetime.datetime.now()
    system_attrs = dict(system_attrs or {})
    for k, v in (constraints or {}).items():
        system_attrs[_CONSTRAINT_PREFIX + str(k)] = float(v)
    distributions = {k: convert(d) for k, d in (distributions or {}).items()}
    trial = FrozenTrial(
        number=-1, state=state, value=value, values=values,
        datetime_start=now if state != TrialState.WAITING else None,
        datetime_complete=now if state.is_finished() else None,
        params=params or {}, distributions=distributions, user_attrs=user_attrs or {},
        system_attrs=system_attrs, intermediate_values=intermediate_values or {}, trial_id=-1)
    _validate(trial)
    return trial


def _validate(trial):
    if set(trial.params) != set(trial.distributions):
        raise ValueError("Inconsistent parameters {} and distributions {}.".format(
            set(trial.params), set(trial.distributions)))
    for name, d in trial.distributions.items():
        if not isinstance(d, BaseDistribution):
            raise TypeError(f"Distribution of '{name}' must be a distribution object.")
        if not d._contains(trial.params[name]):
            raise ValueError(f"The value {trial.params[name]} of parameter '{name}' isn't "
                             f"contained in the distribution {d}.")
    if trial.state == TrialState.COMPLETE:
        if trial.values is None:
            raise ValueError("`value` or `values` must be set for a COMPLETE trial.")
        if any(math.isnan(v) for v in trial.values):
            raise ValueError("`value` must not be NaN for a COMPLETE trial.")
