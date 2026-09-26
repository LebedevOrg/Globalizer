"""Пример 1. Ручное задание задачи + новый API solve(SolverParameters).

Демонстрирует:
  - set_bounds / add_function (задача задаётся прямо в Python);
  - структуру параметров SolverParameters;
  - локальное уточнение local_refine=True;
  - уровень логирования verbose;
  - возврат результата словарём.

Целевая функция — сфера: f(x) = sum(x_i^2), минимум f(0,...,0) = 0.
"""

import sys
from pathlib import Path

# PYDGlobalizer*.pyd лежит в _bin; вспомогательные модули — в PYGlobalizer/.
# Имена различаются, поэтому коллизии импорта нет.
ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"

for d in (HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))

import PYDGlobalizer
from PYProblem import PYProblem


def main() -> None:
    dim = 4

    problem = PYProblem(dimension=dim, numCriterions=1)
    problem.set_bounds([-5.12] * dim, [5.12] * dim)
    problem.add_function(lambda x: sum(xi * xi for xi in x), name="sphere")
    if hasattr(problem, "set_optimum"):
        problem.set_optimum(0.0, [0.0] * dim)

    params = PYDGlobalizer.SolverParameters()
    params.max_iterations = 3000
    params.r = 4.0
    params.num_threads = 1
    params.epsilon = 0.01
    params.local_refine = True
    params.local_iterations = 200
    params.verbose = 2

    result = PYDGlobalizer.solve(problem, params)

    if not result["success"]:
        print("Ошибка:", result["error"])
        return

    print(f"Решатель:        {result['solver']}")
    print(f"Лучшее значение: {result['best_value']:.6e}")
    print(f"Точка:           {[round(v, 5) for v in result['best_point']]}")
    print(f"Итераций:        {result['iterations']}")


if __name__ == "__main__":
    main()
