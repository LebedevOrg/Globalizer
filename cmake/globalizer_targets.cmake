# ============================================================================
#  globalizer_targets.cmake
#  ---------------------------------------------------------------------------
#  Создание целей: основная библиотека Globalizer_l, примеры из examples/,
#  python-модуль PYDGlobalizer, а также обеспечение порядка сборки MP-библиотек.
#
#  Поведение и имена целей идентичны прежнему монолиту:
#    - Globalizer_l (STATIC/SHARED), OUTPUT_NAME_DEBUG = *_l_d
#    - Globalizer<Пример> для каждого examples/*.cpp по тем же правилам отбора
#    - PYDGlobalizer через pybind11_add_module
# ============================================================================

# ---------------------------------------------------------------------------
#  Основная библиотека Globalizer_l.
# ---------------------------------------------------------------------------
function(globalizer_add_main_library)
  set(_glib_type STATIC)
  if(BUILD_SHARED_LIBS)
    set(_glib_type SHARED)
  endif()

  add_library(${GLOBALIZER_LIB_NAME} ${_glib_type}
              ${ALL_SRC_FILES} ${GCGEN_SOURCES} ${GCGEN_HEADERS})

  target_compile_definitions(${GLOBALIZER_LIB_NAME} PUBLIC ${GLOBALIZER_ABI_DEFINITIONS})

  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${GLOBALIZER_LIB_NAME} PRIVATE GLOBALIZER_EXPORTS)
    if(NOT MSVC)
      target_compile_options(${GLOBALIZER_LIB_NAME} PRIVATE -fvisibility=hidden)
    endif()
  else()
    target_compile_definitions(${GLOBALIZER_LIB_NAME} PUBLIC GLOBALIZER_STATIC)
  endif()

  target_include_directories(${GLOBALIZER_LIB_NAME} PUBLIC
      $<BUILD_INTERFACE:${GLOBALIZER_COMMON_SRC_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_PARAMETERS_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_evolvent_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_extended_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_output_system_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_problem_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_method_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_calculation_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_local_method_DIR}/include>
      $<BUILD_INTERFACE:${GLOBALIZER_BENCHMARKS_Interface}/include/>
      $<INSTALL_INTERFACE:include>
  )
endfunction()

# ---------------------------------------------------------------------------
#  Привязка зависимостей к библиотеке + имена выходных файлов + порядок MP.
# ---------------------------------------------------------------------------
function(globalizer_link_main_library)
  if(GLOBALIZER_USE_MP)
    set(_mp_link
        $<BUILD_INTERFACE:${GLOBALIZER_MPFR_LIB}>
        $<BUILD_INTERFACE:${GLOBALIZER_MPIR_LIB}>
        ${GLOBALIZER_GMP_LIB})
  else()
    set(_mp_link "")
  endif()

  # Приватные зависимости (в экспорт не попадают).
  target_link_libraries(${GLOBALIZER_LIB_NAME} PRIVATE
      ${GLOBALIZER_MPI_LIB}
      ${_mp_link}
      ${DISLIN_LIB})

  # OpenMP — PUBLIC для всех компиляторов: статическая библиотека не содержит
  # рантайм OpenMP, его обязан разрешить потребитель, поэтому зависимость
  # экспортируется.
  target_link_libraries(${GLOBALIZER_LIB_NAME} PUBLIC OpenMP::OpenMP_CXX)

  # Порядок сборки MP-библиотек (gmp/mpfr/mpir): они линкуются "по имени"
  # через link_directories, а не как CMake-цели, поэтому CMake не видит
  # зависимость и MSBuild может собрать Globalizer_l раньше gmp.
  if (GLOBALIZER_USE_MP)
    set(_mp_targets "")
    foreach(_mp_target libgmp mpir mpfr)
      if (TARGET ${_mp_target})
        list(APPEND _mp_targets ${_mp_target})
      else()
        message(WARNING "MP dependency target '${_mp_target}' not found "
                        "(check third_party/mpfr_gmp_mpir/*/CMakeLists.txt).")
      endif()
    endforeach()
    if (_mp_targets)
      add_dependencies(${GLOBALIZER_LIB_NAME} ${_mp_targets})
      message(STATUS "MP build order enforced: ${GLOBALIZER_LIB_NAME} depends on ${_mp_targets}")
    endif()
  endif()

  message(STATUS "MPI libraries: ${GLOBALIZER_MPI_LIB}")

  set_target_properties(${GLOBALIZER_LIB_NAME} PROPERTIES
      OUTPUT_NAME_DEBUG   ${GLOBALIZER_LIB_NAME_DEBUG}
      OUTPUT_NAME_RELEASE ${GLOBALIZER_LIB_NAME_RELEASE})
