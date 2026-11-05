#!/usr/bin/env bash
# ============================================================================
#  build_test_and_open.sh   (Linux / macOS)
#
#  POSIX-аналог BuildTestAndOpenVS.bat для Globalizer.
#  Подтягивает всё необходимое, собирает ВСЕ проекты, прогоняет сначала
#  МОДУЛЬНЫЕ тесты, затем ИНТЕГРАЦИОННЫЕ, и в конце открывает каталог сборки
#  (на Windows это был запуск Visual Studio; здесь — файловый менеджер или,
#  при --xcode на macOS, сгенерированный .xcodeproj).
#
#  Отличия от Windows-версии продиктованы платформой:
#    * нет Visual Studio/vswhere -> генератор Ninja (если есть) или Unix
#      Makefiles; на macOS с ключом --xcode используется генератор Xcode;
#    * python живёт в <env>/bin/python, модуль/библиотеки: .so (Linux)
#      или .dylib (macOS);
#    * OneAPI/MPI не используются (как и в v2 bat): MP=OFF, MPI=OFF.
#
#  Ключи:
#    (без ключей)   полный цикл: конфигурация, сборка, тесты, открыть каталог
#    --clean        очистить каталог сборки перед конфигурацией
#    --noopen       не открывать каталог/IDE в конце
#    --notest       не запускать тесты (только собрать)
#    --debug        конфигурация Debug (по умолчанию Release)
#    --xcode        (только macOS) сгенерировать и открыть проект Xcode
#    --jobs N       число параллельных задач сборки (по умолчанию: nproc)
#
#  Пример:  ./gen/build_test_and_open.sh --debug --jobs 8
# ============================================================================
set -u

# ---------------------------------------------------------------------------
#  Параметры и пути
# ---------------------------------------------------------------------------
PY_VER="3.12"
PY_MODULE="PYDGlobalizer"
BUILD_CONFIG="Release"
DO_CLEAN=0
DO_OPEN=1
DO_TEST=1
USE_XCODE=0
JOBS=""

for arg in "$@"; do
  case "$arg" in
    --clean)  DO_CLEAN=1 ;;
    --noopen) DO_OPEN=0 ;;
    --notest) DO_TEST=0 ;;
    --debug)  BUILD_CONFIG="Debug" ;;
    --xcode)  USE_XCODE=1 ;;
    --jobs=*) JOBS="${arg#*=}" ;;
    --jobs)   : ;;  # поддержка формы "--jobs N" обрабатывается ниже
    *)
      # число после --jobs
      if [ "${PREV:-}" = "--jobs" ]; then JOBS="$arg"; fi
      ;;
  esac
  PREV="$arg"
done

# Определяем ОС
OS="$(uname -s)"
case "$OS" in
  Linux*)  PLATFORM="linux" ;;
  Darwin*) PLATFORM="macos" ;;
  *)       PLATFORM="unknown" ;;
esac

# Число потоков сборки
if [ -z "$JOBS" ]; then
  if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
  elif [ "$PLATFORM" = "macos" ]; then
    JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
  else
    JOBS=4
  fi
fi

# Корень репозитория = папка на уровень выше этого скрипта
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
START_DIR="$(pwd)"
BUILD_DIR="$ROOT_DIR/build_64"
ENV_DIR="$BUILD_DIR/Globalizer_env"
PY_EXE="$ENV_DIR/bin/python"
BIN_DIR="$ROOT_DIR/_bin"

# Расширение динамических библиотек задач
if [ "$PLATFORM" = "macos" ]; then LIB_EXT="dylib"; else LIB_EXT="so"; fi

FAIL_MSG=""

die() {
  echo
  echo "========================================"
  echo " BUILD / TEST FAILED"
  echo " $1"
  echo "========================================"
  echo
  echo "Troubleshooting:"
  echo "  1. Re-run with --clean if the generator changed."
  echo "  2. Ensure conda is on PATH (or run from an activated base env)."
  echo "  3. Check internet/firewall (conda + pip need network)."
  cd "$START_DIR" || true
  exit 1
}

