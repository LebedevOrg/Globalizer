"""Пример 3. Задача с ограничением + проверка через validate_problem.

Порядок функций в PYProblem: сначала numCriterions критериев, затем ограничения.
Здесь: 1 критерий + 1 ограничение (n_constraints = len(_functions) - numCriterions).

Критерий:    f(x)  = (x0 - 1)^2 + (x1 - 1)^2      -> минимум в (1, 1)
Ограничение: g(x)  = x0^2 + x1^2 - 1  <= 0        -> точка обязана лежать в круге R=1

Из-за ограничения безусловный минимум (1,1) недостижим; решение уедет
на границу круга в направлении (1,1), т.е. примерно (0.707, 0.707).
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
    problem.set_bounds([-2.0, -2.0], [2.0, 2.0])

    # ВАЖЕН порядок: критерий добавляется первым, ограничение — следом.
    problem.add_function(lambda x: (x[0] - 1) ** 2 + (x[1] - 1) ** 2, name="objective")
    problem.add_function(lambda x: x[0] ** 2 + x[1] ** 2 - 1.0,       name="g_circle")

    # Сначала валидируем без запуска решателя:
    info = PYDGlobalizer.validate_problem(problem)
    print("validate:", info)
    if not info["valid"]:
        return

    params = PYDGlobalizer.SolverParameters()
    params.max_iterations = 4000
    params.r = 3.5
    params.epsilon = 0.01
    params.verbose = 2

    result = PYDGlobalizer.solve(problem, params)

    if result["success"]:
        x = result["best_point"]
        print(f"f* = {result['best_value']:.6f}")
        print(f"x* = {[round(v, 4) for v in x]}  (ожидание ~[0.707, 0.707])")
        print(f"проверка ограничения g(x*) = {x[0]**2 + x[1]**2 - 1:.4f}  (<= 0)")
    else:
        print("Ошибка:", result["error"])


if __name__ == "__main__":
    main()
