#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Globalizer — единый раннер интеграционных тестов для ВСЕХ примеров из examples/.
================================================================================
Запускает как C++ примеры (собранные исполняемые файлы Globalizer<Имя>),
так и Python примеры (examples/*.py через интерфейс PYDGlobalizer).

Критерий успеха (по условию задачи):
  * если для примера известен глобальный минимум (known_optimum != null в
    references/examples.json) — найденное значение должно попасть в допуск:
        |f_found - known_optimum| <= tolerance;
  * если минимум неизвестен (known_optimum == null) — достаточно, что пример
    отработал без краха (код выхода 0, без исключения/segfault).

Статусы:
  PASS  — критерий выполнен;
  FAIL  — краш, ненулевой код выхода, или значение вне допуска;
  SKIP  — пример пропущен осознанно (нет собранного exe / нет библиотеки
          задачи в _bin / нет внешнего python-пакета у optional-примера).

Коды выхода процесса раннера:
  0 — нет ни одного FAIL (все PASS/SKIP);
  1 — есть хотя бы один FAIL;
  2 — ошибка конфигурации (не найден references/examples.json и т. п.).
"""

import argparse
import json
import os
import re
import subprocess
import sys
import time
import importlib.util
from pathlib import Path

# --------------------------------------------------------------------------- #
# ВАЖНО: под CTest вывод раннера перенаправляется в pipe, и Python на Windows
# выбирает системную кодировку (например cp1251), в которой нет многих символов.
# Принудительно переводим stdout/stderr в UTF-8 с заменой неотображаемых
# символов — иначе UnicodeEncodeError роняет сам раннер (а не примеры).
# --------------------------------------------------------------------------- #
for _stream in (sys.stdout, sys.stderr):
    try:
        _stream.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

# --------------------------------------------------------------------------- #
# Пути. Скрипт лежит в integration_tests/, корень репозитория — на уровень выше.
# --------------------------------------------------------------------------- #
SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
EXAMPLES_DIR = ROOT_DIR / "examples"
HELPERS_DIR = ROOT_DIR / "PYGlobalizer"
BENCHMARKS_DIR = ROOT_DIR / "third_party" / "Problems" / "Problems"
REFERENCES = SCRIPT_DIR / "references" / "examples.json"

IS_WINDOWS = os.name == "nt"
EXE_SUFFIX = ".exe" if IS_WINDOWS else ""
LIB_PREFIX = "" if IS_WINDOWS else "lib"
LIB_EXT = ".dll" if IS_WINDOWS else ".so"

DEFAULT_TIMEOUT = 900  # сек на один пример, если в эталоне не задано иное

# Числа с плавающей точкой в выводе: 1.23, -4e-5, 1.5E+3 и т. п.
_FLOAT_RE = r"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"


# --------------------------------------------------------------------------- #
# Мелкие помощники
# --------------------------------------------------------------------------- #
def color(txt, c):
    if IS_WINDOWS or not sys.stdout.isatty():
        return txt
    codes = {"green": 32, "red": 31, "yellow": 33, "cyan": 36, "grey": 90}
    return f"\033[{codes.get(c, 0)}m{txt}\033[0m"


def find_bin_dir(explicit):
    if explicit:
        p = Path(explicit).resolve()
        if p.exists():
            return p
    for cand in (ROOT_DIR / "_bin",
                 ROOT_DIR / "build_64" / "_bin",
                 ROOT_DIR / "build" / "_bin"):
        if cand.exists():
            return cand
    return ROOT_DIR / "_bin"


def resolve_exe(bin_dir, target):
    for name in (f"{target}{EXE_SUFFIX}", f"{target}_d{EXE_SUFFIX}"):
        p = bin_dir / name
        if p.exists():
            return p
    return None


def lib_present(bin_dir, libname):
    for name in (f"{LIB_PREFIX}{libname}{LIB_EXT}",
                 f"{libname}{LIB_EXT}",
                 f"{LIB_PREFIX}{libname}_d{LIB_EXT}"):
        if (bin_dir / name).exists():
            return True
    return False


def py_module_available(modname):
    try:
        return importlib.util.find_spec(modname) is not None
    except (ImportError, ValueError, ModuleNotFoundError):
        return False


def extract_best_value(text):
    """
    Достаёт найденное значение целевой функции из текста вывода примера.
    Берёт ПОСЛЕДНЕЕ совпадение (итоговый результат печатается в конце).
    """
    patterns = [
        r"best_value['\"]?\s*[:=]\s*(" + _FLOAT_RE + r")",   # python dict / print
        r"Minimum\s+value\s*=\s*(" + _FLOAT_RE + r")",        # C++: "Minimum value = ..."
        r"f\*\s*=\s*(" + _FLOAT_RE + r")",                    # "f* = ..."
        r"\bmin\s*=\s*(" + _FLOAT_RE + r")",                  # PrintFinal: "min = ..."
        r"BestValue\s*[:=]\s*(" + _FLOAT_RE + r")",
        r"Best\s+value[^-\d]*(" + _FLOAT_RE + r")",
        r"optimum\s+value\s*=\s*(" + _FLOAT_RE + r")",
        r"Optimum\s*[:=]\s*(" + _FLOAT_RE + r")",
    ]
    best = None
    for pat in patterns:
        matches = re.findall(pat, text, flags=re.IGNORECASE)
        if matches:
            try:
                best = float(matches[-1])
            except ValueError:
                continue
            break
    return best


# --------------------------------------------------------------------------- #
# Результат одного теста
# --------------------------------------------------------------------------- #
class Result:
    def __init__(self, name, kind):
        self.name = name
        self.kind = kind          # "cpp" | "python"
        self.status = "FAIL"      # PASS | FAIL | SKIP
        self.message = ""
        self.value = None
        self.expected = None
        self.seconds = 0.0

    def row(self):
        c = {"PASS": "green", "FAIL": "red", "SKIP": "yellow"}.get(self.status, "grey")
        val = "" if self.value is None else f"f={self.value:.6g}"
        exp = "" if self.expected is None else f" (ref {self.expected:g})"
        # Используем только ASCII в строке результата — вывод не зависит от
        # кодировки консоли/pipe (под CTest это критично).
        return (f"  [{color(self.status, c):>4}] {self.kind:6} {self.name:34} "
                f"{val}{exp}  {self.seconds:5.1f}s  {self.message}")


# --------------------------------------------------------------------------- #
# Запуск процесса
# --------------------------------------------------------------------------- #
def run_process(cmd, cwd, timeout, extra_env=None):
    env = os.environ.copy()
    if extra_env:
        env.update(extra_env)
    env["PYTHONIOENCODING"] = "utf-8"
    t0 = time.time()
    try:
        proc = subprocess.run(
            cmd, cwd=str(cwd), stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            timeout=timeout, env=env,
        )
        out = proc.stdout.decode("utf-8", errors="replace")
        return proc.returncode, out, time.time() - t0, False
    except subprocess.TimeoutExpired as e:
        raw = e.stdout or b""
        out = raw.decode("utf-8", errors="replace") if isinstance(raw, bytes) else str(raw)
        return None, out, time.time() - t0, True


def judge(res, spec, returncode, out, timed_out):
    if timed_out:
        res.status = "FAIL"
        res.message = "timeout"
        return
    if returncode is None or returncode != 0:
        res.status = "FAIL"
        res.message = f"exit code {returncode}"
        return

    known = spec.get("known_optimum", None)
    if known is None:
        res.status = "PASS"
        res.message = "finished without crash (no reference)"
        res.value = extract_best_value(out)
        return

    tol = float(spec.get("tolerance", 0.1))
    res.expected = float(known)
    val = extract_best_value(out)
    res.value = val
    if val is None:
        res.status = "FAIL"
        res.message = "could not extract found value from output"
        return
    if abs(val - known) <= tol:
        res.status = "PASS"
        res.message = f"|d|={abs(val-known):.2e} <= {tol:g}"
    else:
        res.status = "FAIL"
        res.message = f"|d|={abs(val-known):.2e} > {tol:g}"


# --------------------------------------------------------------------------- #
# C++ примеры
# --------------------------------------------------------------------------- #
def run_cpp(spec, bin_dir):
    res = Result(spec["name"], "cpp")

    exe = resolve_exe(bin_dir, spec["target"])
    if exe is None:
        res.status = "SKIP"
        if spec.get("needs_gcgen"):
            res.message = "exe not built (GCGen off?)"
        else:
            res.message = f"executable {spec['target']} not found in _bin"
        return res

    for lib in spec.get("requires_libs", []):
        if not lib_present(bin_dir, lib):
            res.status = "SKIP"
            res.message = f"problem library '{lib}' not found in _bin"
            return res

    timeout = int(spec.get("timeout", DEFAULT_TIMEOUT))
    cmd = [str(exe)] + [str(a) for a in spec.get("args", [])]
    env = {"LD_LIBRARY_PATH": f"{bin_dir}{os.pathsep}" + os.environ.get("LD_LIBRARY_PATH", "")}
    rc, out, secs, to = run_process(cmd, bin_dir, timeout, env)
    res.seconds = secs
    judge(res, spec, rc, out, to)
    return res


# --------------------------------------------------------------------------- #
# Python примеры
# --------------------------------------------------------------------------- #
def run_python(spec, bin_dir):
    res = Result(spec["name"], "python")
    script = EXAMPLES_DIR / spec["script"]
    if not script.exists():
        res.status = "SKIP"
        res.message = f"script {spec['script']} not found"
        return res

    if not any(bin_dir.glob("PYDGlobalizer*.pyd")) and not any(bin_dir.glob("PYDGlobalizer*.so")):
        res.status = "SKIP"
        res.message = "PYDGlobalizer module not built (need -DGLOBALIZER_PYBIND_VERSION=ON)"
        return res

    for mod in spec.get("requires_py", []):
        if not py_module_available(mod):
            res.status = "SKIP"
            res.message = f"external package '{mod}' not found"
            return res

    timeout = int(spec.get("timeout", DEFAULT_TIMEOUT))
    env = {
        "PYTHONPATH": os.pathsep.join(
            [str(bin_dir), str(HELPERS_DIR), str(BENCHMARKS_DIR),
             str(EXAMPLES_DIR), os.environ.get("PYTHONPATH", "")]
        ),
        "LD_LIBRARY_PATH": f"{bin_dir}{os.pathsep}" + os.environ.get("LD_LIBRARY_PATH", ""),
    }
    cmd = [sys.executable, str(script)]
    rc, out, secs, to = run_process(cmd, bin_dir, timeout, env)
    res.seconds = secs

    # Пример мог сам вывести [SKIP] (нет внешнего пакета) — уважаем это.
    if not to and (rc == 0) and re.search(r"\[SKIP\]", out):
        res.status = "SKIP"
        res.message = "example reported SKIP itself"
        return res

    judge(res, spec, rc, out, to)
    return res


# --------------------------------------------------------------------------- #
# main
# --------------------------------------------------------------------------- #
def main():
    ap = argparse.ArgumentParser(description="Run all Globalizer examples as tests.")
    ap.add_argument("--bin-dir", default=None, help="_bin directory with exe and libs.")
    ap.add_argument("--only", choices=["cpp", "python"], default=None,
                    help="run only C++ or only Python examples.")
    ap.add_argument("--filter", default=None,
                    help="comma-separated name substrings to select.")
    ap.add_argument("--list", action="store_true", help="print the list of tests and exit.")
    args = ap.parse_args()

    if not REFERENCES.exists():
        print(f"[FATAL] not found {REFERENCES}", file=sys.stderr)
        return 2
    specs = json.loads(REFERENCES.read_text(encoding="utf-8"))
    bin_dir = find_bin_dir(args.bin_dir)

    def wanted(name):
        if not args.filter:
            return True
        return any(s.strip().lower() in name.lower() for s in args.filter.split(","))

    cpp_specs = [s for s in specs.get("cpp", []) if wanted(s["name"])]
    py_specs = [s for s in specs.get("python", []) if wanted(s["name"])]
    if args.only == "cpp":
        py_specs = []
    elif args.only == "python":
        cpp_specs = []

    if args.list:
        print("C++ examples:")
        for s in cpp_specs:
            print(f"  {s['name']:34} optimum={s.get('known_optimum')}")
        print("Python examples:")
        for s in py_specs:
            print(f"  {s['name']:34} optimum={s.get('known_optimum')}")
        return 0

    print("=" * 78)
    print("   GLOBALIZER - RUN ALL EXAMPLES AS INTEGRATION TESTS")
    print(f"   _bin: {bin_dir}")
    print("=" * 78)

    results = []
    if cpp_specs:
        print("\n--- C++ examples ---")
        for s in cpp_specs:
            r = run_cpp(s, bin_dir)
            results.append(r)
            print(r.row())
    if py_specs:
        print("\n--- Python examples ---")
        for s in py_specs:
            r = run_python(s, bin_dir)
            results.append(r)
            print(r.row())

    n_pass = sum(1 for r in results if r.status == "PASS")
    n_fail = sum(1 for r in results if r.status == "FAIL")
    n_skip = sum(1 for r in results if r.status == "SKIP")

    print("\n" + "=" * 78)
    print(f"Total: {n_pass} PASS, {n_fail} FAIL, {n_skip} SKIP of {len(results)} tests.")
    print("=" * 78)
    return 1 if n_fail else 0


if __name__ == "__main__":
    sys.exit(main())
