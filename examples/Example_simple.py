"""
Простой пример использования интерфейса для Python - задача Растригина из репозитория https://github.com/OptimLLab/Globalizer_Benchmarks
"""

import sys
import os
from pathlib import Path
from math import cos, pi

import numpy as np

# Добавляем пути
current_dir = Path(__file__).parent.absolute()
root_dir = current_dir.parent
pyglobalizer_path = root_dir / "PYGlobalizer"
sys.path.insert(0, str(pyglobalizer_path))
sys.path.insert(0, str(root_dir))
sys.path.insert(0, str(root_dir / "_bin"))
sys.path.insert(0, str(root_dir / "examples"))

# Добавляем путь к задачам
benchmarks_path = root_dir / "third_party" / "Problems" / "Problems"
if benchmarks_path.exists():
    sys.path.insert(0, str(benchmarks_path))

from trial import Point, FunctionValue
from problem import Problem

# Импортиурем задачу растригина, а также модуль интерфейса для Python 
from iOptProblemSimple import rastrigin

from PYProblem import PYProblem
import PYGlobalizer

# Создаём задачу Растригина
p = rastrigin.Rastrigin(2)

# Создаём объект-задачу для Globalizer и копируем в него задачу Растригина, созданную выше
problem = PYProblem()
problem.copy_from_problem(p)

# Вызываем решатель
PYGlobalizer.solve(problem, 50, 5, False, 1)
