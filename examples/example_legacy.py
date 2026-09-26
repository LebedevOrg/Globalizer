"""Пример 4. Старый интерфейс solve_legacy — возвращает тот же dict, что solve().

Сигнатура сохранена для обратной совместимости:
    solve_legacy(problem, maxParams, r, localRefineSolution, numThreads)
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"

for d in (HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))

import PYDGlobalizer
from PYProblem import PYProblem


def main() -> None:
    problem = PYProblem(dimension=2, numCriterions=1)
    problem.set_bounds([-5, -5], [5, 5])
    problem.add_function(lambda x: x[0] ** 2 + x[1] ** 2, name="sphere2d")

    # Старый позиционный вызов — теперь ловим возвращаемый словарь:
    result = PYDGlobalizer.solve_legacy(problem, 2000, 4.0, False, 1)

    print("best_value:", result["best_value"])
    print("best_point:", [round(v, 5) for v in result["best_point"]])
    print("iterations:", result["iterations"])


if __name__ == "__main__":
    main()
