#!/usr/bin/env bash
# ============================================================================
#  configure_no_mp_no_mpi.sh   (Linux / macOS)
#
#  POSIX-аналог второго bat-файла (build_64_nmmp): лёгкая КОНФИГУРАЦИЯ без
#  Python / MP / MPI, затем открытие каталога сборки (на Windows это был
#  запуск globalizer.sln в Visual Studio).
#
#  Не собирает и не тестирует — только инициализирует сабмодули и готовит
#  дерево сборки, как исходный .bat. Для полного цикла используйте
#  build_test_and_open.sh.
#
#  Ключи:
#    (без ключей)   конфигурация + открыть каталог сборки
#    --build        дополнительно собрать после конфигурации
#    --noopen       не открывать каталог/IDE
#    --xcode        (только macOS) генератор Xcode и открытие .xcodeproj
#    --jobs N       число потоков при --build (по умолчанию: nproc)
# ============================================================================
set -u

DO_OPEN=1
DO_BUILD=0
USE_XCODE=0
JOBS=""
PREV=""
for arg in "$@"; do
  case "$arg" in
    --noopen) DO_OPEN=0 ;;
    --build)  DO_BUILD=1 ;;
    --xcode)  USE_XCODE=1 ;;
    --jobs=*) JOBS="${arg#*=}" ;;
    *) [ "$PREV" = "--jobs" ] && JOBS="$arg" ;;
  esac
  PREV="$arg"
done

OS="$(uname -s)"
case "$OS" in
  Linux*)  PLATFORM="linux" ;;
  Darwin*) PLATFORM="macos" ;;
  *)       PLATFORM="unknown" ;;
esac

if [ -z "$JOBS" ]; then
  if command -v nproc >/dev/null 2>&1; then JOBS="$(nproc)";
  elif [ "$PLATFORM" = "macos" ]; then JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)";
  else JOBS=4; fi
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
START_DIR="$(pwd)"
BUILD_DIR="$ROOT_DIR/build_64_nmmp"

die() {
  echo
  echo "========================================"
  echo " BUILD CONFIGURATION FAILED"
  echo " $1"
  echo "========================================"
  cd "$START_DIR" || true
  exit 1
}

cd "$ROOT_DIR" || die "Cannot cd to root directory."
mkdir -p "$BUILD_DIR"

# --- Сабмодули ---------------------------------------------------------------
if command -v git >/dev/null 2>&1; then
  git -C "$ROOT_DIR" submodule init
  git -C "$ROOT_DIR" submodule update
else
  echo "[WARNING] git not found - skipping submodules."
fi

# --- Выбор генератора --------------------------------------------------------
GENERATOR=""
if [ "$USE_XCODE" = "1" ] && [ "$PLATFORM" = "macos" ]; then
  GENERATOR="Xcode"
elif command -v ninja >/dev/null 2>&1; then
  GENERATOR="Ninja"
else
  GENERATOR="Unix Makefiles"
fi

BUILDTYPE_ARG=""
[ "$GENERATOR" = "Xcode" ] || BUILDTYPE_ARG="-DCMAKE_BUILD_TYPE=Release"

command -v cmake >/dev/null 2>&1 || die "cmake not found."

echo "[1/2] CMake Configuration (no MP / no MPI / no Python)..."
echo "      Platform: $PLATFORM   Generator: $GENERATOR"
# shellcheck disable=SC2086
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G "$GENERATOR" $BUILDTYPE_ARG \
  -DGLOBALIZER_BUILD_PROBLEMS=ON \
  -DGLOBALIZER_MAX_DIMENSION=130 \
  -DGLOBALIZER_MAX_Number_Of_Function=70 \
  -DGLOBALIZER_PYTHON=OFF \
  -DGLOBALIZER_BUILD_GCGEN=ON \
  -Drastrigin_build=ON \
  -DrastriginInt_build=ON \
  -DX2_build=ON \
  -Dstronginc3_build=ON \
  -DrastriginC1_build=ON \
  -DGLOBALIZER_BENCHMARKS_PYTHON=OFF \
  -DGLOBALIZER_BUILD_TESTS=ON \
  -DGLOBALIZER_USE_MP=OFF \
  -DGLOBALIZER_USE_MPI=OFF \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  || die "CMake configuration failed."

# --- Необязательная сборка ---------------------------------------------------
if [ "$DO_BUILD" = "1" ]; then
  echo "[+] Building..."
  cmake --build "$BUILD_DIR" --config Release --parallel "$JOBS" || die "Build failed."
fi

# --- Открытие ----------------------------------------------------------------
echo "[2/2] Open build directory / IDE..."
open_path() {
  target="$1"
  if [ "$PLATFORM" = "macos" ]; then
    open "$target" >/dev/null 2>&1 || true
  elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$target" >/dev/null 2>&1 || true
  else
    echo "Open manually: $target"
  fi
}

if [ "$DO_OPEN" = "1" ]; then
  if [ "$USE_XCODE" = "1" ] && [ "$PLATFORM" = "macos" ]; then
    XCODEPROJ="$(find "$BUILD_DIR" -maxdepth 1 -name '*.xcodeproj' | head -1)"
    [ -n "$XCODEPROJ" ] && open_path "$XCODEPROJ" || open_path "$BUILD_DIR"
  else
    open_path "$BUILD_DIR"
  fi
else
  echo "--noopen: not opening anything."
fi

cd "$START_DIR" || true
exit 0
