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

add_definitions(-DGLOBALIZER_MAX_DIMENSION=10)
add_definitions(-DGLOBALIZER_MAX_Number_Of_Function=20)

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

# tests.json кладём рядом с исполняемым файлом.
add_custom_command(TARGET Globalizer_IntegrationTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/references/tests.json"
  "$<TARGET_FILE_DIR:Globalizer_IntegrationTests>/tests.json"
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

set(GLOBALIZER_CPP_EXAMPLES
  SimpleMain
  ConstraintsIterationMain
  NoConstraintsIterationMain
  IterationMain
  Main
  Main_ackley
  SampleMain
  HDOptimizationMain
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

message(STATUS "Globalizer integration tests (v4): integration_cases + examples_cpp/examples_python + per-example tests")
