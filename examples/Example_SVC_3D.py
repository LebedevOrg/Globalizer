"""Пример: подбор гиперпараметров SVC (3 параметра) через Globalizer.

ЧТО ИСПРАВЛЕНО ОТНОСИТЕЛЬНО ИСХОДНОГО ФАЙЛА:
  - модуль переименован: `import PYGlobalizer` -> `import PYDGlobalizer`;
  - позиционный вызов `solve(problem, 3, 5, False, 1)` заменён на
    `solve_legacy(...)`, т.к. у solve() теперь сигнатура (problem, params);
  - внешние зависимости (iOptProblem, scikit-learn) проверяются с понятным
    сообщением вместо длинного трейсбека.

ВНЕШНИЕ ЗАВИСИМОСТИ: пакет iOptProblem (класс SVC_3D) и scikit-learn.
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"
BENCHMARKS_DIR = ROOT / "third_party" / "Problems" / "Problems"

for d in (HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))
if BENCHMARKS_DIR.exists():
    sys.path.insert(0, str(BENCHMARKS_DIR))

from trial import Point, FunctionValue   # noqa: F401
from problem import Problem              # noqa: F401

from PYProblem import PYProblem
import PYDGlobalizer


def load_breast_cancer_data():
    from sklearn.datasets import load_breast_cancer
    from sklearn.utils import shuffle

    dataset = load_breast_cancer()
    x_raw, y_raw = dataset["data"], dataset["target"]
    inputs, outputs = shuffle(x_raw, y_raw ^ 1, random_state=42)
    return inputs, outputs


def main():
    try:
        from iOptProblem.MachineLearning.SupportVectorMachines.SVC_3D import SVC_3D
    except ImportError as exc:
        print("[SKIP] Не найден пакет iOptProblem (SVC_3D). Установите его и повторите.")
        print("       Причина:", exc)
        return

    x, y = load_breast_cancer_data()
    regularization_value_bound = {"low": 1, "up": 10}
    kernel_coefficient_bound = {"low": -9, "up": -6.7}
    kernel_type = {"kernel": ["rbf", "sigmoid", "poly"]}

    p = SVC_3D(x, y, regularization_value_bound, kernel_coefficient_bound, kernel_type)

    problem = PYProblem(dimension=3)
    problem.copy_from_problem(p)

    # Позиционная форма — через solve_legacy (сохраняет прежний смысл вызова).
    result = PYDGlobalizer.solve_legacy(problem, 3, 5, False, 1)

    if result.get("success"):
        print("f* =", result["best_value"])
        print("x* =", [round(v, 5) for v in result["best_point"]])
    else:
        print("Ошибка:", result.get("error"))


if __name__ == "__main__":
    main()
