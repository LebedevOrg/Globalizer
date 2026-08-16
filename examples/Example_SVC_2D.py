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

"""
from sklearn.svm import SVC
from sklearn.model_selection import cross_val_score
from typing import Dict
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler
"""

from iOptProblem.MachineLearning.SupportVectorMachines.SVC_2d import SVC_2D
from sklearn.datasets import load_breast_cancer
from sklearn.utils import shuffle

from PYProblem import PYProblem
import PYGlobalizer

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

    p = SVC_2D(x, y, regularization_value_bound, kernel_coefficient_bound)

    problem = PYProblem()
    problem.copy_from_problem(p)

    PYGlobalizer.solve(problem, 50, 5, False, 1)

if __name__ == "__main__":
    testSVC2D()