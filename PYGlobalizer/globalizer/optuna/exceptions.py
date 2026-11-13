"""Исключения - те же имена, что в optuna.exceptions."""

__all__ = ["OptunaError", "TrialPruned", "DuplicatedStudyError", "ExperimentalWarning"]


class OptunaError(Exception):
    """Базовое исключение."""


class TrialPruned(OptunaError):
    """Поднимается в целевой функции, чтобы прервать (обрезать) испытание."""


class DuplicatedStudyError(OptunaError):
    """Исследование с таким именем уже существует."""


class ExperimentalWarning(Warning):
    """Предупреждение об экспериментальной функции."""
