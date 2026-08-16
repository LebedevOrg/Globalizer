import sys
import os
from pathlib import Path
from math import cos, pi

import numpy as np

current_dir = Path(__file__).parent.absolute()
root_dir = current_dir.parent
pyglobalizer_path = root_dir / "PYGlobalizer"
sys.path.insert(0, str(pyglobalizer_path))
sys.path.insert(0, str(root_dir))
sys.path.insert(0, str(root_dir / "_bin"))
sys.path.insert(0, str(root_dir / "examples"))

# Добавляем путь к папке Problems из репозитория Globalizer_Benchmarks
benchmarks_path = root_dir / "third_party" / "Problems" / "Problems"
print(f"Path to problems: {benchmarks_path}")
if benchmarks_path.exists():
    sys.path.insert(0, str(benchmarks_path))

print(f"Full sys path: {sys.path}")

from trial import Point, FunctionValue
from problem import Problem

from iOptProblemSimple import rastrigin

from PYProblem import PYProblem
import PYGlobalizer

p = rastrigin(2)

problem = PYProblem()
problem.copy_from_problem(p)

PYGlobalizer.solve(problem, 50, 5, False, 1)