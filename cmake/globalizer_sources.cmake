# ============================================================================
#  globalizer_sources.cmake
#  ---------------------------------------------------------------------------
#  Сбор списков заголовков и исходников библиотеки Globalizer_l по модулям.
#  Все переменные *_HEADERS / *_SOURCES и ALL_SRC_FILES объявляются в
#  родительском (directory) scope — так же, как в прежнем монолите.
#
#  Вызывать из корневого CMakeLists ПОСЛЕ определения GLOBALIZER_*_DIR
#  и ПОСЛЕ подключения problems (чтобы знать GLOBALIZER_BENCHMARKS_Interface).
# ============================================================================

macro(globalizer_collect_sources)
  # --- Заголовки по модулям --------------------------------------------------
  file(GLOB ROOT_HEADERS          ${GLOBALIZER_COMMON_SRC_DIR}/include/*.h    ${GLOBALIZER_COMMON_SRC_DIR}/include/*.hpp)
  file(GLOB evolvent_HEADERS      ${GLOBALIZER_evolvent_DIR}/include/*.h      ${GLOBALIZER_evolvent_DIR}/include/*.hpp)
  file(GLOB extended_HEADERS      ${GLOBALIZER_extended_DIR}/include/*.h      ${GLOBALIZER_extended_DIR}/include/*.hpp)
  file(GLOB output_system_HEADERS ${GLOBALIZER_output_system_DIR}/include/*.h ${GLOBALIZER_output_system_DIR}/include/*.hpp)
  file(GLOB problem_HEADERS       ${GLOBALIZER_problem_DIR}/include/*.h       ${GLOBALIZER_problem_DIR}/include/*.hpp)
  file(GLOB method_HEADERS        ${GLOBALIZER_method_DIR}/include/*.h        ${GLOBALIZER_method_DIR}/include/*.hpp)
  file(GLOB calculation_HEADERS   ${GLOBALIZER_calculation_DIR}/include/*.h   ${GLOBALIZER_calculation_DIR}/include/*.hpp)
  file(GLOB local_method_HEADERS  ${GLOBALIZER_local_method_DIR}/include/*.h  ${GLOBALIZER_local_method_DIR}/include/*.hpp)
  file(GLOB PARAMETERS_HEADERS    ${GLOBALIZER_PARAMETERS_DIR}/include/*.h    ${GLOBALIZER_PARAMETERS_DIR}/include/*.hpp)

  # --- Исходники по модулям --------------------------------------------------
  file(GLOB ROOT_SOURCES          ${GLOBALIZER_COMMON_SRC_DIR}/src/*.c    ${GLOBALIZER_COMMON_SRC_DIR}/src/*.cpp)
  file(GLOB evolvent_SOURCES      ${GLOBALIZER_evolvent_DIR}/src/*.c      ${GLOBALIZER_evolvent_DIR}/src/*.cpp)
  file(GLOB extended_SOURCES      ${GLOBALIZER_extended_DIR}/src/*.c      ${GLOBALIZER_extended_DIR}/src/*.cpp)
  file(GLOB output_system_SOURCES ${GLOBALIZER_output_system_DIR}/src/*.c ${GLOBALIZER_output_system_DIR}/src/*.cpp)
  file(GLOB problem_SOURCES       ${GLOBALIZER_problem_DIR}/src/*.c       ${GLOBALIZER_problem_DIR}/src/*.cpp)
  file(GLOB method_SOURCES        ${GLOBALIZER_method_DIR}/src/*.c        ${GLOBALIZER_method_DIR}/src/*.cpp)
  file(GLOB calculation_SOURCES   ${GLOBALIZER_calculation_DIR}/src/*.c   ${GLOBALIZER_calculation_DIR}/src/*.cpp)
  file(GLOB local_method_SOURCES  ${GLOBALIZER_local_method_DIR}/src/*.c  ${GLOBALIZER_local_method_DIR}/src/*.cpp)
  file(GLOB PARAMETERS_SOURCES    ${GLOBALIZER_PARAMETERS_DIR}/src/*.c    ${GLOBALIZER_PARAMETERS_DIR}/src/*.cpp)

  # --- Единый список для цели библиотеки -------------------------------------
  set(ALL_SRC_FILES
      ${ROOT_HEADERS} ${PARAMETERS_HEADERS} ${ROOT_SOURCES}
      ${evolvent_HEADERS} ${extended_HEADERS} ${output_system_HEADERS}
      ${problem_HEADERS} ${method_HEADERS} ${calculation_HEADERS}
      ${local_method_HEADERS} ${GLOBALIZER_BENCHMARKS_Interface_HEADERS}
      ${evolvent_SOURCES} ${extended_SOURCES} ${output_system_SOURCES}
      ${problem_SOURCES} ${method_SOURCES} ${calculation_SOURCES}
      ${local_method_SOURCES} ${PARAMETERS_SOURCES}
      ${GLOBALIZER_BENCHMARKS_Interface_SOURCES})

  # Главный Main.cpp исключаем — библиотека без точки входа.
  list(REMOVE_ITEM ALL_SRC_FILES ${GLOBALIZER_COMMON_SRC_DIR}/Main.cpp)
endmacro()
