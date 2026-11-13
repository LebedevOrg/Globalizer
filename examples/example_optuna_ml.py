"""Пример: подбор гиперпараметров моделей scikit-learn через globalizer.optuna.

Типичный сценарий Optuna, решаемый методом Globalizer:
  1) выбор модели и её гиперпараметров (условное пространство поиска,
     лог-шкала, целые и категориальные параметры) по качеству на
     кросс-валидации;
  2) досрочная остановка (прунинг) неперспективных испытаний по
     промежуточным значениям: SGDClassifier учится по эпохам.

Запуск:  python examples/example_optuna_ml.py [optuna]
(с аргументом optuna - то же самое делает Optuna). Нужен scikit-learn.
"""

import sys
import warnings
from pathlib import Path

import numpy as np

if len(sys.argv) > 1 and sys.argv[1] == "optuna":
    import optuna
else:
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "PYGlobalizer"))
    import globalizer.optuna as optuna

from sklearn.datasets import load_breast_cancer
from sklearn.ensemble import RandomForestClassifier
from sklearn.linear_model import SGDClassifier
from sklearn.model_selection import cross_val_score, train_test_split
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVC

optuna.logging.set_verbosity(optuna.logging.WARNING)
warnings.filterwarnings("ignore")

X, y = load_breast_cancer(return_X_y=True)


def model_selection(trial):
    """Точность на 3-кратной кросс-валидации (максимизируется)."""
    name = trial.suggest_categorical("model", ["svc", "forest"])
    if name == "svc":
        c = trial.suggest_float("svc_C", 1e-3, 1e3, log=True)
        gamma = trial.suggest_float("svc_gamma", 1e-4, 1e1, log=True)
        model = make_pipeline(StandardScaler(), SVC(C=c, gamma=gamma))
    else:
        model = RandomForestClassifier(
            n_estimators=trial.suggest_int("rf_n_estimators", 10, 200, log=True),
            max_depth=trial.suggest_int("rf_max_depth", 2, 16),
            max_features=trial.suggest_categorical("rf_max_features", ["sqrt", "log2"]),
            random_state=0, n_jobs=1)
    return cross_val_score(model, X, y, cv=3).mean()


X_train, X_valid, y_train, y_valid = train_test_split(X, y, test_size=0.3, random_state=0)
scaler = StandardScaler().fit(X_train)
X_train, X_valid = scaler.transform(X_train), scaler.transform(X_valid)


def sgd_with_pruning(trial):
    """Ошибка на валидации после 20 эпох; плохие испытания обрезаются раньше."""
    alpha = trial.suggest_float("alpha", 1e-6, 1e-1, log=True)
    loss = trial.suggest_categorical("loss", ["hinge", "log_loss", "modified_huber"])
    eta0 = trial.suggest_float("eta0", 1e-4, 1.0, log=True)
    clf = SGDClassifier(alpha=alpha, loss=loss, learning_rate="constant", eta0=eta0,
                        random_state=0)
    classes = np.unique(y)
    error = 1.0
    for epoch in range(20):
        clf.partial_fit(X_train, y_train, classes=classes)
        error = 1.0 - clf.score(X_valid, y_valid)
        trial.report(error, epoch)
        if trial.should_prune():
            raise optuna.TrialPruned()
    return error


def main():
    study = optuna.create_study(direction="maximize")
    study.optimize(model_selection, n_trials=60)
    print(f"model selection: accuracy = {study.best_value:.4f}  params = {study.best_params}")

    study2 = optuna.create_study(pruner=optuna.pruners.MedianPruner(n_startup_trials=5,
                                                                      n_warmup_steps=3))
    study2.optimize(sgd_with_pruning, n_trials=60)
    states = [t.state.name for t in study2.trials]
    print(f"SGD + pruning:   error = {study2.best_value:.4f}  params = "
          f"{ {k: (round(v, 6) if isinstance(v, float) else v) for k, v in study2.best_params.items()} }"
          f"  complete = {states.count('COMPLETE')}, pruned = {states.count('PRUNED')}")
    print("Minimum value =", 1.0 - study.best_value)


if __name__ == "__main__":
    main()
