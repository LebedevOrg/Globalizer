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

from trial import Point, FunctionValue
from problem import Problem

from sklearn.svm import SVC
from sklearn.model_selection import cross_val_score
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.datasets import load_breast_cancer
from sklearn.utils import shuffle
from typing import Dict

from iOpt.output_system.listeners.console_outputers import ConsoleOutputListener
from iOpt.output_system.listeners.static_painters import StaticDiscreteListener
from iOpt.solver import Solver
from iOpt.solver_parametrs import SolverParameters

from examples.Machine_learning.SVC._3D.Problem import SVC_3D

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


if __name__ == "__main__":
    x, y = load_breast_cancer_data()
    regularization_value_bound = {'low': 1, 'up': 10}
    kernel_coefficient_bound = {'low': -9, 'up': -6.7}
    kernel_type = {'kernel': ['rbf', 'sigmoid', 'poly']}
    p = SVC_3D.SVC_3D(x, y, regularization_value_bound, kernel_coefficient_bound, kernel_type)

    problem = PYProblem(dimension=3)
    problem.copy_from_problem(p)

    PYGlobalizer.solve(problem, 3, 5, False, 1)
