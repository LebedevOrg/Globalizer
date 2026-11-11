"""Прунеры (досрочная остановка неперспективных испытаний) - как в optuna.pruners.

Прунер решает только, прерывать ли испытание по промежуточным значениям
(trial.report / trial.should_prune), и не зависит от сэмплера. Для метода
Globalizer значением прерванного испытания служит его последнее
промежуточное значение.
"""

import math

import numpy as np

__all__ = ["BasePruner", "NopPruner", "MedianPruner", "PercentilePruner"]


class BasePruner:
    """Базовый класс: prune(study, trial) -> bool."""

    def prune(self, study, trial):              # pragma: no cover - переопределяется
        raise NotImplementedError


class NopPruner(BasePruner):
    """Никогда не прерывает испытания."""

    def prune(self, study, trial):
        return False


class PercentilePruner(BasePruner):
    """Прерывает испытание, если его лучшее промежуточное значение хуже
    заданного перцентиля значений завершённых испытаний на том же шаге."""

    def __init__(self, percentile, n_startup_trials=5, n_warmup_steps=0, interval_steps=1,
                 *, n_min_trials=1):
        if not 0.0 <= percentile <= 100.0:
            raise ValueError(f"Percentile must be between 0 and 100 inclusive but got {percentile}.")
        if n_startup_trials < 0:
            raise ValueError(f"Number of startup trials cannot be negative but got {n_startup_trials}.")
        if n_warmup_steps < 0:
            raise ValueError(f"Number of warmup steps cannot be negative but got {n_warmup_steps}.")
        if interval_steps < 1:
            raise ValueError(f"Pruning interval steps must be at least 1 but got {interval_steps}.")
        if n_min_trials < 1:
            raise ValueError(f"Number of trials for pruning must be at least 1 but got {n_min_trials}.")
        self._percentile = float(percentile)
        self._n_startup_trials = n_startup_trials
        self._n_warmup_steps = n_warmup_steps
        self._interval_steps = interval_steps
        self._n_min_trials = n_min_trials

    def prune(self, study, trial):
        from .trial import TrialState
        from .study import StudyDirection

        completed = study.get_trials(deepcopy=False, states=(TrialState.COMPLETE,))
        if len(completed) == 0 or len(completed) < self._n_startup_trials:
            return False
        step = trial.last_step
        if step is None or step < self._n_warmup_steps:
            return False
        if (step - self._n_warmup_steps) % self._interval_steps != 0:
            return False

        maximize = study.direction == StudyDirection.MAXIMIZE
        values = [v for s, v in trial.intermediate_values.items() if s <= step]
        if not values or all(math.isnan(v) for v in values):
            return True
        best = np.nanmax(values) if maximize else np.nanmin(values)

        others = [t.intermediate_values[step] for t in completed if step in t.intermediate_values]
        if len(others) < self._n_min_trials:
            return False
        pct = 100.0 - self._percentile if maximize else self._percentile
        threshold = float(np.nanpercentile(np.asarray(others, dtype=float), pct))
        if math.isnan(threshold):
            return False
        return best < threshold if maximize else best > threshold


class MedianPruner(PercentilePruner):
    """Прерывает испытание, если оно хуже медианы завершённых (прунер Optuna по умолчанию)."""

    def __init__(self, n_startup_trials=5, n_warmup_steps=0, interval_steps=1, *, n_min_trials=1):
        super().__init__(50.0, n_startup_trials, n_warmup_steps, interval_steps,
                         n_min_trials=n_min_trials)
