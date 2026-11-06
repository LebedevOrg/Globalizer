"""Функции глобальной оптимизации с сигнатурами scipy.optimize.

Все функции решают задачу индексным методом Globalizer. Параметры, задающие
внутреннее устройство алгоритмов scipy (strategy, mutation, initial_temp,
sampling_method, ...), принимаются для совместимости и не используются.

Настройки Globalizer задаются:
  * для одного вызова - аргументом ``globalizer_options={...}``;
  * для всех вызовов - функцией ``set_options(...)``.
Ключи: r, eps, maxfev, local_search, local_iterations, eq_tol, threads, disp
(см. ``set_options``).
"""

import inspect

import numpy as np

from . import _solver
from ._constraints import normalize_bounds
from ._optimize import OptimizeResult

_defaults = {}


def set_options(**options):
    """Задаёт настройки Globalizer по умолчанию для всех функций модуля.

    Parameters
    ----------
    r : float
        Параметр надёжности индексного метода, r > 1 (по умолчанию 4.0).
        Больше r - надёжнее поиск глобального минимума, но больше испытаний.
    eps : float
        Точность остановки в долях диапазона переменных (по умолчанию 0.01).
    maxfev : int
        Лимит испытаний (перекрывает лимит, вычисленный из параметров scipy).
    local_search : bool
        Уточнять ли решение локальным методом Хука-Дживса.
    local_iterations : int
        Число итераций локального метода (по умолчанию 100).
    eq_tol : float
        Допуск для ограничений-равенств |h(x)| <= eq_tol (по умолчанию 1e-2).
        Равенства поддерживаются приближённо: индексный метод ищет точки в
        полосе |h(x)| <= eq_tol, очень узкую полосу он может не найти.
    threads : int
        Число потоков вычисления (перекрывает ``workers``).
    disp : bool
        Печатать итоги решателя.

    Вызов без аргументов сбрасывает настройки. Возвращает текущие настройки.
    """
    unknown = set(options) - set(_solver.GLOBALIZER_OPTIONS)
    if unknown:
        raise TypeError(f"Unknown Globalizer options: {sorted(unknown)}")
    if not options:
        _defaults.clear()
    _defaults.update(options)
    return dict(_defaults)


def get_options():
    """Возвращает настройки Globalizer по умолчанию."""
    return dict(_defaults)


def _merge(globalizer_options):
    opts = dict(_defaults)
    if globalizer_options:
        unknown = set(globalizer_options) - set(_solver.GLOBALIZER_OPTIONS)
        if unknown:
            raise TypeError(f"Unknown Globalizer options: {sorted(unknown)}")
        opts.update(globalizer_options)
    return opts


def _dimension(bounds):
    return normalize_bounds(bounds)[0].size


def _budget(limit):
    return min(int(limit), _solver.DEFAULT_MAXFEV)


def _wants_intermediate_result(callback):
    """Как в scipy: callback(intermediate_result) - новый стиль вызова."""
    try:
        params = list(inspect.signature(callback).parameters.values())
    except (TypeError, ValueError):
        return False
    return len(params) == 1 and params[0].name == "intermediate_result"


def _notify_intermediate(callback, legacy):
    """callback(intermediate_result) или callback в стиле метода (legacy)."""
    if callback is None:
        return None
    if _wants_intermediate_result(callback):
        return lambda x, f: callback(OptimizeResult(x=x, fun=f))
    return legacy


# ---------------------------------------------------------------------------
def differential_evolution(func, bounds, args=(), strategy="best1bin", maxiter=1000,
                           popsize=15, tol=0.01, mutation=(0.5, 1), recombination=0.7,
                           rng=None, callback=None, disp=False, polish=True,
                           init="latinhypercube", atol=0, updating="immediate", workers=1,
                           constraints=(), x0=None, *, integrality=None, vectorized=False,
                           seed=None, globalizer_options=None):
    """Глобальная минимизация - аналог ``scipy.optimize.differential_evolution``.

    Используются: func, bounds, args, maxiter и popsize (лимит испытаний
    ``(maxiter + 1) * popsize * N``), callback, disp, polish (локальное
    уточнение), workers, constraints, x0, integrality, vectorized.
    """
    n = _dimension(bounds)
    notify = _notify_intermediate(
        callback, None if callback is None else (lambda x, f: callback(x, convergence=0.0)))
    return _solver.solve(
        func, bounds, args=args, constraints=constraints, integrality=integrality, x0=x0,
        vectorized=vectorized, maxfev=_budget((maxiter + 1) * popsize * n),
        local_search=polish, workers=_solver.resolve_workers(workers), notify=notify,
        disp=disp, options=_merge(globalizer_options))


def dual_annealing(func, bounds, args=(), maxiter=1000, minimizer_kwargs=None,
                   initial_temp=5230.0, restart_temp_ratio=2e-05, visit=2.62, accept=-5.0,
                   maxfun=1e7, rng=None, no_local_search=False, callback=None, x0=None, *,
                   seed=None, globalizer_options=None):
    """Глобальная минимизация - аналог ``scipy.optimize.dual_annealing``.

    Используются: func, bounds, args, maxfun (лимит испытаний),
    no_local_search, callback(x, f, context), x0.
    """
    notify = None if callback is None else (lambda x, f: callback(x, f, 0))
    res = _solver.solve(
        func, bounds, args=args, x0=x0, maxfev=_budget(maxfun),
        local_search=not no_local_search, notify=notify, options=_merge(globalizer_options))
    res["njev"] = 0
    res["nhev"] = 0
    return res