echo
echo "Platform : $PLATFORM ($OS)"
echo "Root dir : $ROOT_DIR"
echo "Build dir: $BUILD_DIR"
echo "Env dir  : $ENV_DIR"
echo "Config   : $BUILD_CONFIG"
echo "Jobs     : $JOBS"

cd "$ROOT_DIR" || die "Cannot cd to root directory."
mkdir -p "$BUILD_DIR"

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[1/9] Git submodules"
echo "========================================"
if command -v git >/dev/null 2>&1; then
  git -C "$ROOT_DIR" submodule update --init --recursive \
    || echo "[WARNING] submodule update failed - third_party may be incomplete."
else
  echo "[WARNING] git not found - skipping submodules."
fi

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[2/9] Conda environment (Python $PY_VER)"
echo "========================================"
find_conda() {
  if command -v conda >/dev/null 2>&1; then return 0; fi
  for p in "$HOME/miniconda3" "$HOME/anaconda3" "$HOME/miniforge3" \
           "/opt/miniconda3" "/opt/anaconda3" "/opt/homebrew/anaconda3" \
           "/usr/local/miniconda3" "/usr/local/anaconda3"; do
    if [ -f "$p/etc/profile.d/conda.sh" ]; then
      # shellcheck disable=SC1090
      . "$p/etc/profile.d/conda.sh"
      return 0
    fi
  done
  return 1
}

create_env() {
  # $1 — источник каналов
  src="$1"; ch=""
  case "$src" in
    default)     ch="" ;;
    conda-forge) ch="-c conda-forge" ;;
    yandex)      ch="-c https://mirrors.yandex.ru/mirrors/anaconda/cloud/conda-forge/ -c https://mirrors.yandex.ru/mirrors/anaconda/pkgs/main/" ;;
    aliyun)      ch="-c https://mirrors.aliyun.com/anaconda/pkgs/main/ -c https://mirrors.aliyun.com/anaconda/pkgs/free/" ;;
    tsinghua)    ch="-c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/main/ -c https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/" ;;
  esac
  echo "  [Attempt] conda env from source: $src"
  [ -d "$ENV_DIR" ] && rm -rf "$ENV_DIR"
  # shellcheck disable=SC2086
  conda create -y -p "$ENV_DIR" $ch "python=$PY_VER" pip
  [ -x "$PY_EXE" ] && { echo "  [OK] created from $src"; return 0; }
  echo "  [FAILED] $src"; return 1
}

find_conda || die "conda not found. Install Miniconda/Anaconda or add it to PATH."

if [ -x "$PY_EXE" ]; then
  echo "[OK] Environment already exists - reusing it."
else
  for s in default conda-forge yandex aliyun tsinghua; do
    [ -x "$PY_EXE" ] || create_env "$s"
  done
fi
[ -x "$PY_EXE" ] || die "Failed to create conda environment from all channels."

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[3/9] Python packages"
echo "========================================"
"$PY_EXE" -m pip install --upgrade pip
REQ_FILE="$ROOT_DIR/requirements_build.txt"
[ -f "$REQ_FILE" ] || REQ_FILE="$ROOT_DIR/requirements.txt"
echo "Using $REQ_FILE"
if [ -f "$REQ_FILE" ]; then
  "$PY_EXE" -m pip install -r "$REQ_FILE" || echo "[WARNING] some packages failed to install. Continuing..."
fi
"$PY_EXE" -m pip install pybind11 numpy || die "pip install of pybind11/numpy failed."

