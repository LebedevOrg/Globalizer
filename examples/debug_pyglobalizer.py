"""Драйвер для отладки C++-кода PYDGlobalizer из Visual Studio.

PYDGlobalizer - скомпилированное C++-расширение (.pyd).
PYGlobalizer/ - каталог Python-вспомогательных модулей (PYProblem.py, trial.py).
Имена различаются, поэтому порядок путей больше не создаёт коллизию импорта.
"""

import argparse
import glob
import os
import sys
import traceback
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT = SCRIPT_DIR.parent
MODULE_NAME = "PYDGlobalizer"

BIN_DIRS = [
    ROOT / "_bin",
    ROOT / "build_64_py" / "Debug",
    ROOT / "build_64_py" / "bin" / "Debug",
    ROOT / "build_64" / "Debug",
    ROOT / "x64" / "Debug",
]
HELPERS_DIR = ROOT / "PYGlobalizer"
_DLL_DIR_HANDLES = []


def find_module_files():
    matches = []
    for directory in BIN_DIRS:
        if not directory.exists():
            continue
        files = glob.glob(str(directory / f"{MODULE_NAME}*.pyd"))
        files += glob.glob(str(directory / f"{MODULE_NAME}*.so"))
        matches.extend(Path(item) for item in files)
    return matches


def configure_paths(module_files):
    # Binary module must be visible before importing it.
    for directory in reversed([path.parent for path in module_files]):
        path = str(directory)
        if path not in sys.path:
            sys.path.insert(0, path)

    # This exposes PYProblem.py as `from PYProblem import PYProblem`.
    if HELPERS_DIR.exists() and str(HELPERS_DIR) not in sys.path:
        sys.path.append(str(HELPERS_DIR))

    # Python >= 3.8 needs explicit DLL search directories on Windows.
    if hasattr(os, "add_dll_directory"):
        for directory in BIN_DIRS:
            if directory.exists():
                try:
                    _DLL_DIR_HANDLES.append(os.add_dll_directory(str(directory)))
                except OSError:
                    pass


def banner(module_files):
    print("=" * 70)
    print("PYDGlobalizer debug driver")
    print(f"  PID              : {os.getpid()}")
    print(f"  python.exe       : {sys.executable}")
    print(f"  Python           : {sys.version.split()[0]}")
    print(f"  architecture     : {'x64' if sys.maxsize > 2**32 else 'x86'}")
    print(f"  repository       : {ROOT}")
    if module_files:
        print("  extension files:")
        for module_file in module_files:
            print(f"    {module_file}")
    else:
        print(f"  [!] {MODULE_NAME}*.pyd was not found in the configured build directories")
    print("=" * 70)


def wait_for_debugger():
    print(f"[WAIT] Attach Visual Studio to python.exe (PID {os.getpid()}) as Native code,")
    print("       set breakpoints, then press Enter.")
    input()


def import_modules():
    try:
        from PYProblem import PYProblem
    except Exception:
        print("[FATAL] Failed to import PYProblem from the helper directory.")
        traceback.print_exc()
        raise SystemExit(1)

    try:
        module = __import__(MODULE_NAME)
    except Exception:
        print(f"[FATAL] Failed to import native extension {MODULE_NAME}.")
        traceback.print_exc()
        raise SystemExit(2)

    loaded_file = Path(getattr(module, "__file__", ""))
    expected_suffixes = {".pyd", ".so"}
    if loaded_file.suffix.lower() not in expected_suffixes:
        print(f"[FATAL] {MODULE_NAME} was not loaded from a native extension: {loaded_file}")
        raise SystemExit(3)

    missing_api = [name for name in ("solve", "solve_legacy", "validate_problem")
                   if not hasattr(module, name)]
    if missing_api:
        print(f"[FATAL] {MODULE_NAME} is missing API: {', '.join(missing_api)}")
        raise SystemExit(4)

    print(f"  module loaded from: {loaded_file}")
    print(f"  module version    : {getattr(module, '__version__', '<not set>')}")
    return PYProblem, module


def objective_with_counter():
    state = {"count": 0}

    def sphere(x):
        state["count"] += 1
        value = x[0] * x[0] + x[1] * x[1]
        if state["count"] <= 5:
            print(f"  Python objective #{state['count']}: x={x}, f={value:.6f}")
        return value

    return sphere, state


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--wait", action="store_true", help="wait before the import")
    parser.add_argument("--iters", type=int, default=200)
    args = parser.parse_args()

    module_files = find_module_files()
    configure_paths(module_files)
    banner(module_files)
    if args.wait:
        wait_for_debugger()

    PYProblem, pyd_globalizer = import_modules()
    if hasattr(pyd_globalizer, "set_log_level"):
        pyd_globalizer.set_log_level(3)

    objective, state = objective_with_counter()
    problem = PYProblem(dimension=2, numCriterions=1)
    problem.set_bounds([-5.0, -5.0], [5.0, 5.0])
    problem.add_function(objective, name="sphere2d")
    if hasattr(problem, "set_optimum"):
        problem.set_optimum(0.0, [0.0, 0.0])

    print("\n[1/3] validate_problem()")
    print("      ", pyd_globalizer.validate_problem(problem))

    print(f"\n[2/3] solve(max_iterations={args.iters})")
    params = pyd_globalizer.SolverParameters()
    params.max_iterations = args.iters
    params.r = 4.0
    params.num_threads = 1
    params.epsilon = 0.01
    if hasattr(params, "verbose"):
        params.verbose = 3
    result = pyd_globalizer.solve(problem, params)
    print("      ", result)
    print(f"      objective calls: {state['count']}")

    print("\n[3/3] solve_legacy()")
    print("      ", pyd_globalizer.solve_legacy(problem, args.iters, 4.0, False, 1))
    print("\n[OK] Debug run completed.")


if __name__ == "__main__":
    main()
