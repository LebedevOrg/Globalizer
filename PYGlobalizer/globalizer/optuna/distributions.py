"""Распределения (пространство поиска) - те же классы и поля, что в optuna.distributions.

FloatDistribution(low, high, log=False, step=None)
IntDistribution(low, high, log=False, step=1)
CategoricalDistribution(choices)

Принимаются и объекты распределений самой Optuna (по именам полей), см.
:func:`convert`.
"""

import copy
import decimal
import math

__all__ = ["BaseDistribution", "FloatDistribution", "IntDistribution",
           "CategoricalDistribution", "convert", "check_distribution_compatibility"]


class BaseDistribution:
    """Базовый класс распределения."""

    def _contains(self, value):            # pragma: no cover - переопределяется
        raise NotImplementedError

    def single(self):
        """True, если у распределения ровно одно значение."""
        raise NotImplementedError

    def _asdict(self):
        return dict(self.__dict__)

    def __eq__(self, other):
        return type(self) is type(other) and self._asdict() == other._asdict()

    def __hash__(self):
        return hash((type(self).__name__,) + tuple(
            (k, tuple(v) if isinstance(v, list) else v) for k, v in sorted(self._asdict().items())))

    def __repr__(self):
        kw = ", ".join(f"{k}={v!r}" for k, v in self._asdict().items())
        return f"{type(self).__name__}({kw})"


def _adjust_high(low, high, step):
    """Как в Optuna: high сдвигается вниз до ближайшего узла сетки low + k*step."""
    d_high = decimal.Decimal(str(high))
    d_low = decimal.Decimal(str(low))
    d_step = decimal.Decimal(str(step))
    d_r = d_high - d_low
    if d_r % d_step != decimal.Decimal("0"):
        high = float((d_r // d_step) * d_step + d_low)
    return high


class FloatDistribution(BaseDistribution):
    """Вещественный параметр на [low, high]; log - логарифмическая шкала, step - сетка."""

    def __init__(self, low, high, log=False, step=None):
        if log and step is not None:
            raise ValueError("The parameter `step` is not supported when `log` is true.")
        if low > high:
            raise ValueError(f"`low <= high` must hold, but got (low, high)=({low}, {high}).")
        if log and low <= 0.0:
            raise ValueError(f"`low > 0` must hold for `log=True`, but got (low, high)=({low}, {high}).")
        if step is not None and step <= 0:
            raise ValueError(f"`step > 0` must hold, but got step={step}.")
        if step is not None:
            high = _adjust_high(low, high, step)
        self.low = float(low)
        self.high = float(high)
        self.log = bool(log)
        self.step = None if step is None else float(step)

    def single(self):
        if self.step is None:
            return self.low == self.high
        return self.high - self.low < self.step

    def _contains(self, value):
        v = float(value)
        if self.step is None:
            return self.low <= v <= self.high
        k = (v - self.low) / self.step
        return self.low <= v <= self.high and abs(k - round(k)) < 1e-8

    def n_values(self):
        """Число узлов сетки (None, если сетки нет)."""
        if self.step is None:
            return None
        return int(round((self.high - self.low) / self.step)) + 1


class IntDistribution(BaseDistribution):
    """Целый параметр на [low, high] с шагом step; log - логарифмическая шкала."""

    def __init__(self, low, high, log=False, step=1):
        if log and step != 1:
            raise ValueError("Samplers and other components in Optuna only accept step is 1 "
                             "when `log` argument is True.")
        if low > high:
            raise ValueError(f"`low <= high` must hold, but got (low, high)=({low}, {high}).")
        if log and low < 1:
            raise ValueError(f"`low >= 1` must hold for `log=True`, but got (low, high)=({low}, {high}).")
        if step <= 0:
            raise ValueError(f"`step > 0` must hold, but got step={step}.")
        self.log = bool(log)
        self.step = int(step)
        self.low = int(low)
        high = int(high)
        self.high = high - (high - self.low) % self.step

    def single(self):
        return self.low == self.high

    def _contains(self, value):
        v = float(value)
        return self.low <= v <= self.high and (v - self.low) % self.step == 0

    def n_values(self):
        return (self.high - self.low) // self.step + 1


class CategoricalDistribution(BaseDistribution):
    """Категориальный параметр: одно из значений choices (None, bool, int, float, str)."""

    def __init__(self, choices):
        if len(choices) == 0:
            raise ValueError("The `choices` must contain one or more elements.")
        for c in choices:
            if c is not None and not isinstance(c, (bool, int, float, str)):
                raise ValueError(
                    "Choices for a categorical distribution should be a tuple of None, bool, "
                    f"int, float and str for persistent storage but contains {c!r} which is of "
                    f"type {type(c).__name__}.")
        self.choices = tuple(choices)

    def single(self):
        return len(self.choices) == 1

    def _contains(self, value):
        return any(_same(value, c) for c in self.choices)

    def index_of(self, value):
        for i, c in enumerate(self.choices):
            if _same(value, c):
                return i
        raise ValueError(f"{value!r} is not in choices {self.choices}.")

    def n_values(self):
        return len(self.choices)


def _same(a, b):
    if isinstance(a, float) and isinstance(b, float) and math.isnan(a) and math.isnan(b):
        return True
    if isinstance(a, bool) != isinstance(b, bool):     # True != 1 для категорий
        return False
    return a == b


def convert(dist):
    """Распределение Optuna (или любое с теми же полями) -> распределение этого модуля."""
    if isinstance(dist, BaseDistribution):
        return copy.deepcopy(dist)
    name = type(dist).__name__
    if name == "FloatDistribution":
        return FloatDistribution(dist.low, dist.high, log=dist.log, step=dist.step)
    if name == "IntDistribution":
        return IntDistribution(dist.low, dist.high, log=dist.log, step=dist.step)
    if name == "CategoricalDistribution":
        return CategoricalDistribution(dist.choices)
    # Устаревшие распределения Optuna 2.x.
    if name == "UniformDistribution":
        return FloatDistribution(dist.low, dist.high)
    if name == "LogUniformDistribution":
        return FloatDistribution(dist.low, dist.high, log=True)
    if name == "DiscreteUniformDistribution":
        return FloatDistribution(dist.low, dist.high, step=dist.q)
    if name == "IntUniformDistribution":
        return IntDistribution(dist.low, dist.high, step=getattr(dist, "step", 1))
    if name == "IntLogUniformDistribution":
        return IntDistribution(dist.low, dist.high, log=True)
    raise TypeError(f"Unsupported distribution: {dist!r}")


def check_distribution_compatibility(dist_old, dist_new):
    """Как в Optuna: у одного параметра нельзя менять тип распределения."""
    if type(dist_old) is not type(dist_new):
        raise ValueError("Cannot set different distribution kind to the same parameter name.")
    if isinstance(dist_old, CategoricalDistribution) and dist_old.choices != dist_new.choices:
        raise ValueError(CategoricalDistribution.__name__ +
                         " does not support dynamic value space.")
