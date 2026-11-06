"""Globalizer - система глобальной оптимизации (Python-интерфейс).

Модуль ``globalizer.optimize`` повторяет интерфейс ``scipy.optimize`` для
задач глобальной оптимизации: достаточно заменить ``scipy`` на ``globalizer``.
"""

from . import optimize

__all__ = ["optimize"]
__version__ = "0.1.0"
