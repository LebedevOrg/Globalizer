"""Пример: интерфейс globalizer.optimize, совместимый с scipy.optimize.

Код ниже написан для scipy. Чтобы решить те же задачи Globalizer'ом,
достаточно заменить имя библиотеки в импорте:

    from scipy.optimize import ...      ->   from globalizer.optimize import ...

Запуск:  python examples/example_scipy_interface.py [scipy]
(с аргументом scipy - те же задачи решает scipy, для сравнения).
"""

import sys
from pathlib import Path

import numpy as np

if len(sys.argv) > 1 and sys.argv[1] == "scipy":
    from scipy.optimize import (differential_evolution, dual_annealing, shgo, direct,
                                NonlinearConstraint)
else:
    # Папка с пакетом globalizer (PYGlobalizer); PYDGlobalizer ищется в _bin.
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "PYGlobalizer"))
    from globalizer.optimize import (differential_evolution, dual_annealing, shgo, direct,
                                     NonlinearConstraint)


def rastrigin(x):
    return 10 * len(x) + np.sum(x ** 2 - 10 * np.cos(2 * np.pi * x))


def main():
    bounds = [(-5.12, 5.12)] * 2

    # 1. Многоэкстремальная функция Растригина, глобальный минимум 0 в (0, 0).
    for method in (differential_evolution, dual_annealing, shgo, direct):
        res = method(rastrigin, bounds)
        print(f"{method.__name__:24s} f* = {res.fun:.3e}  x* = {np.round(res.x, 4)}  "
              f"nfev = {res.nfev}")

    # 2. Задача с ограничением: min (x-1)^2 + (y-1)^2 при x^2 + y^2 <= 1.
    circle = NonlinearConstraint(lambda x: x[0] ** 2 + x[1] ** 2, -np.inf, 1.0)
    res = differential_evolution(lambda x: (x[0] - 1) ** 2 + (x[1] - 1) ** 2,
                                 [(-2, 2)] * 2, constraints=circle)
    print(f"constrained              f* = {res.fun:.5f} (exact {3 - 2 * np.sqrt(2):.5f})  "
          f"x* = {np.round(res.x, 4)}")

    # 3. Частично целочисленная задача: x0, x2 - целые.
    res = differential_evolution(
        lambda x: (x[0] - 1.3) ** 2 + (x[1] + 2.6) ** 2 + (x[2] - 0.4) ** 2,
        [(-5, 5)] * 3, integrality=[True, False, True])
    print(f"mixed-integer            f* = {res.fun:.5f}  x* = {np.round(res.x, 4)}")

    print("Minimum value =", res.fun)


if __name__ == "__main__":
    main()
