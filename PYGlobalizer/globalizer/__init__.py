"""Globalizer - система глобальной оптимизации (Python-интерфейс).

Модуль ``globalizer.optimize`` повторяет интерфейс ``scipy.optimize`` для
задач глобальной оптимизации: достаточно заменить ``scipy`` на ``globalizer``.

Модуль ``globalizer.optuna`` повторяет интерфейс Optuna (study / trial /
suggest_*): достаточно заменить ``import optuna`` на
``import globalizer.optuna as optuna``.
"""

from . import optimize
from . import optuna

__all__ = ["optimize", "optuna"]
__version__ = "0.2.0"
