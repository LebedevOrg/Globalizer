# ============================================================================
#  Globalizer — корневой CMakeLists (slim-оркестратор)
#  ---------------------------------------------------------------------------
#  Логика вынесена в тематические модули cmake/globalizer_*.cmake:
#    globalizer_options.cmake       — опции, кэш-переменные, имена/пути
#    globalizer_utils.cmake         — утилиты (компилятор, арх, пути)   [прежний]
#    globalizer_mpi.cmake           — MPI / stub-MPI                     [прежний]
#    globalizer_lib.cmake           — MPFR/GMP/MPIR пути                 [прежний]
#    globalizer_doxygen.cmake       — документация                      [прежний]
#    globalizer_compiler.cmake      — флаги, ASan, ABI, build type
#    globalizer_dependencies.cmake  — MPI/MP/CUDA/Python/problems/GCGen
#    globalizer_sources.cmake       — сбор исходников библиотеки
#    globalizer_targets.cmake       — библиотека, примеры, pybind-модуль
#    globalizer_ide.cmake           — раскладка проектов и source_group (VS)
#    globalizer_install.cmake       — install/export + GlobalizerConfig
#
#  ВАЖНО: имена опций, целей, переменных и структура вывода (_bin) сохранены
#  1:1 — все прежние командные строки сборки и bat-файлы продолжают работать.
# ============================================================================

set(GLOBALIZER_PROJECT_NAME Globalizer)
cmake_minimum_required(VERSION 3.31)

cmake_policy(SET CMP0057 NEW) # поддержка оператора IN_LIST

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available build types" FORCE)
set(CMAKE_CXX_STANDARD 17)

# --- Опции и переменные (до project(): часть имён целей от них зависит) ------
include(cmake/globalizer_options.cmake)

project(${GLOBALIZER_PROJECT_NAME})

# Эти опции зависят от project()/BUILD_SHARED_LIBS — объявляем после project().
option(GLOBALIZER_BUILD_AS_LIBRARY_ONLY "Собрать только библиотеку (пропустить примеры/.exe)" OFF)
option(BUILD_SHARED_LIBS "Собрать динамическую библиотеку (DLL/.so) вместо статической (.lib/.a)" OFF)

# --- Каталог вывода (_bin) ---------------------------------------------------
set(GLOBALIZER_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/_bin CACHE STRING "Output directory")
message("GLOBALIZER_OUTPUT_DIRECTORY" ${GLOBALIZER_OUTPUT_DIRECTORY})

if(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE AND
   NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE STREQUAL "")
  message("CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${GLOBALIZER_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GLOBALIZER_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${GLOBALIZER_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${GLOBALIZER_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG   ${GLOBALIZER_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${GLOBALIZER_OUTPUT_DIRECTORY})
endif()

# --- Подключение модулей -----------------------------------------------------
include(cmake/globalizer_utils.cmake)
include(cmake/globalizer_mpi.cmake)
include(cmake/globalizer_lib.cmake)
include(cmake/globalizer_compiler.cmake)
include(cmake/globalizer_dependencies.cmake)
include(cmake/globalizer_sources.cmake)
include(cmake/globalizer_targets.cmake)
include(cmake/globalizer_ide.cmake)
include(cmake/globalizer_install.cmake)
if (GLOBALIZER_USE_DOXYGEN)
  include(cmake/globalizer_doxygen.cmake)
endif()

# --- Инициализация окружения -------------------------------------------------
GLOBALIZER_initialize()

globalizer_init_abi_definitions()      # -> GLOBALIZER_ABI_DEFINITIONS
globalizer_setup_compiler_flags()
globalizer_setup_asan()
globalizer_normalize_build_type()

if (GLOBALIZER_USE_DOXYGEN)
  init_doxygen()
endif()

# --- Зависимости -------------------------------------------------------------
globalizer_configure_mpi()                          # -> GLOBALIZER_MPI_LIB / _INCLUDES
globalizer_configure_mp(GLOBALIZER_ABI_DEFINITIONS) # MPFR/GMP/MPIR + NO_MPREAL/ONE_MPI...

# Include-пути и link-директории (directory scope — до создания целей).
include_directories(
  ${GLOBALIZER_MPI_INCLUDES}
  ${GLOBALIZER_COMMON_SRC_DIR}/include
  ${GLOBALIZER_PARAMETERS_DIR}/include
  ${GLOBALIZER_evolvent_DIR}/include
  ${GLOBALIZER_extended_DIR}/include
  ${GLOBALIZER_output_system_DIR}/include
  ${GLOBALIZER_problem_DIR}/include
  ${GLOBALIZER_method_DIR}/include
  ${GLOBALIZER_calculation_DIR}/include
  ${GLOBALIZER_local_method_DIR}/include
)
link_directories(
  ${GLOBALIZER_MPIR_LIB_DIR}
  ${GLOBALIZER_GMP_LIB_DIR}
  ${GLOBALIZER_MPFR_LIB_DIR}
)

globalizer_configure_cuda()
globalizer_configure_problems(GLOBALIZER_ABI_DEFINITIONS)  # problems + _GLOBALIZER_BENCHMARKS
globalizer_configure_gcgen()                               # GCGen + _GLOBALIZER_GCGEN

# --- Сбор исходников библиотеки ----------------------------------------------
globalizer_collect_sources()                              # -> ALL_SRC_FILES и *_SOURCES/_HEADERS

# --- Python + OpenMP (до add_library) ----------------------------------------
globalizer_configure_python(GLOBALIZER_ABI_DEFINITIONS)   # Python_* + USE_PYTHON
# OpenMP ищем ДО add_library: цель OpenMP::OpenMP_CXX должна существовать
# к моменту target_link_libraries и попасть в экспорт вместе с флагами.
find_package(OpenMP REQUIRED)

# --- Цели --------------------------------------------------------------------
globalizer_add_main_library()
globalizer_add_examples()
globalizer_add_pybind_module()

# Include Python-путей в directory scope (как в прежнем монолите — для целей,
# созданных ниже, например подпроектов).
include_directories(${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS})

globalizer_link_main_library()

# --- Интеграционные тесты ----------------------------------------------------
if(GLOBALIZER_BUILD_INTEGRATION_TESTS)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/integration_tests)
  set_target_properties(Globalizer_IntegrationTests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}")
endif()

# --- Модульные тесты ---------------------------------------------------------
if(GLOBALIZER_BUILD_TESTS)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/gtest)
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

# --- Диагностика конфигурации ------------------------------------------------
if(GLOBALIZER_CMAKE_VERBOSE)
  print_build_config()
endif()

# --- Раскладка для IDE (Visual Studio) и установка ---------------------------
globalizer_setup_ide_layout()
globalizer_setup_install()
