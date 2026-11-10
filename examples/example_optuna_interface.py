"""Пример: интерфейс globalizer.optuna, совместимый с Optuna.

Код ниже написан для Optuna. Чтобы решить те же задачи Globalizer'ом,
достаточно заменить импорт:

    import optuna                         ->   import globalizer.optuna as optuna

Запуск:  python examples/example_optuna_interface.py [optuna]
(с аргументом optuna - те же задачи решает Optuna, для сравнения).
"""

import math
import sys
from pathlib import Path

if len(sys.argv) > 1 and sys.argv[1] == "optuna":
    import optuna
else:
    # Папка с пакетом globalizer (PYGlobalizer); PYDGlobalizer ищется в _bin.
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "PYGlobalizer"))
    import globalizer.optuna as optuna

optuna.logging.set_verbosity(optuna.logging.WARNING)


# 1. Функция Branin: три глобальных минимума со значением 0.397887.
def branin(trial):
    x = trial.suggest_float("x", -5, 10)
    y = trial.suggest_float("y", 0, 15)
    b, c, t = 5.1 / (4 * math.pi ** 2), 5 / math.pi, 1 / (8 * math.pi)
    return (y - b * x * x + c * x - 6) ** 2 + 10 * (1 - t) * math.cos(x) + 10


# 2. Параметры разных типов: лог-шкала, целый, с шагом, категориальный.
def mixed(trial):
    lr = trial.suggest_float("lr", 1e-5, 1e-1, log=True)
    layers = trial.suggest_int("layers", 1, 8)
    dropout = trial.suggest_float("dropout", 0.0, 0.5, step=0.05)
    act = trial.suggest_categorical("activation", ["relu", "tanh", "sigmoid"])
    penalty = {"relu": 0.0, "tanh": 0.3, "sigmoid": 1.0}[act]
    return (math.log10(lr) + 3) ** 2 + 0.1 * (layers - 4) ** 2 + (dropout - 0.2) ** 2 + penalty


# 3. Ограничение (Optuna 5): допустимо, если значение <= 0.
#    min (x-1)^2 + (y-1)^2 при x^2 + y^2 <= 1; точный ответ 3 - 2*sqrt(2).
def constrained(trial):
    x = trial.suggest_float("x", -2, 2)
    y = trial.suggest_float("y", -2, 2)
    trial.set_constraint("circle", x * x + y * y - 1.0)
    return (x - 1) ** 2 + (y - 1) ** 2


# 4. Условное пространство: набор параметров зависит от выбранной модели.
def conditional(trial):
    model = trial.suggest_categorical("model", ["poly", "sin"])
    if model == "poly":
        a = trial.suggest_float("a", -3, 3)
        return (a * a - 2) ** 2 + 0.5
    w = trial.suggest_float("w", 0.5, 5.0)
    return 1.0 + math.sin(w * 1.3)          # минимум 0 при w = 3*pi/2/1.3


def main():
    study = optuna.create_study()
    study.optimize(branin, n_trials=200)
    print(f"branin         f* = {study.best_value:.6f} (exact 0.397887)  params = "
          f"{ {k: round(v, 4) for k, v in study.best_params.items()} }")
    best_branin = study.best_value

    study = optuna.create_study()
    study.optimize(mixed, n_trials=300)
    print(f"mixed types    f* = {study.best_value:.4f}  params = {study.best_params}")

    study = optuna.create_study()
    study.optimize(constrained, n_trials=300)
    print(f"constrained    f* = {study.best_value:.5f} (exact {3 - 2 * math.sqrt(2):.5f})  "
          f"circle = {study.best_trial.constraints['circle']:.1e}")

    study = optuna.create_study()
    study.optimize(conditional, n_trials=200)
    print(f"conditional    f* = {study.best_value:.2e}  params = "
          f"{ {k: (round(v, 4) if isinstance(v, float) else v) for k, v in study.best_params.items()} }")

    # 5. ask / tell и направление maximize.
    study = optuna.create_study(direction="maximize")
    study.enqueue_trial({"x": 1.0})                     # своя стартовая догадка
    for _ in range(100):
        trial = study.ask()
        x = trial.suggest_float("x", 0.0, 3.0)
        study.tell(trial, x * math.sin(3 * x))
    print(f"ask/tell max   f* = {study.best_value:.5f}  x = {study.best_params['x']:.4f}  "
          f"trials = {len(study.trials)}")

    print("Minimum value =", best_branin)


if __name__ == "__main__":
    main()