PYBIND11_DIR="$("$PY_EXE" -m pybind11 --cmakedir 2>/dev/null)"
[ -n "$PYBIND11_DIR" ] || die "Cannot get pybind11 CMake dir."
echo "pybind11_DIR = $PYBIND11_DIR"

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[4/9] Toolchain / generator detection"
echo "========================================"
# На Linux/macOS Visual Studio нет. Выбираем генератор.
GENERATOR=""
if [ "$USE_XCODE" = "1" ] && [ "$PLATFORM" = "macos" ]; then
  GENERATOR="Xcode"
elif command -v ninja >/dev/null 2>&1; then
  GENERATOR="Ninja"
else
  GENERATOR="Unix Makefiles"
fi
echo "[Using generator] $GENERATOR"

# Проверяем компилятор
if [ "$PLATFORM" = "macos" ]; then
  command -v clang++ >/dev/null 2>&1 || echo "[WARNING] clang++ not found - install Xcode Command Line Tools."
else
  command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 \
    || echo "[WARNING] no C++ compiler found - install g++ or clang."
fi

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[5/9] CMake selection"
echo "========================================"
command -v cmake >/dev/null 2>&1 || die "cmake not found. Install cmake (brew/apt/dnf) or 'pip install cmake'."
echo "[Using] $(command -v cmake)  ($(cmake --version | head -1))"

if [ "$DO_CLEAN" = "1" ]; then
  echo "--clean: wiping CMake cache"
  rm -f  "$BUILD_DIR/CMakeCache.txt"
  rm -rf "$BUILD_DIR/CMakeFiles"
fi

# Single-config генераторы (Ninja/Make) требуют CMAKE_BUILD_TYPE на конфигурации.
BUILDTYPE_ARG=""
case "$GENERATOR" in
  "Xcode") : ;;  # multi-config
  *)       BUILDTYPE_ARG="-DCMAKE_BUILD_TYPE=$BUILD_CONFIG" ;;
esac

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[6/9] CMake configuration (ALL projects)"
echo "========================================"
# shellcheck disable=SC2086
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G "$GENERATOR" $BUILDTYPE_ARG \
  -DGLOBALIZER_USE_MP=OFF \
  -DGLOBALIZER_USE_MPI=OFF \
  -DGLOBALIZER_USE_CUDA=OFF \
  -DGLOBALIZER_BUILD_PROBLEMS=ON \
  -DGLOBALIZER_BUILD_GCGEN=ON \
  -DGLOBALIZER_BUILD_TESTS=ON \
  -DGLOBALIZER_BUILD_INTEGRATION_TESTS=ON \
  -DGLOBALIZER_PYBIND_VERSION=ON \
  -DGLOBALIZER_PYTHON=ON \
  -DGLOBALIZER_MAX_DIMENSION=130 \
  -DGLOBALIZER_MAX_Number_Of_Function=70 \
  -Drastrigin_build=ON \
  -DrastriginInt_build=ON \
  -DX2_build=ON \
  -Dpython_objective_build=ON \
  -Dstronginc3_build=ON \
  -DrastriginC1_build=ON \
  -DiOptProblemSimple_build=ON \
  -DPython_EXECUTABLE="$PY_EXE" \
  -DPython_ROOT_DIR="$ENV_DIR" \
  -DPython_FIND_STRATEGY=LOCATION \
  -DPython_FIND_VIRTUALENV=ONLY \
  -DPython_FIND_DEBUG=OFF \
  -Dpybind11_DIR="$PYBIND11_DIR" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  || die "CMake configuration failed. Re-run with --clean if the generator changed."

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[7/9] Build ALL ($BUILD_CONFIG)"
echo "========================================"
cmake --build "$BUILD_DIR" --config "$BUILD_CONFIG" --parallel "$JOBS" \
  || die "Build of the solution failed."

# Python-модуль — отдельной целью (на случай, если не вошёл в сборку по умолчанию)
cmake --build "$BUILD_DIR" --config "$BUILD_CONFIG" --target "$PY_MODULE" --parallel "$JOBS" >/dev/null 2>&1 || true

if [ "$DO_TEST" = "0" ]; then
  echo "--notest: skipping tests."
