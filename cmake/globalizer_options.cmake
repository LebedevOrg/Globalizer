# ============================================================================
#  globalizer_options.cmake
#  ---------------------------------------------------------------------------
#  Объявление всех опций сборки, кэш-переменных и производных имён целей.
#  Выполняется через include() в корневом CMakeLists (directory scope),
#  поэтому все заданные здесь переменные видны далее без изменений.
#
#  ВАЖНО: имена опций и переменных НЕ меняются — совместимость с bat-файлами
#  и существующими командными строками сборки.
# ============================================================================

# --- Пути/режимы MPI и уровень вывода ---------------------------------------
set(GLOBALIZER_MPI_LIB_USER_PATH     "" CACHE STRING "Path to MPI libraries")
set(GLOBALIZER_MPI_INCLUDE_USER_PATH "" CACHE STRING "Path to MPI includes")
set(GLOBALIZER_MPI      "ms" CACHE STRING "Set MPI implementation (ms, mpich, intel)")
set(GLOBALIZER_OUTPUT_LEVEL "L0" CACHE STRING "Set level of output (L0, L1, L2)")

# --- Основные переключатели сборки ------------------------------------------
option(GLOBALIZER_USE_MP   "Set OFF to disable MPFR (extended precision in evolvent will unavaliable)" OFF)
option(GLOBALIZER_USE_MPI  "Set OFF to disable MPI.
  In this case a stub implementation of MPI will be used and globalizer won't support multi-process mode" OFF)
option(GLOBALIZER_USE_CUDA "Set ON to use CUDA" OFF)
option(GLOBALIZER_BUILD_TESTS "Set ON to build tests" OFF)
option(GLOBALIZER_CMAKE_VERBOSE "Set ON to show debug messages in CMake" ON)
option(GLOBALIZER_BUILD_PROBLEMS "Set ON to build problems" OFF)
option(GLOBALIZER_USE_DOXYGEN "Set ON to build documentation" OFF)
option(GLOBALIZER_BUILD_GCGEN "Set ON to build GCGen" OFF)
option(GLOBALIZER_BUILD_INTEGRATION_TESTS "Set ON to build integration tests" OFF)

option(GLOBALIZER_ONE_MPI_PROCESS_PER_NODE "ONE PROCESS PER NODE" OFF)
option(GLOBALIZER_PYTHON "Set ON to use Python" OFF)
option(GLOBALIZER_PYBIND_VERSION "Set ON to use Python interface" OFF)

option(GLOBALIZER_USE_ASAN "Set ON to build with AddressSanitizer (diagnostics)" OFF)

# Опции GLOBALIZER_BUILD_AS_LIBRARY_ONLY и BUILD_SHARED_LIBS объявлены в
# корневом CMakeLists ПОСЛЕ project() (зависят от него).

# --- Имена целей -------------------------------------------------------------
set(GLOBALIZER_DEBUG_SUFFIX d)

set(GLOBALIZER_EXECUTABLE_NAME         ${GLOBALIZER_PROJECT_NAME}${GLOBALIZER_EXECUTABLE_SUFFIX})
set(GLOBALIZER_EXECUTABLE_NAME_DEBUG   ${GLOBALIZER_EXECUTABLE_NAME}_${GLOBALIZER_DEBUG_SUFFIX})
set(GLOBALIZER_EXECUTABLE_NAME_RELEASE ${GLOBALIZER_EXECUTABLE_NAME})
set(GLOBALIZER_LIB_NAME                ${GLOBALIZER_EXECUTABLE_NAME}_l)
set(GLOBALIZER_LIB_NAME_RELEASE        ${GLOBALIZER_EXECUTABLE_NAME}_l)
set(GLOBALIZER_LIB_NAME_DEBUG          ${GLOBALIZER_EXECUTABLE_NAME}_l_${GLOBALIZER_DEBUG_SUFFIX})
set(GLOBALIZER_PYD_NAME                PYDGlobalizer)

# --- Каталоги исходников -----------------------------------------------------
# Используем CMAKE_SOURCE_DIR, а не PROJECT_SOURCE_DIR: этот модуль
# подключается ДО project(), когда PROJECT_SOURCE_DIR ещё пуст. На верхнем
# уровне CMAKE_SOURCE_DIR указывает на корень репозитория и доступен сразу.
set(GLOBALIZER_EXAMPLES_SRC_DIR  ${CMAKE_SOURCE_DIR}/examples)
set(GLOBALIZER_COMMON_SRC_DIR    ${CMAKE_SOURCE_DIR}/globalizer)
set(GLOBALIZER_PARAMETERS_DIR    ${GLOBALIZER_COMMON_SRC_DIR}/parameters)
set(GLOBALIZER_evolvent_DIR      ${GLOBALIZER_COMMON_SRC_DIR}/evolvent)
set(GLOBALIZER_extended_DIR      ${GLOBALIZER_COMMON_SRC_DIR}/extended)
set(GLOBALIZER_output_system_DIR ${GLOBALIZER_COMMON_SRC_DIR}/output_system)
set(GLOBALIZER_problem_DIR       ${GLOBALIZER_COMMON_SRC_DIR}/problem)
set(GLOBALIZER_method_DIR        ${GLOBALIZER_COMMON_SRC_DIR}/method)
set(GLOBALIZER_calculation_DIR   ${GLOBALIZER_COMMON_SRC_DIR}/method/calculation)
set(GLOBALIZER_local_method_DIR  ${GLOBALIZER_COMMON_SRC_DIR}/method/local_method)