def shgo(func, bounds, args=(), constraints=None, n=100, iters=1, callback=None,
         minimizer_kwargs=None, options=None, sampling_method="simplicial", *, workers=1,
         globalizer_options=None):
    """Глобальная минимизация - аналог ``scipy.optimize.shgo``.

    Используются: func, bounds, args, constraints, callback(xk), workers и
    из options - maxfev (лимит испытаний), disp.
    """
    options = dict(options or {})
    notify = None if callback is None else (lambda x, f: callback(x))
    res = _solver.solve(
        func, bounds, args=args, constraints=constraints,
        maxfev=_budget(options.get("maxfev", _solver.DEFAULT_MAXFEV)), local_search=True,
        workers=_solver.resolve_workers(workers), notify=notify,
        disp=bool(options.get("disp", False)), options=_merge(globalizer_options))
    res["xl"] = np.atleast_2d(res.x).copy()
    res["funl"] = np.array([res.fun])
    res["nlfev"] = 0
    res["nljev"] = 0
    res["nlhev"] = 0
    return res


def direct(func, bounds, *, args=(), eps=1e-4, maxfun=None, maxiter=1000,
           locally_biased=True, f_min=-np.inf, f_min_rtol=1e-4, vol_tol=1e-16, len_tol=1e-6,
           callback=None, globalizer_options=None):
    """Глобальная минимизация - аналог ``scipy.optimize.direct``.

    Используются: func, bounds, args, maxfun (по умолчанию 1000 * N),
    f_min и f_min_rtol (остановка при достижении известного минимума),
    callback(xk).
    """
    n = _dimension(bounds)
    limit = 1000 * n if maxfun is None else maxfun

    def notify(x, f):
        stop = False
        if callback is not None:
            stop = bool(callback(x))
        if np.isfinite(f_min):
            scale = abs(f_min) if f_min != 0 else 1.0
            if (f - f_min) / scale <= f_min_rtol:
                stop = True
        return stop

    use_notify = callback is not None or np.isfinite(f_min)
    res = _solver.solve(
        func, bounds, args=args, maxfev=_budget(limit), local_search=True,
        notify=notify if use_notify else None, options=_merge(globalizer_options))
    if res.status == 2 and callback is None:
        # остановка по f_min - штатное завершение, как в scipy
        res["status"] = 0
        res["success"] = True
        res["message"] = "The best function value found is within a relative error=" \
                         f"{f_min_rtol} of the (known) global optimum f_min"
    return res


def basinhopping(func, x0, niter=100, T=1.0, stepsize=0.5, minimizer_kwargs=None,
                 take_step=None, accept_test=None, callback=None, interval=50, disp=False,
                 niter_success=None, rng=None, *, target_accept_rate=0.5,
                 stepwise_factor=0.9, seed=None, globalizer_options=None):
    """Глобальная минимизация - аналог ``scipy.optimize.basinhopping``.

    Globalizer нужна область поиска: границы берутся из
    ``minimizer_kwargs['bounds']``; оттуда же - args и constraints.
    Используются также x0 и callback(x, f, accept).
    """
    mk = dict(minimizer_kwargs or {})
    bounds = mk.get("bounds")
    if bounds is None:
        raise ValueError("Globalizer requires finite bounds: pass them as "
                         "minimizer_kwargs={'bounds': ...}")
    notify = None if callback is None else (lambda x, f: callback(x, f, True))
    res = _solver.solve(
        func, bounds, args=mk.get("args", ()), constraints=mk.get("constraints", ()), x0=x0,
        maxfev=_solver.DEFAULT_MAXFEV, local_search=True, notify=notify, disp=disp,
        options=_merge(globalizer_options))
    res["lowest_optimization_result"] = OptimizeResult(res)
    res["minimization_failures"] = 0
    return res


def minimize(fun, x0, args=(), method=None, jac=None, hess=None, hessp=None, bounds=None,
             constraints=(), tol=None, callback=None, options=None, *, globalizer_options=None):
    """Минимизация - аналог ``scipy.optimize.minimize``, но ГЛОБАЛЬНАЯ.

    В отличие от scipy, требуются конечные ``bounds``: Globalizer ищет
    глобальный минимум во всей области, x0 - лишь дополнительный кандидат.
    method, jac, hess, hessp, tol принимаются для совместимости.
    Используются из options: maxfev / maxfun (лимит испытаний), disp.
    """
    if bounds is None:
        raise ValueError("globalizer.optimize.minimize requires finite bounds: "
                         "Globalizer searches for the global minimum in a box")
    options = dict(options or {})
    limit = options.get("maxfev", options.get("maxfun", _solver.DEFAULT_MAXFEV))
    notify = _notify_intermediate(
        callback, None if callback is None else (lambda x, f: callback(x)))
    return _solver.solve(
        fun, bounds, args=args, constraints=constraints, x0=x0, maxfev=_budget(limit),
        local_search=True, notify=notify, disp=bool(options.get("disp", False)),
        options=_merge(globalizer_options))