endfunction()

# ---------------------------------------------------------------------------
#  Один исполняемый пример из examples/<sample>.cpp.
#  Инкапсулирует правила отбора и линковки, которые были в прежнем цикле.
# ---------------------------------------------------------------------------
function(globalizer_add_example main_filename)
  get_filename_component(sample ${main_filename} NAME_WE)

  # GCGen-примеры собираем только при включённом GCGen (иначе нет заголовков).
  if((sample MATCHES "GCGen") AND (NOT GLOBALIZER_BUILD_GCGEN))
    return()
  endif()

  # SimpleMain доступен всегда; остальные — только при GLOBALIZER_BUILD_PROBLEMS.
  if(NOT (GLOBALIZER_BUILD_PROBLEMS OR (sample STREQUAL "SimpleMain")))
    return()
  endif()

  set(_exe        ${GLOBALIZER_PROJECT_NAME}${sample}${GLOBALIZER_EXECUTABLE_SUFFIX})
  set(_exe_debug  ${_exe}_${GLOBALIZER_DEBUG_SUFFIX})
  set(_exe_release ${_exe})

  message("Name: " ${_exe})
  add_executable(${_exe} ${main_filename})

  if (GLOBALIZER_USE_MP)
    foreach(_mp_target libgmp mpir mpfr)
      if (TARGET ${_mp_target})
        add_dependencies(${_exe} ${_mp_target})
      endif()
    endforeach()
  endif()

  target_include_directories(${_exe} PRIVATE
      ${GLOBALIZER_COMMON_SRC_DIR}
      ${Python_INCLUDE_DIRS}
      ${Python_NumPy_INCLUDE_DIRS})

  if (IS_CLANG_COMPILER)
    target_link_libraries(${_exe} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES} OpenMP::OpenMP_CXX)
  elseif (IS_MICROSOFT_COMPILER)
    target_link_libraries(${_exe} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES})
  else()
    target_link_libraries(${_exe} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES} stdc++fs)
  endif()

  set_target_properties(${_exe} PROPERTIES
      OUTPUT_NAME_DEBUG   ${_exe_debug}
      OUTPUT_NAME_RELEASE ${_exe_release})

  if (IS_GNU_COMPILER)
    set_target_properties(${_exe} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -ldl -fopenmp")
  endif()
endfunction()

# ---------------------------------------------------------------------------
#  Все примеры из examples/ (если не режим "только библиотека").
# ---------------------------------------------------------------------------
function(globalizer_add_examples)
  if(GLOBALIZER_BUILD_AS_LIBRARY_ONLY)
    return()
  endif()
  file(GLOB main_list "${GLOBALIZER_EXAMPLES_SRC_DIR}/*.cpp")
  foreach(main_filename ${main_list})
    globalizer_add_example(${main_filename})
  endforeach()
endfunction()

# ---------------------------------------------------------------------------
#  Python-модуль PYDGlobalizer (pybind11).
# ---------------------------------------------------------------------------
function(globalizer_add_pybind_module)
  if(NOT GLOBALIZER_PYBIND_VERSION)
    return()
  endif()

  find_package(Python 3.11 REQUIRED COMPONENTS Interpreter Development)
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 CONFIG REQUIRED)

  pybind11_add_module(${GLOBALIZER_PYD_NAME}
      ${PROJECT_SOURCE_DIR}/PYGlobalizer/PYGlobalizer.cpp
      ${PROJECT_SOURCE_DIR}/PYGlobalizer/PYProblem.cpp)

  target_link_libraries(${GLOBALIZER_PYD_NAME} PRIVATE ${GLOBALIZER_LIB_NAME})

  target_include_directories(${GLOBALIZER_PYD_NAME} PRIVATE
      ${PROJECT_SOURCE_DIR}/globalizer/include
      ${PROJECT_SOURCE_DIR}/globalizer/problem/include
      ${PROJECT_SOURCE_DIR}/globalizer/solver/include
      ${Python_INCLUDE_DIRS})

  # Модуль кладём прямо в _bin (и для Debug, и для Release). Файлы
  # PYGlobalizer/*.py остаются в исходной папке под другим import-именем.
  set_target_properties(${GLOBALIZER_PYD_NAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}"
      LIBRARY_OUTPUT_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}")

  install(TARGETS ${GLOBALIZER_PYD_NAME} DESTINATION .)
endfunction()
