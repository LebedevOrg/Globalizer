# ============================================================================
#  globalizer_dependencies.cmake
#  ---------------------------------------------------------------------------
#  Внешние и внутренние зависимости: MPI, MPFR/GMP/MPIR, CUDA, Python, OpenMP,
#  а также подпроекты problems / GCGen / integration-tests include-пути.
#
#  Каждая функция изменяет переменные в родительском (directory) scope через
#  PARENT_SCOPE или работает с кэшем — поведение совпадает с прежним монолитом.
# ============================================================================

# ---------------------------------------------------------------------------
#  MPI: настройка + предупреждения о ненайденных путях.
#  Результат: GLOBALIZER_MPI_LIB, GLOBALIZER_MPI_INCLUDES (в PARENT_SCOPE).
# ---------------------------------------------------------------------------
function(globalizer_configure_mpi)
  setup_mpi(GLOBALIZER_USE_MPI _mpi_lib _mpi_inc)

  foreach(includeFolder ${_mpi_inc})
    if (NOT EXISTS ${includeFolder})
      message(WARNING "MPI include directories is not found: " ${includeFolder})
    endif()
  endforeach()

  if (_mpi_lib STREQUAL "")
    message(WARNING "MPI library is not found")
  endif()

  set(GLOBALIZER_MPI_LIB      "${_mpi_lib}" PARENT_SCOPE)
  set(GLOBALIZER_MPI_INCLUDES "${_mpi_inc}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
#  Множественная точность (MPFR/GMP/MPIR). Добавляет ABI-макросы:
#  ONE_MPI_PROCESS_PER_NODE и NO_MPREAL. Принимает имя переменной со списком
#  ABI-определений и дополняет её в PARENT_SCOPE.
#  Также заполняет GLOBALIZER_MP*_LIB / *_LIB_DIR при включённом MP.
# ---------------------------------------------------------------------------
function(globalizer_configure_mp abi_var)
  set(_abi "${${abi_var}}")

  if (GLOBALIZER_ONE_MPI_PROCESS_PER_NODE)
    list(APPEND _abi ONE_MPI_PROCESS_PER_NODE)
  endif()

  if (GLOBALIZER_USE_MP)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/mpfr_gmp_mpir)
    setup_mpir(_mpir_dir _mpir_lib)
    setup_gmp(_gmp_dir  _gmp_lib)
    setup_mpfr(_mpfr_dir _mpfr_lib)

    set(GLOBALIZER_MPIR_LIB_DIR "${_mpir_dir}" PARENT_SCOPE)
    set(GLOBALIZER_MPIR_LIB     "${_mpir_lib}" PARENT_SCOPE)
    set(GLOBALIZER_GMP_LIB_DIR  "${_gmp_dir}"  PARENT_SCOPE)
    set(GLOBALIZER_GMP_LIB      "${_gmp_lib}"  PARENT_SCOPE)
    set(GLOBALIZER_MPFR_LIB_DIR "${_mpfr_dir}" PARENT_SCOPE)
    set(GLOBALIZER_MPFR_LIB     "${_mpfr_lib}" PARENT_SCOPE)
  else()
    # sizeof(Extended) = 8 вместо 16: ABI
    list(APPEND _abi NO_MPREAL)
  endif()

  set(${abi_var} "${_abi}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
#  CUDA (опционально).
# ---------------------------------------------------------------------------
function(globalizer_configure_cuda)
  if (GLOBALIZER_USE_CUDA)
    find_package(CUDA REQUIRED)
  endif()
endfunction()

# ---------------------------------------------------------------------------
#  Подпроект benchmark-задач (third_party/Problems). Заполняет:
#    GLOBALIZER_BENCHMARKS_Interface,
#    GLOBALIZER_BENCHMARKS_Interface_HEADERS / _SOURCES,
#  добавляет include-путь и ABI-макрос _GLOBALIZER_BENCHMARKS.
# ---------------------------------------------------------------------------
function(globalizer_configure_problems abi_var)
  set(_abi "${${abi_var}}")
  set(_iface "")

  if (GLOBALIZER_BUILD_PROBLEMS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/Problems)
    set(_iface ${PROJECT_SOURCE_DIR}/third_party/Problems/Interface)

    file(GLOB _iface_headers ${_iface}/include/*.h ${_iface}/include/*.hpp)
    file(GLOB _iface_sources ${_iface}/src/*.c     ${_iface}/src/*.cpp)

    list(APPEND _abi _GLOBALIZER_BENCHMARKS)
    include_directories(${_iface}/include/)

    set(GLOBALIZER_BENCHMARKS_Interface_HEADERS "${_iface_headers}" PARENT_SCOPE)
    set(GLOBALIZER_BENCHMARKS_Interface_SOURCES "${_iface_sources}" PARENT_SCOPE)
  endif()

  if (GLOBALIZER_BUILD_INTEGRATION_TESTS)
    list(APPEND _abi _GLOBALIZER_BENCHMARKS)
    include_directories(${PROJECT_SOURCE_DIR}/third_party/Problems/Interface/include)
  endif()

  set(GLOBALIZER_BENCHMARKS_Interface "${_iface}" PARENT_SCOPE)
  set(${abi_var} "${_abi}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
#  Генератор задач GCGen (third_party/GCGen). Заполняет GCGEN_SOURCES /
#  GCGEN_HEADERS и добавляет определение _GLOBALIZER_GCGEN.
# ---------------------------------------------------------------------------
function(globalizer_configure_gcgen)
  if (NOT GLOBALIZER_BUILD_GCGEN)
    return()
  endif()

  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/GCGen)
  include_directories(
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src
  )

  file(GLOB_RECURSE _gcgen_sources
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src/*.cpp
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src/*.cpp)
  file(GLOB_RECURSE _gcgen_headers
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src/*.hpp
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src/*.hpp)

  add_definitions(-D_GLOBALIZER_GCGEN)

  set(GCGEN_SOURCES "${_gcgen_sources}" PARENT_SCOPE)
  set(GCGEN_HEADERS "${_gcgen_headers}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
#  Python (интерпретатор + Development + NumPy). Добавляет ABI-макрос
#  USE_PYTHON. Переменные Python_* выставляет в PARENT_SCOPE (пустые, если
#  Python выключен) — потребители используют их далее без изменений.
# ---------------------------------------------------------------------------
function(globalizer_configure_python abi_var)
  set(_abi "${${abi_var}}")

  if (GLOBALIZER_PYTHON OR GLOBALIZER_PYBIND_VERSION)
    set(Python_FIND_DEBUG OFF)
    find_package(Python REQUIRED COMPONENTS Interpreter Development NumPy)
    message(STATUS "Python executable: ${Python_EXECUTABLE}")
    message(STATUS "Python includes: ${Python_INCLUDE_DIRS}")
    list(APPEND _abi USE_PYTHON)

    set(Python_INCLUDE_DIRS       "${Python_INCLUDE_DIRS}"       PARENT_SCOPE)
    set(Python_NumPy_INCLUDE_DIRS "${Python_NumPy_INCLUDE_DIRS}" PARENT_SCOPE)
    set(Python_LIBRARIES          "${Python_LIBRARIES}"          PARENT_SCOPE)
    set(Python_EXECUTABLE         "${Python_EXECUTABLE}"         PARENT_SCOPE)
  else()
    message(STATUS "Python support disabled")
    set(Python_INCLUDE_DIRS       "" PARENT_SCOPE)
    set(Python_NumPy_INCLUDE_DIRS "" PARENT_SCOPE)
    set(Python_LIBRARIES          "" PARENT_SCOPE)
  endif()

  set(${abi_var} "${_abi}" PARENT_SCOPE)
endfunction()
