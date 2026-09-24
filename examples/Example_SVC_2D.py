"""Пример: подбор гиперпараметров SVC (2 параметра) через Globalizer.

ЧТО ИСПРАВЛЕНО ОТНОСИТЕЛЬНО ИСХОДНОГО ФАЙЛА:
  1) была неопределённая переменная benchmarks_path -> NameError;
     теперь путь строится явно;
  2) было `import PYGlobalizer` и позиционный `solve(problem, 50, 5, False, 1)`;
     модуль переименован в PYDGlobalizer, а позиционная форма вызова
     перенесена в solve_legacy() (у solve() теперь сигнатура (problem, params)).

ВНЕШНИЕ ЗАВИСИМОСТИ (не входят в поставку Globalizer):
  - пакет iOptProblem (откуда берётся класс SVC_2D);
  - scikit-learn (датасет load_breast_cancer).
Если они не установлены, скрипт напечатает понятную ошибку, а не трейсбек
в недрах импорта.
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"
BENCHMARKS_DIR = ROOT / "third_party" / "Problems" / "Problems"

# Каталоги с .pyd и вспомогательными модулями — в начало пути.
for d in (HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))
if BENCHMARKS_DIR.exists():
    sys.path.insert(0, str(BENCHMARKS_DIR))

from trial import Point, FunctionValue   # noqa: F401  (нужны для совместимости)
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


def testSVC2D():
    try:
        from iOptProblem.MachineLearning.SupportVectorMachines.SVC_2d import SVC_2D
    except ImportError as exc:
        print("[SKIP] Не найден пакет iOptProblem (SVC_2D). Установите его и повторите.")
        print("       Причина:", exc)
        return

    x, y = load_breast_cancer_data()
    regularization_value_bound = {"low": 1, "up": 6}
    kernel_coefficient_bound = {"low": -7, "up": -3}

    # Задача-источник (иностранный интерфейс) и её копия в формат Globalizer.
    p = SVC_2D(x, y, regularization_value_bound, kernel_coefficient_bound)
    problem = PYProblem()
    problem.copy_from_problem(p)

    # Позиционная форма — теперь через solve_legacy (solve() принимает params-объект).
    result = PYDGlobalizer.solve_legacy(problem, 50, 5, False, 1)

    if result.get("success"):
        print("f* =", result["best_value"])
        print("x* =", [round(v, 5) for v in result["best_point"]])
    else:
        print("Ошибка:", result.get("error"))


if __name__ == "__main__":
    testSVC2D()
