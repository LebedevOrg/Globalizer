"""Простой пример: задача Растригина через Python-интерфейс Globalizer.

Задача Растригина берётся из бенчмарков (iOptProblemSimple) и копируется
в объект PYProblem через copy_from_problem().
"""

import sys
from pathlib import Path

current_dir = Path(__file__).resolve().parent
root_dir = current_dir.parent
helpers_dir = root_dir / "PYGlobalizer"          # PYProblem.py, trial.py, problem.py
bin_dir = root_dir / "_bin"                       # PYDGlobalizer*.pyd
benchmarks_dir = root_dir / "third_party" / "Problems" / "Problems"

# PYProblem.py и trial.py — Python-вспомогательные модули.
sys.path.insert(0, str(helpers_dir))
# PYDGlobalizer*.pyd — отдельное C++-расширение с другим именем.
sys.path.insert(0, str(bin_dir))
if benchmarks_dir.exists():
    sys.path.insert(0, str(benchmarks_dir))

from iOptProblemSimple import rastrigin
from PYProblem import PYProblem
import PYDGlobalizer


def main():
    source_problem = rastrigin.Rastrigin(2)

    problem = PYProblem()
    problem.copy_from_problem(source_problem)

    params = PYDGlobalizer.SolverParameters()
    params.max_iterations = 50
    params.r = 5.0
    params.num_threads = 1
    params.epsilon = 0.01

    result = PYDGlobalizer.solve(problem, params)
    print("PYDGlobalizer:", PYDGlobalizer.__file__)
    print("Result:", result)


if __name__ == "__main__":
    main()
