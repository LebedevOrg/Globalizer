"""globalizer.optimize - глобальная оптимизация Globalizer с интерфейсом scipy.optimize.

Чтобы перевести код со scipy на Globalizer, достаточно заменить имя библиотеки:

    from scipy.optimize import differential_evolution    # было
    from globalizer.optimize import differential_evolution  # стало

    import scipy.optimize as opt        ->  import globalizer.optimize as opt
    from scipy import optimize          ->  from globalizer import optimize

Функции: differential_evolution, dual_annealing, shgo, direct, basinhopping,
minimize. Классы: OptimizeResult, Bounds, NonlinearConstraint,
LinearConstraint, OptimizeWarning. Настройки Globalizer: set_options().
"""

from ._optimize import OptimizeResult, OptimizeWarning
from ._constraints import Bounds, NonlinearConstraint, LinearConstraint
from ._global import (differential_evolution, dual_annealing, shgo, direct,
                      basinhopping, minimize, set_options, get_options)

__all__ = [
    "differential_evolution", "dual_annealing", "shgo", "direct", "basinhopping",
    "minimize", "OptimizeResult", "OptimizeWarning", "Bounds", "NonlinearConstraint",
    "LinearConstraint", "set_options", "get_options",
]
