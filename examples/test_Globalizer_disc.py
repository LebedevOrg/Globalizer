import numpy as np

import importlib.util
import sys
import os
from pathlib import Path
from math import cos, pi

# Определяем путь к текущему скрипту (он в examples)
current_dir = Path(__file__).parent.absolute()

# Поднимаемся на два уровня вверх: examples -> fork -> Globalizer
# Или на один уровень, если структура другая
root_dir = current_dir.parent.parent  # из examples в Globalizer


# Или можно подняться до папки Globalizer, ища её
def find_globalizer_root(start_path):
    """Ищем корневую директорию Globalizer"""
    current = start_path
    for _ in range(5):  # поднимаемся максимум на 5 уровней
        if (current / "PYGlobalizer").exists() and (current / "_bin").exists():
            return current
        current = current.parent
    return start_path.parent.parent  # fallback


root_dir = find_globalizer_root(current_dir)
print(f"Корневая директория: {root_dir}")

# Путь к PYProblem.py (Globalizer/PYGlobalizer/PYProblem.py)
pyproblem_path = root_dir / "PYGlobalizer" / "PYProblem.py"

# Проверяем существование файла
if not pyproblem_path.exists():
    raise FileNotFoundError(f"PYProblem.py не найден по пути: {pyproblem_path}")

# Загрузка модуля PYProblem
spec = importlib.util.spec_from_file_location(
    "PYProblem",
    str(pyproblem_path)
)

module = importlib.util.module_from_spec(spec)
sys.modules["PYProblem"] = module
spec.loader.exec_module(module)
PYProblem = module.PYProblem

# Путь к PYGlobalizer (Globalizer/_bin)
module_dir = root_dir / "_bin"

# Добавляем директорию с PYGlobalizer в sys.path
sys.path.insert(0, str(module_dir))

# Импортируем PYGlobalizer
import PYGlobalizer

print(f"PYProblem загружен из: {pyproblem_path}")
print(f"PYGlobalizer загружен из: {module_dir}")


def rastrigin(x):
    return sum(xi ** 2 - 10 * np.cos(2 * np.pi * xi) + 10 for xi in x)


print("Started")
problem = PYProblem(dimension=1)
print('Created problem')
problem.add_function(rastrigin, name='rastrigin')
problem.set_bounds(
    lower=[-5.0],
    upper=[5.0]
)

PYGlobalizer.solve(problem)
print('Solved!')
