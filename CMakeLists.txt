set(GLOBALIZER_PROJECT_NAME Globalizer)
cmake_minimum_required(VERSION 3.31)

cmake_policy(SET CMP0057 NEW)#adds support for the new IN_LIST operator

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Available build types" FORCE)

set(GLOBALIZER_MPI_LIB_USER_PATH "" CACHE STRING "Path to MPI libraries")
set(GLOBALIZER_MPI_INCLUDE_USER_PATH "" CACHE STRING "Path to MPI includes")
set(GLOBALIZER_MPI "ms" CACHE STRING "Set MPI implementation (ms, mpich, intel)")
set(GLOBALIZER_OUTPUT_LEVEL "L0" CACHE STRING "Set level of output (L0, L1, L2)")
option(GLOBALIZER_USE_MP "Set OFF to disable MPFR (extended precision in evolvent will unavaliable)" OFF)
option(GLOBALIZER_USE_MPI "Set OFF to disable MPI.
  In this case a stub implementation of MPI will be used and globalizer won't support multi-process mode" OFF)
option(GLOBALIZER_USE_CUDA "Set ON to use CUDA" OFF)
option(GLOBALIZER_BUILD_TESTS "Set ON to build tests" OFF)
option(GLOBALIZER_CMAKE_VERBOSE "Set ON to show debug messages in CMake" ON)
option(GLOBALIZER_MAX_DIMENSION "Set MAX DIMENSION Globalizer" OFF)
option(GLOBALIZER_MAX_Number_Of_Function "Set MAX Number Of Function Globalizer" OFF)
option(GLOBALIZER_BUILD_PROBLEMS "Set ON to build problems" OFF)
option(GLOBALIZER_USE_DOXYGEN "Set ON to build documentation" OFF)
option(GLOBALIZER_BUILD_GCGEN "Set ON to build GCGen" OFF)
option(GLOBALIZER_BUILD_INTEGRATION_TESTS "Set ON to build integration tests" OFF)

option(GLOBALIZER_ONE_MPI_PROCESS_PER_NODE "ONE PROCESS PER NODE" OFF)
option(GLOBALIZER_PYTHON "Set ON to use Python" OFF)
option(GLOBALIZER_PYBIND_VERSION "Set ON to use Python interface" OFF)

option(GLOBALIZER_USE_ASAN "Set ON to build with AddressSanitizer (diagnostics)" OFF)

set(GLOBALIZER_DEBUG_SUFFIX   d)

set(GLOBALIZER_EXECUTABLE_NAME                 ${GLOBALIZER_PROJECT_NAME}${GLOBALIZER_EXECUTABLE_SUFFIX})
set(GLOBALIZER_EXECUTABLE_NAME_DEBUG           ${GLOBALIZER_EXECUTABLE_NAME}_${GLOBALIZER_DEBUG_SUFFIX})
set(GLOBALIZER_EXECUTABLE_NAME_RELEASE         ${GLOBALIZER_EXECUTABLE_NAME})
set(GLOBALIZER_LIB_NAME                        ${GLOBALIZER_EXECUTABLE_NAME}_l)
set(GLOBALIZER_LIB_NAME_RELEASE                ${GLOBALIZER_EXECUTABLE_NAME}_l)
set(GLOBALIZER_LIB_NAME_DEBUG                  ${GLOBALIZER_EXECUTABLE_NAME}_l_${GLOBALIZER_DEBUG_SUFFIX})
set(GLOBALIZER_PYD_NAME PYDGlobalizer)


set(CMAKE_CXX_STANDARD 17)
project(${GLOBALIZER_PROJECT_NAME})

option(GLOBALIZER_BUILD_AS_LIBRARY_ONLY "Собрать только библиотеку (пропустить примеры/.exe)" OFF)
option(BUILD_SHARED_LIBS "Собрать динамическую библиотеку (DLL/.so) вместо статической (.lib/.a)" OFF)

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

include(cmake/globalizer_utils.cmake)
include(cmake/globalizer_mpi.cmake)
include(cmake/globalizer_lib.cmake)
if (GLOBALIZER_USE_DOXYGEN)
  include(cmake/globalizer_doxygen.cmake)
endif()
GLOBALIZER_initialize()

if(${GLOBALIZER_MAX_DIMENSION})
	message("true " ${GLOBALIZER_MAX_DIMENSION})
	add_definitions(-DGLOBALIZER_MAX_DIMENSION=${GLOBALIZER_MAX_DIMENSION})
else()
	message("false" ${GLOBALIZER_MAX_DIMENSION})
	add_definitions(-DGLOBALIZER_MAX_DIMENSION=10)
endif()

if(${GLOBALIZER_MAX_Number_Of_Function})
	message("true " ${GLOBALIZER_MAX_Number_Of_Function})
	add_definitions(-DGLOBALIZER_MAX_Number_Of_Function=${GLOBALIZER_MAX_Number_Of_Function})
else()
	message("false" ${GLOBALIZER_MAX_Number_Of_Function})
	add_definitions(-DGLOBALIZER_MAX_Number_Of_Function=20)
endif()


if (IS_GNU_COMPILER)
  include(cmake/GMPConfig.cmake)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
	set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
  endif(CCACHE_FOUND)
elseif (IS_MICROSOFT_COMPILER)
  add_definitions(/EHsc /openmp -D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
  replace_compiler_flag("W3" "W4")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /wd4100")
  if (GLOBALIZER_TARGET_ARCH MATCHES "x86")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4100")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
else()
  set(OpenMP_C_FLAGS "-fopenmp")
  set(OpenMP_CXX_FLAGS "-fopenmp")
endif()

if (GLOBALIZER_USE_ASAN)
  message(STATUS "AddressSanitizer ENABLED")
  if (IS_MICROSOFT_COMPILER)
    # MSVC / clang-cl
    add_compile_options(/fsanitize=address /Zi)
    # ВАЖНО: с ASan под MSVC несовместим Edit&Continue и инкрементальная линковка
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  else()
    # GCC / Clang
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
  endif()
endif()


if(NOT (CMAKE_BUILD_TYPE IN_LIST CMAKE_CONFIGURATION_TYPES))
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
  message(STATUS "Unknown build type has been changed to " ${CMAKE_BUILD_TYPE})
endif()

if (GLOBALIZER_USE_DOXYGEN)
  init_doxygen()
endif()  
setup_mpi(GLOBALIZER_USE_MPI GLOBALIZER_MPI_LIB GLOBALIZER_MPI_INCLUDES)

foreach(includeFolder ${GLOBALIZER_MPI_INCLUDES})
  if (NOT EXISTS ${includeFolder})
    message(WARNING "MPI include directories is not found: " ${includeFolder})
  endif()
endforeach()

if (GLOBALIZER_MPI_LIB STREQUAL "")
  message(WARNING "MPI library is not found")
endif()




if (${GLOBALIZER_ONE_MPI_PROCESS_PER_NODE})
	add_definitions(-DONE_MPI_PROCESS_PER_NODE)
endif()

#add MPFR_GMP_MPIR
if (${GLOBALIZER_USE_MP})
	add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/mpfr_gmp_mpir)

	setup_mpir(GLOBALIZER_MPIR_LIB_DIR GLOBALIZER_MPIR_LIB)
	setup_gmp(GLOBALIZER_GMP_LIB_DIR GLOBALIZER_GMP_LIB)
	setup_mpfr(GLOBALIZER_MPFR_LIB_DIR GLOBALIZER_MPFR_LIB)
else()
  add_definitions(-DNO_MPREAL)
endif()

# set include directories for project
set(GLOBALIZER_EXAMPLES_SRC_DIR ${PROJECT_SOURCE_DIR}/examples)
set(GLOBALIZER_COMMON_SRC_DIR ${PROJECT_SOURCE_DIR}/globalizer)
set(GLOBALIZER_PARAMETERS_DIR ${GLOBALIZER_COMMON_SRC_DIR}/parameters)
set(GLOBALIZER_evolvent_DIR ${GLOBALIZER_COMMON_SRC_DIR}/evolvent)
set(GLOBALIZER_extended_DIR ${GLOBALIZER_COMMON_SRC_DIR}/extended)
set(GLOBALIZER_output_system_DIR ${GLOBALIZER_COMMON_SRC_DIR}/output_system)
set(GLOBALIZER_problem_DIR ${GLOBALIZER_COMMON_SRC_DIR}/problem)
set(GLOBALIZER_method_DIR ${GLOBALIZER_COMMON_SRC_DIR}/method)
set(GLOBALIZER_calculation_DIR ${GLOBALIZER_COMMON_SRC_DIR}/method/calculation)
set(GLOBALIZER_local_method_DIR ${GLOBALIZER_COMMON_SRC_DIR}/method/local_method)


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

file(GLOB ROOT_HEADERS ${GLOBALIZER_COMMON_SRC_DIR}/include/*.h ${GLOBALIZER_COMMON_SRC_DIR}/include/*.hpp )
file(GLOB evolvent_HEADERS ${GLOBALIZER_evolvent_DIR}/include/*.h ${GLOBALIZER_evolvent_DIR}/include/*.hpp)
file(GLOB extended_HEADERS ${GLOBALIZER_extended_DIR}/include/*.h ${GLOBALIZER_extended_DIR}/include/*.hpp)
file(GLOB output_system_HEADERS ${GLOBALIZER_output_system_DIR}/include/*.h ${GLOBALIZER_output_system_DIR}/include/*.hpp)
file(GLOB problem_HEADERS ${GLOBALIZER_problem_DIR}/include/*.h ${GLOBALIZER_problem_DIR}/include/*.hpp)
file(GLOB method_HEADERS ${GLOBALIZER_method_DIR}/include/*.h ${GLOBALIZER_method_DIR}/include/*.hpp)
file(GLOB calculation_HEADERS ${GLOBALIZER_calculation_DIR}/include/*.h ${GLOBALIZER_calculation_DIR}/include/*.hpp)
file(GLOB local_method_HEADERS ${GLOBALIZER_local_method_DIR}/include/*.h ${GLOBALIZER_local_method_DIR}/include/*.hpp)						
file(GLOB PARAMETERS_HEADERS ${GLOBALIZER_PARAMETERS_DIR}/include/*.h ${GLOBALIZER_PARAMETERS_DIR}/include/*.hpp)

file(GLOB ROOT_SOURCES ${GLOBALIZER_COMMON_SRC_DIR}/src/*.c ${GLOBALIZER_COMMON_SRC_DIR}/src/*.cpp)
file(GLOB evolvent_SOURCES ${GLOBALIZER_evolvent_DIR}/src/*.c ${GLOBALIZER_evolvent_DIR}/src/*.cpp)
file(GLOB extended_SOURCES ${GLOBALIZER_extended_DIR}/src/*.c ${GLOBALIZER_extended_DIR}/src/*.cpp)
file(GLOB output_system_SOURCES ${GLOBALIZER_output_system_DIR}/src/*.c ${GLOBALIZER_output_system_DIR}/src/*.cpp)
file(GLOB problem_SOURCES ${GLOBALIZER_problem_DIR}/src/*.c ${GLOBALIZER_problem_DIR}/src/*.cpp)
file(GLOB method_SOURCES ${GLOBALIZER_method_DIR}/src/*.c ${GLOBALIZER_method_DIR}/src/*.cpp)
file(GLOB calculation_SOURCES ${GLOBALIZER_calculation_DIR}/src/*.c ${GLOBALIZER_calculation_DIR}/src/*.cpp)
file(GLOB local_method_SOURCES ${GLOBALIZER_local_method_DIR}/src/*.c ${GLOBALIZER_local_method_DIR}/src/*.cpp)
file(GLOB PARAMETERS_SOURCES ${GLOBALIZER_PARAMETERS_DIR}/src/*.c ${GLOBALIZER_PARAMETERS_DIR}/src/*.cpp)


if (GLOBALIZER_USE_CUDA)
  find_package(CUDA REQUIRED)
endif()

set(GLOBALIZER_BENCHMARKS_Interface "")
#add problems
if(${GLOBALIZER_BUILD_PROBLEMS})
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/Problems)
	
  set(GLOBALIZER_BENCHMARKS_Interface ${PROJECT_SOURCE_DIR}/third_party/Problems/Interface)
  file(GLOB GLOBALIZER_BENCHMARKS_Interface_HEADERS ${GLOBALIZER_BENCHMARKS_Interface}/include/*.h ${GLOBALIZER_BENCHMARKS_Interface}/include/*.hpp )
  file(GLOB GLOBALIZER_BENCHMARKS_Interface_SOURCES ${GLOBALIZER_BENCHMARKS_Interface}/src/*.c ${GLOBALIZER_BENCHMARKS_Interface}/src/*.cpp)
  add_definitions(-D_GLOBALIZER_BENCHMARKS)		
  include_directories(${GLOBALIZER_BENCHMARKS_Interface}/include/)
endif()

if(${GLOBALIZER_BUILD_INTEGRATION_TESTS})
  add_definitions(-D_GLOBALIZER_BENCHMARKS)
 
  include_directories(${PROJECT_SOURCE_DIR}/third_party/Problems/Interface/include)
endif()

if(${GLOBALIZER_BUILD_GCGEN})
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/GCGen)

  include_directories(
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src
  )

  file(GLOB_RECURSE GCGEN_SOURCES
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src/*.cpp
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src/*.cpp
  )
  file(GLOB_RECURSE GCGEN_HEADERS
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/src/*.hpp
    ${PROJECT_SOURCE_DIR}/third_party/GCGen/sample_src/*.hpp
  )

  add_definitions(-D_GLOBALIZER_GCGEN)
endif()

# build main library and executable
set(ALL_SRC_FILES ${ROOT_HEADERS} ${PARAMETERS_HEADERS} ${ROOT_SOURCES}
${evolvent_HEADERS} 
${extended_HEADERS}
${output_system_HEADERS}
${problem_HEADERS}
${method_HEADERS}
${calculation_HEADERS}
${local_method_HEADERS}
${GLOBALIZER_BENCHMARKS_Interface_HEADERS}
${evolvent_SOURCES}
${extended_SOURCES}
${output_system_SOURCES}
${problem_SOURCES}
${method_SOURCES}
${calculation_SOURCES}
${local_method_SOURCES}
${PARAMETERS_SOURCES}
${GLOBALIZER_BENCHMARKS_Interface_SOURCES}
)
list(REMOVE_ITEM ALL_SRC_FILES ${GLOBALIZER_COMMON_SRC_DIR}/Main.cpp)

if (GLOBALIZER_PYTHON OR GLOBALIZER_PYBIND_VERSION)
  set(Python_FIND_DEBUG OFF)
  find_package(Python REQUIRED COMPONENTS Interpreter Development NumPy)

  message(STATUS "Python executable: ${Python_EXECUTABLE}")
  message(STATUS "Python includes: ${Python_INCLUDE_DIRS}")

  add_definitions(-DUSE_PYTHON)
else()
  message(STATUS "Python support disabled")
  set(Python_INCLUDE_DIRS "")
  set(Python_NumPy_INCLUDE_DIRS "")
  set(Python_LIBRARIES "")
endif()

set(_glib_type STATIC)
if(BUILD_SHARED_LIBS)
  set(_glib_type SHARED)
endif()

add_library(${GLOBALIZER_LIB_NAME} ${_glib_type} ${ALL_SRC_FILES} ${GCGEN_SOURCES} ${GCGEN_HEADERS})

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


if(NOT GLOBALIZER_BUILD_AS_LIBRARY_ONLY)
  file(GLOB main_list "${GLOBALIZER_EXAMPLES_SRC_DIR}/*.cpp")

  foreach(main_filename ${main_list})
    get_filename_component(sample ${main_filename} NAME_WE)
  
    if(GLOBALIZER_BUILD_PROBLEMS OR (sample STREQUAL "SimpleMain"))  
      set(GLOBALIZER_EXECUTABLE_NAME                 ${GLOBALIZER_PROJECT_NAME}${sample}${GLOBALIZER_EXECUTABLE_SUFFIX})
      set(GLOBALIZER_EXECUTABLE_NAME_DEBUG           ${GLOBALIZER_EXECUTABLE_NAME}_${GLOBALIZER_DEBUG_SUFFIX})
      set(GLOBALIZER_EXECUTABLE_NAME_RELEASE         ${GLOBALIZER_EXECUTABLE_NAME})
    
      message("Name: " ${GLOBALIZER_EXECUTABLE_NAME})
    
      add_executable(${GLOBALIZER_EXECUTABLE_NAME} ${main_filename})
	
      if (${GLOBALIZER_USE_MP})
        foreach(_mp_target libgmp mpir mpfr)
          if (TARGET ${_mp_target})
            add_dependencies(${GLOBALIZER_EXECUTABLE_NAME} ${_mp_target})
          endif()
        endforeach()
      endif()
    
      target_include_directories(${GLOBALIZER_EXECUTABLE_NAME} PRIVATE 
          ${GLOBALIZER_COMMON_SRC_DIR}
          ${Python_INCLUDE_DIRS} 
          ${Python_NumPy_INCLUDE_DIRS}
      )
     		
      if (${IS_CLANG_COMPILER})	
        target_link_libraries(${GLOBALIZER_EXECUTABLE_NAME} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES} OpenMP::OpenMP_CXX)
      elseif (${IS_MICROSOFT_COMPILER})
        target_link_libraries(${GLOBALIZER_EXECUTABLE_NAME} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES}) 
      else()
        target_link_libraries(${GLOBALIZER_EXECUTABLE_NAME} ${GLOBALIZER_LIB_NAME} ${Python_LIBRARIES} stdc++fs)  
      endif()
    
      set_target_properties(${GLOBALIZER_EXECUTABLE_NAME} PROPERTIES 
          OUTPUT_NAME_DEBUG ${GLOBALIZER_EXECUTABLE_NAME_DEBUG}
          OUTPUT_NAME_RELEASE ${GLOBALIZER_EXECUTABLE_NAME_RELEASE}
      )
    
      if (IS_GNU_COMPILER)
        set_target_properties(${GLOBALIZER_EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -ldl -fopenmp")
      endif()
    endif()
  endforeach()
endif() 

if(GLOBALIZER_PYBIND_VERSION)
    # Поиск Python 3.11+
    find_package(Python 3.11 REQUIRED COMPONENTS Interpreter Development)
    
    # Включить поиск Python для pybind11
    set(PYBIND11_FINDPYTHON ON)
    
    # Поиск pybind11
    find_package(pybind11 CONFIG REQUIRED)
    

	pybind11_add_module(${GLOBALIZER_PYD_NAME}
		${PROJECT_SOURCE_DIR}/PYGlobalizer/PYGlobalizer.cpp
		${PROJECT_SOURCE_DIR}/PYGlobalizer/PYProblem.cpp
	)

	target_link_libraries(${GLOBALIZER_PYD_NAME}
		PRIVATE
			${GLOBALIZER_LIB_NAME}
	)

	target_include_directories(${GLOBALIZER_PYD_NAME}
		PRIVATE
			${PROJECT_SOURCE_DIR}/globalizer/include
			${PROJECT_SOURCE_DIR}/globalizer/problem/include
			${PROJECT_SOURCE_DIR}/globalizer/solver/include
			${Python_INCLUDE_DIRS}
	)

	# The module is copied directly to _bin in both Debug and Release.
	# Do not copy PYGlobalizer/*.py here: those files are helper modules and
	# remain in the PYGlobalizer/ source directory under a different import name.
	set_target_properties(${GLOBALIZER_PYD_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}"
		LIBRARY_OUTPUT_DIRECTORY "${GLOBALIZER_OUTPUT_DIRECTORY}"
	)

	install(TARGETS ${GLOBALIZER_PYD_NAME} DESTINATION .)
endif()



#add_executable(${GLOBALIZER_PROJECT_NAME} ${GLOBALIZER_EXAMPLES_SRC_DIR}/Main.cpp)



												  
include_directories(${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS})



if(GLOBALIZER_USE_MP)
    set(GLOBALIZER_MP_LINK
        $<BUILD_INTERFACE:${GLOBALIZER_MPFR_LIB}>
        $<BUILD_INTERFACE:${GLOBALIZER_MPIR_LIB}>
        ${GLOBALIZER_GMP_LIB}
    )
else()
    set(GLOBALIZER_MP_LINK "")
endif()

if(IS_CLANG_COMPILER)
    target_link_libraries(${GLOBALIZER_LIB_NAME} PRIVATE
        ${GLOBALIZER_MPI_LIB}
        ${GLOBALIZER_MP_LINK}
        ${DISLIN_LIB}
        OpenMP::OpenMP_CXX
    )
else()
    target_link_libraries(${GLOBALIZER_LIB_NAME} PRIVATE
        ${GLOBALIZER_MPI_LIB}
        ${GLOBALIZER_MP_LINK}
        ${DISLIN_LIB}
    )
endif()

# ------------------------------------------------------------------------------
# Гарантируем правильный порядок сборки MP-библиотек (gmp/mpfr/mpir).
#
# Проблема: libgmp/mpfr/mpir линкуются к Globalizer_l "по имени" (gmp/mpfr/mpir)
# через link_directories, а не как CMake-цели. Поэтому CMake НЕ видит зависимости
# и MSBuild может собрать Globalizer_l РАНЬШЕ gmp -> "cannot open gmp.lib"
# на первой сборке. add_dependencies заставляет собрать MP-цели ПЕРЕД Globalizer_l.
#
# ВАЖНО: имя цели GMP — libgmp (OUTPUT_NAME = gmp), цели MPFR/MPIR — mpfr/mpir.
# ------------------------------------------------------------------------------
if (${GLOBALIZER_USE_MP})
  set(GLOBALIZER_MP_BUILD_TARGETS "")
  foreach(_mp_target libgmp mpir mpfr)
    if (TARGET ${_mp_target})
      list(APPEND GLOBALIZER_MP_BUILD_TARGETS ${_mp_target})
    else()
      message(WARNING "MP dependency target '${_mp_target}' not found "
                      "(check third_party/mpfr_gmp_mpir/*/CMakeLists.txt).")
    endif()
  endforeach()

  if (GLOBALIZER_MP_BUILD_TARGETS)
    add_dependencies(${GLOBALIZER_LIB_NAME} ${GLOBALIZER_MP_BUILD_TARGETS})
    message(STATUS "MP build order enforced: ${GLOBALIZER_LIB_NAME} depends on "
                   "${GLOBALIZER_MP_BUILD_TARGETS}")
  endif()
endif()

message(STATUS "MPI libraries: ${GLOBALIZER_MPI_LIB}")

set_target_properties(${GLOBALIZER_LIB_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${GLOBALIZER_LIB_NAME_DEBUG})
set_target_properties(${GLOBALIZER_LIB_NAME} PROPERTIES OUTPUT_NAME_RELEASE ${GLOBALIZER_LIB_NAME_RELEASE})



message("Find OpenMP") 
find_package(OpenMP)
if (OPENMP_FOUND)
  message("OpenMP found")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
  message("OpenMP not found")
endif()

# add linker flags for GCC
if (IS_GNU_COMPILER)
  target_compile_options(${GLOBALIZER_LIB_NAME} INTERFACE -march=native -fopenmp)
endif()

# add integration tests
if(${GLOBALIZER_BUILD_INTEGRATION_TESTS})
  add_subdirectory(${PROJECT_SOURCE_DIR}/integration_tests)
endif()

#add tests
if(${GLOBALIZER_BUILD_TESTS})
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/gtest)
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()




if(${GLOBALIZER_CMAKE_VERBOSE})
  print_build_config()
endif()


# create source groups for VS IDE projects
if (IS_MICROSOFT_COMPILER)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  
  set(3RD_PARTY_PROJECT "")
  
  if(GLOBALIZER_BUILD_TESTS)
    list(APPEND 3RD_PARTY_PROJECT "gtest" "gmock" "gmock_main" "gtest_main")	
  endif()
  
  if(NOT GLOBALIZER_USE_MPI)
      list(APPEND 3RD_PARTY_PROJECT "mpi_stub")	
  endif()
  
  if(GLOBALIZER_BUILD_GCGEN)
    list(APPEND 3RD_PARTY_PROJECT "builder_sample")	
  endif()
  
  if(3RD_PARTY_PROJECT)
    set_property(TARGET ${3RD_PARTY_PROJECT} PROPERTY FOLDER "3rd-party")
  endif()
  
  set(MultiplePrecision "")
  
  if(${GLOBALIZER_USE_MP})
	list (APPEND MultiplePrecision "bootstrap" "gen-bases" "gen-fac" "gen-fac-table" "gen-fib" "gen-fib-table"
	"gen-jacobitab" "gen-mp-bases" "gen-mpn-jacobitab" "gen-mpn-perfsqr" "gen-psqr" "gen-trialdivtab" 
	"gen-trialdivtab-h" "libgmp" "libmpf" "libmpn" "libmpq" "libmpz" "libprintf" "librandom" "libscanf"
	"mpfr" "mpir")
  endif()
  
  if(MultiplePrecision)
  	set_property(TARGET ${MultiplePrecision} PROPERTY FOLDER "multiple precision")
  endif()

  source_group("Header Files" FILES ${ROOT_HEADERS})
  source_group("Header Files\\Parameters" FILES ${PARAMETERS_HEADERS})
  source_group("Header Files\\evolvent" FILES ${evolvent_HEADERS}) 
  source_group("Header Files\\extended" FILES ${extended_HEADERS})
  source_group("Header Files\\output_system" FILES ${output_system_HEADERS})
  source_group("Header Files\\problem" FILES ${problem_HEADERS} ${GLOBALIZER_BENCHMARKS_Interface_HEADERS})
  source_group("Header Files\\method" FILES ${method_HEADERS})
  source_group("Header Files\\calculation" FILES ${calculation_HEADERS})
  source_group("Header Files\\local_method" FILES ${local_method_HEADERS})
  source_group("Header Files\\gcgen" FILES ${GCGEN_HEADERS})

  source_group("Source Files" FILES ${ROOT_SOURCES})
  source_group("Source Files\\evolvent" FILES ${evolvent_SOURCES})
  source_group("Source Files\\extended" FILES ${extended_SOURCES})
  source_group("Source Files\\output_system" FILES ${output_system_SOURCES})
  source_group("Source Files\\problem" FILES ${problem_SOURCES} ${GLOBALIZER_BENCHMARKS_Interface_SOURCES})
  source_group("Source Files\\method" FILES ${method_SOURCES})
  source_group("Source Files\\calculation" FILES ${calculation_SOURCES})
  source_group("Source Files\\local_method" FILES ${local_method_SOURCES})
  source_group("Source Files\\parameters" FILES ${PARAMETERS_SOURCES})
  source_group("Source Files\\gcgen" FILES ${GCGEN_SOURCES})
  
  
  
endif()


# === УСТАНОВКА И ЭКСПОРТ ДЛЯ ВНЕШНИХ ПРОЕКТОВ ===
include(GNUInstallDirs)

install(DIRECTORY
    ${GLOBALIZER_COMMON_SRC_DIR}/include/
    ${GLOBALIZER_PARAMETERS_DIR}/include/
    ${GLOBALIZER_evolvent_DIR}/include/
    ${GLOBALIZER_extended_DIR}/include/
    ${GLOBALIZER_output_system_DIR}/include/
    ${GLOBALIZER_problem_DIR}/include/
    ${GLOBALIZER_method_DIR}/include/
    ${GLOBALIZER_calculation_DIR}/include/
    ${GLOBALIZER_local_method_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Если MPI выключен, копируем заглушку mpi.h
if(NOT GLOBALIZER_USE_MPI)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/lib/mpi_stub/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h")
endif()

# Если mpi_stub существует, делаем его пути "экспорт-безопасными"
if(TARGET mpi_stub)
  set_target_properties(mpi_stub PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/mpi_stub/include>;$<INSTALL_INTERFACE:include>"
  )
endif()

# Цели, которые реально существуют в текущей конфигурации
set(GLOBALIZER_INSTALL_TARGETS
    ${GLOBALIZER_LIB_NAME}
)

if(TARGET mpi_stub)
    list(APPEND GLOBALIZER_INSTALL_TARGETS mpi_stub)
endif()

install(TARGETS ${GLOBALIZER_INSTALL_TARGETS}
        EXPORT GlobalizerTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT GlobalizerTargets
        FILE GlobalizerTargets.cmake
        NAMESPACE Globalizer::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Globalizer)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GlobalizerConfig.cmake"
     "include(\"\${CMAKE_CURRENT_LIST_DIR}/GlobalizerTargets.cmake\")\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GlobalizerConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Globalizer)