"""Загрузка C++-модуля PYDGlobalizer и вспомогательного PYProblem.

Порядок поиска модуля PYDGlobalizer*.pyd / *.so:
  1) уже доступен через sys.path / PYTHONPATH;
  2) каталог из переменной окружения GLOBALIZER_BIN;
  3) каталог _bin репозитория Globalizer (рядом с папкой PYGlobalizer).
PYProblem.py берётся из папки PYGlobalizer (родительской для пакета).
"""

import importlib
import os
import sys
from pathlib import Path

_HELPERS_DIR = Path(__file__).resolve().parent.parent.parent      # .../PYGlobalizer
_DEFAULT_BIN_DIR = _HELPERS_DIR.parent / "_bin"                    # .../_bin

_native = None
_pyproblem_cls = None


def _candidate_dirs():
    dirs = []
    env = os.environ.get("GLOBALIZER_BIN")
    if env:
        dirs.append(Path(env))
    dirs.append(_DEFAULT_BIN_DIR)
    return dirs


def load():
    """Возвращает (модуль PYDGlobalizer, класс PYProblem)."""
    global _native, _pyproblem_cls
    if _native is not None:
        return _native, _pyproblem_cls

    # PYProblem импортируется C++-модулем по имени "PYProblem".
    if str(_HELPERS_DIR) not in sys.path:
        sys.path.append(str(_HELPERS_DIR))

    try:
        native = importlib.import_module("PYDGlobalizer")
    except ImportError:
        native = None
        for d in _candidate_dirs():
            if d.is_dir() and str(d) not in sys.path:
                sys.path.insert(0, str(d))
                try:
                    native = importlib.import_module("PYDGlobalizer")
                    break
                except ImportError:
                    sys.path.remove(str(d))
        if native is None:
            raise ImportError(
                "Cannot import the Globalizer extension module PYDGlobalizer. "
                "Build it (gen/StartGCC-no_mp-No_MPI_PYGlobalizer.bat or "
                "gen/StartVS-no_mp-No_MPI_PYGlobalizer.bat) and put its folder "
                "into PYTHONPATH or the GLOBALIZER_BIN environment variable.") from None

    pyproblem = importlib.import_module("PYProblem")
    _native, _pyproblem_cls = native, pyproblem.PYProblem
    return _native, _pyproblem_cls
