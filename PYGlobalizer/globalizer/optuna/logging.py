"""Логирование в стиле optuna.logging.

По умолчанию, как в Optuna, о каждом завершённом испытании пишется строка
уровня INFO в stderr. Уменьшить вывод: ``optuna.logging.set_verbosity(optuna.logging.WARNING)``.
"""

import logging as _logging
import sys
import threading

CRITICAL = _logging.CRITICAL
FATAL = _logging.FATAL
ERROR = _logging.ERROR
WARNING = _logging.WARNING
WARN = _logging.WARN
INFO = _logging.INFO
DEBUG = _logging.DEBUG

__all__ = ["CRITICAL", "FATAL", "ERROR", "WARNING", "WARN", "INFO", "DEBUG",
           "get_logger", "get_verbosity", "set_verbosity", "disable_default_handler",
           "enable_default_handler", "disable_propagation", "enable_propagation"]

_ROOT = "globalizer.optuna"
_lock = threading.Lock()
_default_handler = None


class _Formatter(_logging.Formatter):
    def __init__(self):
        super().__init__("[%(levelname)1.1s %(asctime)s] %(message)s")


def _root_logger():
    return _logging.getLogger(_ROOT)


def _configure():
    global _default_handler
    with _lock:
        if _default_handler is not None:
            return
        _default_handler = _logging.StreamHandler(sys.stderr)
        _default_handler.setFormatter(_Formatter())
        root = _root_logger()
        root.addHandler(_default_handler)
        root.setLevel(INFO)
        root.propagate = False


def get_logger(name):
    """Логгер пакета (имя - внутри пространства globalizer.optuna)."""
    _configure()
    if not name.startswith(_ROOT):
        name = _ROOT + "." + name
    return _logging.getLogger(name)


def get_verbosity():
    _configure()
    return _root_logger().getEffectiveLevel()


def set_verbosity(verbosity):
    _configure()
    _root_logger().setLevel(verbosity)


def disable_default_handler():
    _configure()
    _root_logger().removeHandler(_default_handler)


def enable_default_handler():
    _configure()
    _root_logger().addHandler(_default_handler)


def disable_propagation():
    _configure()
    _root_logger().propagate = False


def enable_propagation():
    _configure()
    _root_logger().propagate = True
