"""Пример: задача RASTRIGIN_INT из SimpleMain.cpp

Смешанная задача с частично целочисленными параметрами:
- 2 непрерывные переменные x[0], x[1]: функция Растригина
- 2 дискретные переменные x[2], x[3]: каждая принимает значения {-1, 0, 1}

Размерность: 4 (2 непрерывные + 2 дискретные)
Границы непрерывных: [-2.2, 1.8]
Дискретные значения: {-1, 0, 1} для каждой
Оптимум: f* = 0.0 в точке (0, 0, 0, 0)
"""

import sys
from pathlib import Path
import math

ROOT = Path(__file__).resolve().parent.parent
BIN_DIR = ROOT / "_bin"
HELPERS_DIR = ROOT / "PYGlobalizer"
BENCHMARKS_DIR = ROOT / "third_party" / "Problems" / "Problems"

for d in (BENCHMARKS_DIR, HELPERS_DIR, BIN_DIR):
    if d.exists():
        sys.path.insert(0, str(d))

import PYDGlobalizer
from PYProblem import PYProblem


def rastrigin_int_function(x):
    """
    Функция RASTRIGIN_INT из SimpleMain.cpp.

    x[0:2] - непрерывные переменные (функция Растригина)
    x[2:4] - дискретные переменные со штрафом за отклонение
    """
    pi = math.pi
    sum_val = 0.0

    # Функция Растригина для первых двух непрерывных переменных
    for j in range(2):
        sum_val += x[j]**2 - 10.0 * math.cos(2.0 * pi * x[j]) + 10.0

    # Штраф за отклонение от дискретных значений для x[2] и x[3]
    for j in range(2, 4):
        rounded = round(x[j]) / 2.0
        sum_val += 0.01 * (x[j] - rounded) ** 2

    return sum_val


def main() -> None:
    # КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ: dimension=2 задаёт только непрерывные переменные
    # Общая размерность станет 4 после добавления дискретных
    problem = PYProblem(dimension=2, numCriterions=1)

    # Границы для непрерывных переменных (2 переменные)
    problem.set_bounds([-2.2, -2.2], [1.8, 1.8])

    # Установка дискретных переменных (2 переменные)
    problem.set_discrete_variables(
        [["-1", "0", "1"], ["-1", "0", "1"]],
        ["discrete_1", "discrete_2"]
    )

    # Теперь общая размерность: 2 + 2 = 4
    print(f"Общая размерность: {problem._dimension}")

    # Целевая функция
    problem.add_function(rastrigin_int_function, name="RASTRIGIN_INT")

    # Известный оптимум
    problem.set_optimum(value=0.0, point=[0.0, 0.0, 0.0, 0.0])

    # Параметры решателя
    max_iterations = 5000
    r = 4.5
    local_refine = True
    num_threads = 1

    print("=" * 70)
    print("Задача RASTRIGIN_INT (смешанная оптимизация)")
    print("=" * 70)
    print(f"Размерность:         4 (2 непрерывные + 2 дискретные)")
    print(f"Непрерывные:         x[0], x[1] ∈ [-2.2, 1.8]")
    print(f"Дискретные:          x[2], x[3] ∈ {{-1, 0, 1}}")
    print(f"Известный оптимум:   f* = 0.0 в точке (0, 0, 0, 0)")
    print(f"Итераций:            {max_iterations}")
    print(f"Параметр r:          {r}")
    print(f"Локальная догонка:   {'Да' if local_refine else 'Нет'}")
    print(f"Потоков:             {num_threads}")
    print("-" * 70)

    # Решение
    result = PYDGlobalizer.solve_legacy(
        problem,
        max_iterations,
        r,
        local_refine,
        num_threads
    )

    # Проверка успешности
    if not result.get("success", False):
        raise RuntimeError(f"Ошибка решателя: {result.get('error', 'unknown')}")

    # Результаты
    best_value = result["best_value"]
    best_point = result["best_point"]
    iterations = result["iterations"]

    print("\nРезультаты оптимизации:")
    print("-" * 70)
    print(f"Найденное значение:      f = {best_value:.8f}")
    print(f"Отклонение от оптимума:  Δf = {abs(best_value):.2e}")
    print(f"Итераций выполнено:      {iterations}")
    print()
    print("Найденная точка:")
    print(f"  x[0] = {best_point[0]:8.5f}  (непрерывная, Растригин)")
    print(f"  x[1] = {best_point[1]:8.5f}  (непрерывная, Растригин)")
    print(f"  x[2] = {best_point[2]:8.5f}  (дискретная)")
    print(f"  x[3] = {best_point[3]:8.5f}  (дискретная)")
    print()

    # Декодирование дискретных переменных
    print("Округлённые дискретные значения:")
    for j in range(2, 4):
        rounded = round(best_point[j]) / 2.0
        deviation = best_point[j] - rounded
        nearest = int(round(best_point[j]))
        nearest = max(-1, min(1, nearest))
        print(f"  x[{j}] = {best_point[j]:7.4f} → округление/2 = {rounded:6.3f}, "
              f"ближайшее {{-1,0,1}} = {nearest:2d}, штраф = {0.01 * deviation**2:.2e}")

    print("=" * 70)


if __name__ == "__main__":
    main()