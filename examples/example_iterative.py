"""Пример: пошаговая (итеративная) работа Globalizer из Python.

Аналог examples/IterationMain.cpp. Решатель создаётся один раз, а итерации
выполняются по одной: между ними можно менять данные, от которых зависит
функция задачи, смотреть текущий результат и решать, продолжать ли поиск.

    solver = PYDGlobalizer.IterativeSolver(problem, params)  # Solver + Initialize()
    finished = solver.do_iteration()                         # DoIteration(finished)
    solver.result()                                          # GetSolutionResult()
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
for d in (ROOT / "PYGlobalizer", ROOT / "_bin"):
    if d.exists():
        sys.path.insert(0, str(d))

import PYDGlobalizer                      # noqa: E402
from PYProblem import PYProblem           # noqa: E402


def iteration_main():
    """Повтор IterationMain.cpp: f(x) = x^2 * data, data меняется на каждой итерации."""
    data = {"value": 100.0}

    def calculate_functional(x):
        return x[0] * x[0] * data["value"]

    problem = PYProblem(dimension=1)
    problem.set_bounds([0.0], [10.0])
    problem.add_function(calculate_functional)

    params = PYDGlobalizer.SolverParameters()
    params.stop_condition = "iterations"   # как parameters.StopCondition = IterationOnly
    params.max_iterations = 300
    params.verbose = 0

    solver = PYDGlobalizer.IterativeSolver(problem, params)
    for it in range(300):
        data["value"] = it + 1.0           # данные задачи меняются между итерациями
        if solver.do_iteration():
            break
        if it % 100 == 0:
            r = solver.result()
            print(f"iteration {r['iterations']:4d}: best x = {r['best_point']}, "
                  f"f = {r['best_value']}")

    solver.print_result()
    return solver.result()


def iterate_until_accuracy():
    """Поиск до достижения точности; итерации - через протокол итератора."""
    problem = PYProblem(dimension=2)
    problem.set_bounds([-1.0, -1.0], [1.0, 1.0])
    problem.add_function(lambda x: (x[0] - 0.3) ** 2 + (x[1] + 0.2) ** 2)

    params = PYDGlobalizer.SolverParameters()
    params.max_iterations = 5000
    params.epsilon = 0.01
    params.verbose = 0
    params.print_result = False

    state = None
    for state in PYDGlobalizer.IterativeSolver(problem, params):
        # Пользовательский критерий остановки поверх критерия Globalizer.
        if state["best_value"] is not None and state["best_value"] < 1e-4:
            break
    print(f"stopped after {state['iterations']} iterations: f* = {state['best_value']:.3e}, "
          f"x* = {[round(v, 4) for v in state['best_point']]}")
    return state


if __name__ == "__main__":
    iteration_main()
    final = iterate_until_accuracy()
    # Итоги print_result() печатает C++ (printf) и могут оказаться в выводе
    # позже этой строки - поэтому итог помечен отдельным ключом best_value.
    print("best_value =", final["best_value"])
