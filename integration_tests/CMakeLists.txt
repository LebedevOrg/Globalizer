project(Globalizer_IntegrationTests)

# ============================================================================
#  Интеграционные тесты Globalizer
#  ---------------------------------------------------------------------------
#  1) Globalizer_IntegrationTests  — прежний набор кейсов (tests.json);
#  2) CTest-тесты, запускающие ВСЕ примеры из examples/ (C++ и Python)
#     через единый раннер run_all_examples.py.
#
#  Всё расширение тестов — ТОЛЬКО в этой папке. В ядре нужна одна строка
#  enable_testing() в корневом CMakeLists (в блоке GLOBALIZER_BUILD_INTEGRATION_TESTS).
# ============================================================================

# ABI-макросы (GLOBALIZER_MAX_DIMENSION, GLOBALIZER_MAX_Number_Of_Function)
# приходят от Globalizer_l (PUBLIC). Переопределять их здесь нельзя: раскладка
# Trial/Parameters в тестах разошлась бы с библиотекой.

file(GLOB BENCHMARK_INTERFACE_SOURCES
  "${CMAKE_SOURCE_DIR}/third_party/Problems/Interface/src/*.cpp"
)

add_executable(Globalizer_IntegrationTests
  src/main.cpp
  ${BENCHMARK_INTERFACE_SOURCES}
)

target_include_directories(Globalizer_IntegrationTests PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party/Problems/Interface/include
  ${CMAKE_SOURCE_DIR}/lib/nlohmann_json
)

target_link_libraries(Globalizer_IntegrationTests PRIVATE Globalizer_l)

# ---------------------------------------------------------------------------
#  Исправление LNK1104: python312.lib. При GLOBALIZER_PYTHON=ON библиотека
#  Globalizer_l транзитивно требует python<ver>.lib. Подключаем Python к
#  таргету тестов (сам тест Python не использует — зависимость транзитивная).
if (GLOBALIZER_PYTHON OR GLOBALIZER_PYBIND_VERSION)
  find_package(Python COMPONENTS Interpreter Development QUIET)
  if (Python_FOUND)
    message(STATUS "IntegrationTests: linking Python from ${Python_LIBRARY_DIRS}")
    if (TARGET Python::Python)
      target_link_libraries(Globalizer_IntegrationTests PRIVATE Python::Python)
    endif()
    if (Python_LIBRARY_DIRS)
      target_link_directories(Globalizer_IntegrationTests PRIVATE ${Python_LIBRARY_DIRS})
    endif()
    if (Python_ROOT_DIR)
      target_link_directories(Globalizer_IntegrationTests PRIVATE "${Python_ROOT_DIR}/libs")
    endif()
  else()
    message(WARNING "IntegrationTests: Python not found but GLOBALIZER_PYTHON is ON. "
                    "Linking may fail with LNK1104 python*.lib.")
  endif()
endif()

# tests.json и iopt_tests.json кладём рядом с исполняемым файлом.
add_custom_command(TARGET Globalizer_IntegrationTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/references/tests.json"
  "$<TARGET_FILE_DIR:Globalizer_IntegrationTests>/tests.json"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/references/iopt_tests.json"
  "$<TARGET_FILE_DIR:Globalizer_IntegrationTests>/iopt_tests.json"
)

if (MSVC)
  set_target_properties(Globalizer_IntegrationTests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}")
endif()

# ---------------------------------------------------------------------------
#  Регистрация тестов в CTest (enable_testing() вызван в корневом CMakeLists).
# ---------------------------------------------------------------------------
set(GLOBALIZER_TEST_WORKDIR "${GLOBALIZER_OUTPUT_DIRECTORY}")

