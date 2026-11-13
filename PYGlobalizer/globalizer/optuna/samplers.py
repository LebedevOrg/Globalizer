"""Сэмплеры: точки испытаний выбирает индексный метод Globalizer.

GlobalizerSampler - единственный «настоящий» сэмплер пакета. Имена сэмплеров
Optuna (TPESampler, RandomSampler, CmaEsSampler, GPSampler, QMCSampler, ...)
оставлены для совместимости: их параметры принимаются, но точки всё равно
выбирает Globalizer; используются только ``seed`` (для запасной случайной
выборки) и ``constraints_func`` (ограничения).
"""

import warnings

__all__ = ["BaseSampler", "GlobalizerSampler", "TPESampler", "RandomSampler",
           "CmaEsSampler", "GPSampler", "QMCSampler", "BruteForceSampler"]

#: Лимит итераций одного запуска метода. Число испытаний ограничивает
#: study.optimize(n_trials=...), а этот лимит лишь страхует от бесконечного поиска.
DEFAULT_MAX_ITERATIONS = 1_000_000


class BaseSampler:
    """Базовый класс сэмплеров пакета."""


class GlobalizerSampler(BaseSampler):
    """Сэмплер на основе индексного метода глобальной оптимизации Globalizer.

    Parameters
    ----------
    r : float
        Параметр надёжности метода (> 1): больше - надёжнее, но больше испытаний.
    eps : float
        Точность первого запуска метода в долях диапазона параметров. Когда
        метод достигает точности, а испытания ещё нужны, он перезапускается
        с точностью ``eps * eps_decay`` (уже вычисленные точки не повторяются).
    eps_decay, eps_min : float
        Уменьшение точности при перезапуске и её нижняя граница; дальше
        пространство поиска считается исчерпанным и ``optimize`` завершается.
    local_search : bool
        Локальное уточнение (Хук-Дживс) после достижения точности.
    local_iterations : int
        Итераций локального метода.
    max_discrete_combinations : int
        Категориальные параметры перебираются Globalizer'ом как дискретные
        переменные, пока число их сочетаний не больше этого значения;
        остальные (и все целые/с шагом) кодируются непрерывной переменной с
        округлением.
    seed : int or None
        Зерно запасной случайной выборки (используется, только если метод
        не может выдать точку: параллельные ask() без tell(), исчерпанное
        пространство).
    constraints_func : callable or None
        Как в Optuna: ``constraints_func(frozen_trial) -> Sequence[float]``,
        испытание допустимо, если все значения <= 0. Вместо него можно
        вызывать ``trial.set_constraint(name, value)`` в целевой функции.
    """

    def __init__(self, *, r=4.0, eps=0.1, eps_decay=0.5, eps_min=1e-6,
                 local_search=True, local_refine_mode="updated", local_iterations=300,
                 max_discrete_combinations=64, max_iterations=DEFAULT_MAX_ITERATIONS, seed=None,
                 constraints_func=None):
        if local_refine_mode not in ("final", "updated"):
            raise ValueError(f"local_refine_mode must be 'final' or 'updated', got {local_refine_mode!r}")
        if not r > 1.0:
            raise ValueError(f"r must be > 1, got {r}")
        if not 0.0 < eps < 1.0:
            raise ValueError(f"eps must be in (0, 1), got {eps}")
        if not 0.0 < eps_decay < 1.0:
            raise ValueError(f"eps_decay must be in (0, 1), got {eps_decay}")
        self.r = float(r)
        self.eps = float(eps)
        self.eps_decay = float(eps_decay)
        self.eps_min = float(eps_min)
        self.local_search = bool(local_search)
        self.local_refine_mode = local_refine_mode
        self.local_iterations = int(local_iterations)
        self.max_discrete_combinations = int(max_discrete_combinations)
        self.max_iterations = int(max_iterations)
        self.seed = seed
        self.constraints_func = constraints_func

    def __repr__(self):
        return (f"GlobalizerSampler(r={self.r}, eps={self.eps}, local_search={self.local_search}, "
                f"seed={self.seed})")


def _compat(name):
    def factory(*args, seed=None, constraints_func=None, **kwargs):
        """Совместимое имя сэмплера Optuna: возвращает GlobalizerSampler."""
        return GlobalizerSampler(seed=seed, constraints_func=constraints_func)
    factory.__name__ = factory.__qualname__ = name
    factory.__doc__ = (f"{name} из Optuna: параметры принимаются, но точки выбирает "
                       "Globalizer (возвращается GlobalizerSampler; используются seed и "
                       "constraints_func).")
    return factory


TPESampler = _compat("TPESampler")
RandomSampler = _compat("RandomSampler")
CmaEsSampler = _compat("CmaEsSampler")
GPSampler = _compat("GPSampler")
QMCSampler = _compat("QMCSampler")
BruteForceSampler = _compat("BruteForceSampler")


def as_globalizer_sampler(sampler):
    """Любой сэмплер (в т.ч. объект самой Optuna) -> GlobalizerSampler."""
    if sampler is None:
        return GlobalizerSampler()
    if isinstance(sampler, GlobalizerSampler):
        return sampler
    # Сэмплер настоящей Optuna: берём seed и constraints_func, если они есть.
    seed = getattr(sampler, "_seed", None)
    constraints_func = getattr(sampler, "_constraints_func", None)
    warnings.warn(f"{type(sampler).__name__} is replaced by GlobalizerSampler: trial points "
                  "are chosen by the Globalizer index method.", UserWarning, stacklevel=3)
    return GlobalizerSampler(seed=seed if isinstance(seed, int) else None,
                             constraints_func=constraints_func)
