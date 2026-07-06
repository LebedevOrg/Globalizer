import numpy as np
from trial import Point
from trial import FunctionValue
from problem import Problem
from sklearn.svm import SVC
from sklearn.model_selection import cross_val_score
from typing import Dict
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler

from examples.Machine_learning.SVC._2D.Problems import SVC_2d
from sklearn.datasets import load_breast_cancer
from sklearn.utils import shuffle

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

"""
Call problem here
"""

def load_breast_cancer_data():
    dataset = load_breast_cancer()
    x_raw, y_raw = dataset['data'], dataset['target']
    inputs, outputs = shuffle(x_raw, y_raw ^ 1, random_state=42)
    return inputs, outputs

def testSVC2D():
    x, y = load_breast_cancer_data()
    regularization_value_bound = {'low': 1, 'up': 6}
    kernel_coefficient_bound = {'low': -7, 'up': -3}

    p = SVC_2d.SVC_2D(x, y, regularization_value_bound, kernel_coefficient_bound)

    problem = PYProblem()
    problem.copy_from_problem(p)

    PYGlobalizer.solve(problem, 50, 5, False, 1)

if __name__ == "__main__":
    #TestSVC3D()
    #TestsProblemTest()
    #test_ecg_classification_problem()
    #test_svc1d_problem()
    #test_segmentation_problem()
    testSVC2D()
