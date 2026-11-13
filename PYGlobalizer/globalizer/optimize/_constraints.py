"""Границы и ограничения в стиле scipy.optimize.

Классы ``Bounds``, ``NonlinearConstraint`` и ``LinearConstraint`` повторяют
конструкторы scipy. Функции нормализации принимают как эти классы, так и
объекты самого scipy (по «утиной типизации»: атрибуты ``lb``/``ub``, ``fun``,
``A``), а также ограничения-словари ``{'type': 'ineq'|'eq', 'fun': ..., 'args': ...}``.

Globalizer работает с ограничениями вида g(x) <= 0, поэтому каждое
ограничение scipy раскладывается на скалярные функции g_j(x) <= 0.
"""

import threading

import numpy as np


class Bounds:
    """Границы переменных: ``lb <= x <= ub`` (аналог ``scipy.optimize.Bounds``)."""

    def __init__(self, lb=-np.inf, ub=np.inf, keep_feasible=False):
        self.lb = lb
        self.ub = ub
        self.keep_feasible = keep_feasible

    def __repr__(self):
        return f"Bounds({self.lb!r}, {self.ub!r})"


class NonlinearConstraint:
    """Нелинейное ограничение ``lb <= fun(x) <= ub``.

    Параметры ``jac``, ``hess``, ``keep_feasible``, ``finite_diff_*``
    принимаются для совместимости и не используются: Globalizer не требует
    производных.
    """

    def __init__(self, fun, lb, ub, jac="2-point", hess=None, keep_feasible=False,
                 finite_diff_rel_step=None, finite_diff_jac_sparsity=None):
        self.fun = fun
        self.lb = lb
        self.ub = ub
        self.jac = jac
        self.hess = hess
        self.keep_feasible = keep_feasible
        self.finite_diff_rel_step = finite_diff_rel_step
        self.finite_diff_jac_sparsity = finite_diff_jac_sparsity


class LinearConstraint:
    """Линейное ограничение ``lb <= A.dot(x) <= ub``."""

    def __init__(self, A, lb=-np.inf, ub=np.inf, keep_feasible=False):
        self.A = A
        self.lb = lb
        self.ub = ub
        self.keep_feasible = keep_feasible


# ---------------------------------------------------------------------------
#  Границы
# ---------------------------------------------------------------------------
def normalize_bounds(bounds):
    """Возвращает (lb, ub) как массивы float64.

    Допускаются: последовательность пар ``(min, max)`` и объект с атрибутами
    ``lb``/``ub`` (``Bounds`` этого модуля или scipy).
    """
    if bounds is None:
        raise ValueError("Globalizer requires finite bounds for every variable")
    if hasattr(bounds, "lb") and hasattr(bounds, "ub") and not hasattr(bounds, "fun") \
            and not hasattr(bounds, "A"):
        lb = np.atleast_1d(np.asarray(bounds.lb, dtype=float))
        ub = np.atleast_1d(np.asarray(bounds.ub, dtype=float))
        lb, ub = np.broadcast_arrays(lb, ub)
    else:
        pairs = list(bounds)
        if len(pairs) == 0:
            raise ValueError("bounds must not be empty")
        lb = np.empty(len(pairs))
        ub = np.empty(len(pairs))
        for i, pair in enumerate(pairs):
            try:
                lo, hi = pair
            except (TypeError, ValueError):
                raise ValueError(
                    "bounds should be a sequence of (min, max) pairs or a Bounds object") from None
            lb[i] = -np.inf if lo is None else float(lo)
            ub[i] = np.inf if hi is None else float(hi)
    lb = np.array(lb, dtype=float)
    ub = np.array(ub, dtype=float)
    if lb.ndim != 1:
        raise ValueError("bounds must be one-dimensional")
    if not (np.all(np.isfinite(lb)) and np.all(np.isfinite(ub))):
        raise ValueError("Globalizer requires finite bounds for every variable "
                         "(None and inf are not allowed)")
    if np.any(lb > ub):
        raise ValueError("lower bound must not exceed upper bound in bounds")
    return lb, ub


# ---------------------------------------------------------------------------
#  Ограничения
# ---------------------------------------------------------------------------
class _CachedFunction:
    """Вычисляет векторную функцию ограничения один раз на точку.

    Globalizer вызывает скалярные функции g_j по очереди, а ограничение scipy
    возвращает сразу вектор — кэш не даёт пересчитывать его для каждой
    компоненты.
    """

    _MAX_CACHE = 256

    def __init__(self, fun):
        self._fun = fun
        self._cache = {}
        self._lock = threading.Lock()

    def __call__(self, x):
        key = x.tobytes()
        with self._lock:
            hit = self._cache.get(key)
        if hit is not None:
            return hit
        value = np.atleast_1d(np.asarray(self._fun(x), dtype=float)).ravel()
        with self._lock:
            if len(self._cache) >= self._MAX_CACHE:
                self._cache.clear()
            self._cache[key] = value
        return value


