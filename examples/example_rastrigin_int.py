"""Пример: задача RASTRIGIN_INT из SimpleMain.cpp

Смешанная задача: 2 непрерывные переменные (функция Растригина) +
2 квазидискретные переменные (со штрафом за отклонение от округлённых значений).

Размерность: 4
Границы: [-2.2, 1.8]^4
Оптимум: f* = 0.0 в точке (0, 0, 0, 0)
"""

import sys
from pathlib import Path
import math

ROOT = Path(__file__).resolve().parent.parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
_pyglob_dir = str(ROOT / "PYGlobalizer")
if _pyglob_dir not in sys.path:
    sys.path.insert(0, _pyglob_dir)

import _bin.PYDGlobalizer as PYDGlobalizer
from PYGlobalizer.PYProblem import PYProblem


def rastrigin_int_function(x):
    """
    Функция RASTRIGIN_INT из SimpleMain.cpp.

    Первые две переменные: классическая функция Растригина
    Последние две: штраф за отклонение от дискретных значений (округление + деление на 2)

    Parameters
    ----------
    x : list[float]
        Вектор из 4 переменных

    Returns
    -------
    float
        Значение целевой функции
    """
    pi = math.pi
    sum_val = 0.0

    # Функция Растригина для x[0] и x[1]
    for j in range(2):
        sum_val += x[j] ** 2 - 10.0 * math.cos(2.0 * pi * x[j]) + 10.0

    # Квадратичный штраф за отклонение от дискретных значений для x[2] и x[3]
    # Логика из C++: rounded = round(y[j]) / 2.0; penalty = 0.01 * (y[j] - rounded)^2
    for j in range(2, 4):
        rounded = round(x[j]) / 2.0
        sum_val += 0.01 * (x[j] - rounded) ** 2

    return sum_val


def main() -> None:
    # Создание задачи
    problem = PYProblem(dimension=4, numCriterions=1)

    # Границы: [-2.2, 1.8] для всех 4 переменных (как в C++)
    problem.set_bounds(
        [-2.2, -2.2, -2.2, -2.2],
        [1.8, 1.8, 1.8, 1.8]
    )

    # Целевая функция
    problem.add_function(rastrigin_int_function, name="RASTRIGIN_INT")

    # Известный оптимум (опционально, для оценки качества)
    problem.set_optimum(value=0.0, point=[0.0, 0.0, 0.0, 0.0])

    # Параметры решателя (увеличены для 4D задачи)
    max_iterations = 5000
    r = 4.5
    local_refine = True
    num_threads = 1

    print("=" * 70)
    print("Задача RASTRIGIN_INT")
    print("=" * 70)
    print(f"Размерность:      4 (2 непрерывные + 2 квазидискретные)")
    print(f"Границы:          [-2.2, 1.8]⁴")
    print(f"Известный оптимум: f* = 0.0 в точке (0, 0, 0, 0)")
    print(f"Итераций:         {max_iterations}")
    print(f"Параметр r:       {r}")
    print(f"Локальная догонка: {'Да' if local_refine else 'Нет'}")
    print(f"Потоков:          {num_threads}")
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
    print(f"Найденное значение:     f = {best_value:.8f}")
    print(f"Отклонение от оптимума: Δf = {abs(best_value):.2e}")
    print(f"Итераций выполнено:     {iterations}")
    print()
    print("Найденная точка:")
    print(f"  x[0] = {best_point[0]:8.5f}  (непрерывная, Растригин)")
    print(f"  x[1] = {best_point[1]:8.5f}  (непрерывная, Растригин)")
    print(f"  x[2] = {best_point[2]:8.5f}  (квазидискретная)")
    print(f"  x[3] = {best_point[3]:8.5f}  (квазидискретная)")
    print()

    # Декодирование дискретных переменных
    print("Округлённые дискретные значения:")
    for j in range(2, 4):
        rounded = round(best_point[j]) / 2.0
        deviation = best_point[j] - rounded
        print(f"  x[{j}] → round/2 = {rounded:6.3f}  "
              f"(отклонение: {deviation:+.2e}, штраф: {0.01 * deviation ** 2:.2e})")

    print("=" * 70)


if __name__ == "__main__":
    main()