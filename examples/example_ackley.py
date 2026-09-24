"""Пример 2. Функция Экли (Ackley) из Python — аналог C++-примера Main_ackley.cpp.

Многоэкстремальный бенчмарк: почти плоское поле с множеством мелких
локальных минимумов и одной резкой воронкой в центре.

  f(x) = -20*exp(-0.2*sqrt(mean(x_i^2))) - exp(mean(cos(2*pi*x_i))) + 20 + e

Область: x_i in [-32.768, 32.768]. Глобальный минимум f(0,...,0) = 0.

ВНИМАНИЕ: из-за огромной области поиска и почти плоского плато решатель
с маленьким max_iterations даёт грубое приближение. Для приличной точности
нужны десятки тысяч итераций, поэтому по умолчанию стоит 20000.
"""

import sys
from math import cos, exp, sqrt, pi, e
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"
BENCHMARKS_DIR = ROOT / "third_party" / "Problems" / "Problems"

for d in (BENCHMARKS_DIR, HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))

import PYDGlobalizer
from PYProblem import PYProblem


def ackley(x):
    n = len(x)
    sum_sq = sum(xi * xi for xi in x)
    sum_cos = sum(cos(2.0 * pi * xi) for xi in x)
    return -20.0 * exp(-0.2 * sqrt(sum_sq / n)) - exp(sum_cos / n) + 20.0 + e


def main() -> None:
    dim = 3

    problem = PYProblem(dimension=dim, numCriterions=1)
    problem.set_bounds([-32.768] * dim, [32.768] * dim)
    problem.add_function(ackley, name="ackley")
    if hasattr(problem, "set_optimum"):
        problem.set_optimum(0.0, [0.0] * dim)

    params = PYDGlobalizer.SolverParameters()
    params.max_iterations = 20000
    params.r = 4.5
    params.epsilon = 0.005
    params.local_refine = True
    params.verbose = 1  # только ошибки — тихий прогон

    result = PYDGlobalizer.solve(problem, params)

    if result["success"]:
        print(f"f* = {result['best_value']:.6e}  (эталон 0.0)")
        print(f"x* = {[round(v, 4) for v in result['best_point']]}")
        print(f"итераций: {result['iterations']}, решатель: {result['solver']}")
    else:
        print("Ошибка:", result["error"])


if __name__ == "__main__":
    main()