class ScalarConstraint:
    """Скалярное ограничение Globalizer: ``g(x) <= 0``.

    ``value(x)`` — значение g (с учётом допуска для равенств),
    ``violation(x)`` — фактическое нарушение исходного ограничения (>= 0).
    """

    __slots__ = ("_vec", "_index", "_kind", "_bound", "_eq_tol")

    def __init__(self, vec, index, kind, bound, eq_tol):
        self._vec = vec          # _CachedFunction
        self._index = index      # номер компоненты
        self._kind = kind        # 'lb' | 'ub' | 'eq'
        self._bound = bound
        self._eq_tol = eq_tol

    def _raw(self, x):
        v = self._vec(x)[self._index]
        if self._kind == "lb":
            return self._bound - v
        if self._kind == "ub":
            return v - self._bound
        return abs(v - self._bound)

    def value(self, x):
        g = self._raw(x)
        if self._kind == "eq":
            g -= self._eq_tol
        return g

    def violation(self, x):
        g = self._raw(x)
        if not np.isfinite(g):
            return np.inf
        return max(0.0, g)


def _split(vec, lb, ub, x_probe, eq_tol):
    """Раскладывает ``lb <= vec(x) <= ub`` на скалярные ограничения."""
    m = vec(x_probe).size
    lb = np.broadcast_to(np.asarray(lb, dtype=float), (m,))
    ub = np.broadcast_to(np.asarray(ub, dtype=float), (m,))
    result = []
    for j in range(m):
        if lb[j] > ub[j]:
            raise ValueError("constraint lower bound must not exceed the upper bound")
        if lb[j] == ub[j]:
            result.append(ScalarConstraint(vec, j, "eq", lb[j], eq_tol))
            continue
        if np.isfinite(lb[j]):
            result.append(ScalarConstraint(vec, j, "lb", lb[j], eq_tol))
        if np.isfinite(ub[j]):
            result.append(ScalarConstraint(vec, j, "ub", ub[j], eq_tol))
    return result


def normalize_constraints(constraints, x_probe, eq_tol):
    """Преобразует ограничения scipy в список ``ScalarConstraint``.

    ``x_probe`` — допустимая по границам точка, в которой один раз
    вычисляются векторные ограничения, чтобы узнать их размерность.
    """
    if constraints is None:
        return []
    if isinstance(constraints, (dict, NonlinearConstraint, LinearConstraint, Bounds)) \
            or hasattr(constraints, "fun") or hasattr(constraints, "A") \
            or (hasattr(constraints, "lb") and hasattr(constraints, "ub")):
        constraints = [constraints]

    result = []
    for c in constraints:
        if isinstance(c, dict):
            ctype = str(c.get("type", "")).lower()
            fun = c.get("fun")
            if fun is None:
                raise ValueError("constraint dict must contain 'fun'")
            cargs = tuple(c.get("args", ()))
            vec = _CachedFunction(lambda x, f=fun, a=cargs: f(x, *a))
            if ctype == "ineq":            # fun(x) >= 0
                result += _split(vec, 0.0, np.inf, x_probe, eq_tol)
            elif ctype == "eq":            # fun(x) == 0
                result += _split(vec, 0.0, 0.0, x_probe, eq_tol)
            else:
                raise ValueError(f"Unknown constraint type '{c.get('type')}'")
        elif hasattr(c, "fun"):           # NonlinearConstraint
            vec = _CachedFunction(c.fun)
            result += _split(vec, c.lb, c.ub, x_probe, eq_tol)
        elif hasattr(c, "A"):             # LinearConstraint
            A = np.atleast_2d(np.asarray(c.A, dtype=float))
            vec = _CachedFunction(lambda x, A=A: A @ x)
            result += _split(vec, c.lb, c.ub, x_probe, eq_tol)
        elif hasattr(c, "lb") and hasattr(c, "ub"):   # Bounds как ограничение
            vec = _CachedFunction(lambda x: np.asarray(x, dtype=float))
            result += _split(vec, c.lb, c.ub, x_probe, eq_tol)
        else:
            raise TypeError(f"Unsupported constraint type: {type(c).__name__}")
    return result