add_test(NAME integration_cases
  COMMAND $<TARGET_FILE:Globalizer_IntegrationTests>
  WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
set_tests_properties(integration_cases PROPERTIES TIMEOUT 1800)

# ---------------------------------------------------------------------------
#  Тесты, перенесённые из iOpt (references/iopt_tests.json), по группам:
#    iopt_solving       — test_solving_test_problems.py (точное число испытаний)
#    iopt_gkls100       — 100 задач GKLS 2D (test_Solve_100_GKLS_2D_problem)
#    iopt_mixed_integer — test_mixed_integer_method.py, test_RastriginInt_Solve
#    iopt_load_progress — test_load_progress.py (сохранение/загрузка прогресса)
#    benchmark_libs     — задачи Globalizer_Benchmarks, которые использует SampleMain
# ---------------------------------------------------------------------------
set(GLOBALIZER_IOPT_GROUPS
  iopt_solving
  iopt_gkls100
  iopt_mixed_integer
  iopt_load_progress
  benchmark_libs
)
set(_integration_tests integration_cases)
foreach(_group ${GLOBALIZER_IOPT_GROUPS})
  add_test(NAME ${_group}
    # JSON читается из исходников: правка эталонов не требует пересборки.
    COMMAND $<TARGET_FILE:Globalizer_IntegrationTests>
            ${CMAKE_CURRENT_SOURCE_DIR}/references/iopt_tests.json --group ${_group}
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(${_group} PROPERTIES TIMEOUT 1800)
  list(APPEND _integration_tests ${_group})
endforeach()


find_package(Python3 COMPONENTS Interpreter QUIET)
if (Python3_Interpreter_FOUND)
  set(GLOBALIZER_PY ${Python3_EXECUTABLE})
else()
  set(GLOBALIZER_PY python)
endif()

set(GLOBALIZER_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_all_examples.py")

add_test(NAME examples_cpp
  COMMAND ${GLOBALIZER_PY} ${GLOBALIZER_RUNNER}
          --only cpp --bin-dir ${GLOBALIZER_TEST_WORKDIR}
  WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
set_tests_properties(examples_cpp PROPERTIES TIMEOUT 3600)

add_test(NAME examples_python
  COMMAND ${GLOBALIZER_PY} ${GLOBALIZER_RUNNER}
          --only python --bin-dir ${GLOBALIZER_TEST_WORKDIR}
  WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
set_tests_properties(examples_python PROPERTIES TIMEOUT 3600)

# Интерфейс globalizer.optimize, совместимый с scipy.optimize (нужен модуль
# PYDGlobalizer, т.е. сборка с GLOBALIZER_PYBIND_VERSION=ON).
if (GLOBALIZER_PYBIND_VERSION)
  # Интерпретатор - тот, под который собран модуль (его версия в имени .pyd).
  if (Python_EXECUTABLE)
    set(_scipy_iface_py ${Python_EXECUTABLE})
  else()
    set(_scipy_iface_py ${GLOBALIZER_PY})
  endif()
  add_test(NAME python_scipy_interface
    COMMAND ${_scipy_iface_py} ${CMAKE_CURRENT_SOURCE_DIR}/test_globalizer_optimize.py
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(python_scipy_interface PROPERTIES TIMEOUT 600
    ENVIRONMENT "GLOBALIZER_BIN=${GLOBALIZER_OUTPUT_DIRECTORY}")
  list(APPEND _integration_tests python_scipy_interface)

  # Пошаговая работа PYDGlobalizer.IterativeSolver (аналог IterationMain.cpp).
  add_test(NAME python_iterative_solver
    COMMAND ${_scipy_iface_py} ${CMAKE_CURRENT_SOURCE_DIR}/test_pyd_iterative.py
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(python_iterative_solver PROPERTIES TIMEOUT 600
    ENVIRONMENT "GLOBALIZER_BIN=${GLOBALIZER_OUTPUT_DIRECTORY}")
  list(APPEND _integration_tests python_iterative_solver)

  # Интерфейс globalizer.optuna, совместимый с Optuna (сама Optuna не нужна;
  # если она установлена, проверяется ещё и совместимость сигнатур).
  add_test(NAME python_optuna_interface
    COMMAND ${_scipy_iface_py} ${CMAKE_CURRENT_SOURCE_DIR}/test_globalizer_optuna.py
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(python_optuna_interface PROPERTIES TIMEOUT 600
    ENVIRONMENT "GLOBALIZER_BIN=${GLOBALIZER_OUTPUT_DIRECTORY}")
  list(APPEND _integration_tests python_optuna_interface)

  # Короткий прогон сравнения Globalizer / scipy / Optuna (examples/compare_optimizers.py).
  # Без scipy или optuna скрипт возвращает 77 - тест помечается пропущенным.
  add_test(NAME python_optimizer_comparison_smoke
    COMMAND ${_scipy_iface_py} ${CMAKE_SOURCE_DIR}/examples/compare_optimizers.py
            --budgets 60 --seeds 1 --problems branin circle mixed3 --quiet
            --out ${CMAKE_CURRENT_BINARY_DIR}/comparison_smoke
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(python_optimizer_comparison_smoke PROPERTIES TIMEOUT 600
    SKIP_RETURN_CODE 77
    ENVIRONMENT "GLOBALIZER_BIN=${GLOBALIZER_OUTPUT_DIRECTORY}")
  list(APPEND _integration_tests python_optimizer_comparison_smoke)
endif()

set(GLOBALIZER_CPP_EXAMPLES
  SimpleMain
  ConstraintsIterationMain
  NoConstraintsIterationMain
  IterationMain
  Main
  Main_ackley
  SampleMain
  DynamicProblemMain
  GCGenMain
  GCGenGKLSMain
)

foreach(_ex ${GLOBALIZER_CPP_EXAMPLES})
  add_test(NAME example_${_ex}
    COMMAND ${GLOBALIZER_PY} ${GLOBALIZER_RUNNER}
            --only cpp --filter ${_ex} --bin-dir ${GLOBALIZER_TEST_WORKDIR}
    WORKING_DIRECTORY ${GLOBALIZER_TEST_WORKDIR})
  set_tests_properties(example_${_ex} PROPERTIES TIMEOUT 1800)
endforeach()

# Globalizer_l и примеры линкуются с Python: на Windows всем тестам этой папки
# нужна pythonXY.dll (иначе exit code 0xC0000135, если окружение не активировано).
list(APPEND _integration_tests examples_cpp examples_python)
foreach(_ex ${GLOBALIZER_CPP_EXAMPLES})
  list(APPEND _integration_tests example_${_ex})
endforeach()
if (WIN32 AND Python_EXECUTABLE)
  get_filename_component(_it_python_dir "${Python_EXECUTABLE}" DIRECTORY)
  set_tests_properties(${_integration_tests} PROPERTIES
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${_it_python_dir}")
endif()

message(STATUS "Globalizer integration tests (v5): integration_cases + iOpt groups + examples_cpp/examples_python + per-example tests")
