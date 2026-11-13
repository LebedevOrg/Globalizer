"""Результат оптимизации и предупреждения в стиле scipy.optimize."""

import numpy as np


class OptimizeResult(dict):
    """Результат оптимизации — аналог ``scipy.optimize.OptimizeResult``.

    Словарь с доступом к полям как к атрибутам. Основные поля:

    x : ndarray
        Найденное решение.
    fun : float
        Значение целевой функции в ``x``.
    success : bool
        Найдено ли допустимое решение.
    status : int
        Код завершения (0 - достигнута точность, 1 - исчерпан лимит
        вычислений, 2 - остановка из callback, 3 - не найдено допустимой точки).
    message : str
        Описание причины завершения.
    nfev : int
        Число вычислений целевой функции.
    nit : int
        Число итераций метода Globalizer.
    maxcv, constr_violation : float
        Максимальное нарушение ограничений (если они заданы).
    """

    def __getattr__(self, name):
        try:
            return self[name]
        except KeyError as e:
            raise AttributeError(name) from e

    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__

    def __repr__(self):
        if not self:
            return self.__class__.__name__ + "()"
        width = max(map(len, self.keys())) + 1
        lines = []
        for k, v in self.items():
            if isinstance(v, np.ndarray):
                text = np.array2string(v, precision=4, threshold=10, separator=" ")
            else:
                text = repr(v)
            lines.append(k.rjust(width) + ": " + text.replace("\n", "\n" + " " * (width + 2)))
        return "\n".join(lines)

    def __dir__(self):
        return list(self.keys())


class OptimizeWarning(UserWarning):
    """Предупреждение о параметрах, которые Globalizer не использует."""
