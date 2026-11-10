"""globalizer.optuna - оптимизация Globalizer с интерфейсом Optuna.

Чтобы перевести код с Optuna на Globalizer, достаточно заменить импорт:

    import optuna                         # было
    import globalizer.optuna as optuna    # стало

    def objective(trial):
        x = trial.suggest_float("x", -10, 10)
        return (x - 2) ** 2

    study = optuna.create_study()
    study.optimize(objective, n_trials=100)
    print(study.best_params, study.best_value)

Точки испытаний выбирает индексный метод глобальной оптимизации Globalizer
(сэмплер :class:`samplers.GlobalizerSampler`); пространство поиска строится
по вызовам ``trial.suggest_*`` (define-by-run), как в Optuna. Поддерживаются
study.optimize / ask / tell / enqueue_trial / add_trial, ограничения
(``trial.set_constraint`` и ``constraints_func``), прунеры (MedianPruner,
PercentilePruner, NopPruner), направления minimize / maximize.
"""

from . import distributions, exceptions, logging, pruners, samplers, study, trial
from .exceptions import TrialPruned
from .study import (Study, StudyDirection, copy_study, create_study, delete_study,
                    get_all_study_names, get_all_study_summaries, load_study)
from .trial import FixedTrial, FrozenTrial, Trial, TrialState, create_trial

__version__ = "0.1.0"

__all__ = [
    "Study", "StudyDirection", "Trial", "FrozenTrial", "FixedTrial", "TrialState",
    "TrialPruned", "create_study", "create_trial", "load_study", "delete_study", "copy_study",
    "get_all_study_names", "get_all_study_summaries", "distributions", "exceptions",
    "logging", "pruners", "samplers", "study", "trial",
]