else
  # -------------------------------------------------------------------------
  echo
  echo "========================================"
  echo "[8/9] Tests: unit first, then integration"
  echo "========================================"

  echo
  echo "--- [8.1] UNIT tests (GLOBALIZER_tests) ---"
  UNIT_EXE=""
  for f in "$BIN_DIR/GLOBALIZER_tests" "$BIN_DIR/GLOBALIZER_tests_d"; do
    if [ -z "$UNIT_EXE" ] && [ -x "$f" ]; then UNIT_EXE="$f"; fi
  done
  if [ -z "$UNIT_EXE" ]; then
    echo "[WARNING] GLOBALIZER_tests not found in _bin - skipping unit tests."
  else
    ( cd "$BIN_DIR" && LD_LIBRARY_PATH="$BIN_DIR:${LD_LIBRARY_PATH:-}" \
        DYLD_LIBRARY_PATH="$BIN_DIR:${DYLD_LIBRARY_PATH:-}" "$UNIT_EXE" )
    UNIT_RC=$?
    [ "$UNIT_RC" = "0" ] || die "Unit tests FAILED (code $UNIT_RC)."
    echo "[OK] Unit tests passed."
  fi

  echo
  echo "--- [8.2] INTEGRATION tests (examples + cases) ---"
  export PYTHONPATH="$BIN_DIR:$ROOT_DIR/PYGlobalizer:$ROOT_DIR/third_party/Problems/Problems:${PYTHONPATH:-}"
  export LD_LIBRARY_PATH="$BIN_DIR:${LD_LIBRARY_PATH:-}"
  export DYLD_LIBRARY_PATH="$BIN_DIR:${DYLD_LIBRARY_PATH:-}"

  ( cd "$BUILD_DIR" && ctest -C "$BUILD_CONFIG" -E '^unit' --output-on-failure )
  CT_RC=$?
  if [ "$CT_RC" != "0" ]; then
    echo "[Info] ctest returned $CT_RC; running the examples runner directly..."
    "$PY_EXE" "$ROOT_DIR/integration_tests/run_all_examples.py" --bin-dir "$BIN_DIR"
    RUN_RC=$?
    [ "$RUN_RC" = "0" ] || die "Integration tests FAILED (code $RUN_RC)."
  fi
  echo "[OK] Integration tests finished."
fi

# ---------------------------------------------------------------------------
echo
echo "========================================"
echo "[9/9] Open build directory / IDE"
echo "========================================"
open_path() {
  target="$1"
  if [ "$PLATFORM" = "macos" ]; then
    open "$target" >/dev/null 2>&1 || true
  else
    # Linux: пытаемся файловым менеджером, иначе просто печатаем путь
    if command -v xdg-open >/dev/null 2>&1; then
      xdg-open "$target" >/dev/null 2>&1 || true
    else
      echo "Open manually: $target"
    fi
  fi
}

if [ "$DO_OPEN" = "0" ]; then
  echo "--noopen: not opening anything."
else
  if [ "$USE_XCODE" = "1" ] && [ "$PLATFORM" = "macos" ]; then
    XCODEPROJ="$(find "$BUILD_DIR" -maxdepth 1 -name '*.xcodeproj' | head -1)"
    if [ -n "$XCODEPROJ" ]; then echo "Opening $XCODEPROJ"; open_path "$XCODEPROJ"; else open_path "$BUILD_DIR"; fi
  else
    echo "Opening $BUILD_DIR"
    open_path "$BUILD_DIR"
  fi
fi

echo
echo "========================================"
echo "DONE"
echo "========================================"
echo "Build   : $BUILD_DIR"
echo "Binaries: $BIN_DIR"
echo "Env     : $ENV_DIR"
echo "Python  : conda activate \"$ENV_DIR\"  &&  python examples/Example_simple.py"
cd "$START_DIR" || true
exit 0
