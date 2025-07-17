set(EXAMIN_PROJECT_NAME Globalizer)
cmake_minimum_required(VERSION 4.0)

cmake_policy(SET CMP0057 NEW)#adds support for the new IN_LIST operator

set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)

set(EXAMIN_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/_bin CACHE STRING "Output directory")
set(EXAMIN_MPI_LIB_USER_PATH "" CACHE STRING "Path to MPI libraries")
set(EXAMIN_MPI_INCLUDE_USER_PATH "" CACHE STRING "Path to MPI includes")
set(EXAMIN_MPI "ms" CACHE STRING "Set MPI implementation (ms, mpich, intel)")
set(EXAMIN_OUTPUT_LEVEL "L0" CACHE STRING "Set level of output (L0, L1, L2)")
option(EXAMIN_USE_MP "Set OFF to disable MPFR (extended precision in evolvent will unavaliable)" ON)
option(EXAMIN_USE_MPI "Set OFF to disable MPI.
  In this case a stub implementation of MPI will be used and globalizer won't support multi-process mode" ON)
option(EXAMIN_USE_CUDA "Set ON to use CUDA" OFF)
option(EXAMIN_BUILD_TESTS "Set ON to build tests" ON)
option(EXAMIN_CMAKE_VERBOSE "Set ON to show debug messages in CMake" OFF)
option(EXAMIN_MAX_DIMENSION "Set MAX DIMENSION Globalizer" OFF)



option(EXAMIN_ONE_MPI_PROCESS_PER_NODE "ONE PROCESS PER NODE" OFF)

option(EXAMIN_PYTHON "Set ON to use Python" OFF)

set(EXAMIN_DEBUG_SUFFIX   d)

set(EXAMIN_EXECUTABLE_NAME                 ${EXAMIN_PROJECT_NAME}${EXAMIN_EXECUTABLE_SUFFIX})
set(EXAMIN_EXECUTABLE_NAME_DEBUG           ${EXAMIN_EXECUTABLE_NAME}_${EXAMIN_DEBUG_SUFFIX})
set(EXAMIN_EXECUTABLE_NAME_RELEASE         ${EXAMIN_EXECUTABLE_NAME})
set(EXAMIN_LIB_NAME                        ${EXAMIN_EXECUTABLE_NAME}_l)
set(EXAMIN_LIB_NAME_RELEASE                ${EXAMIN_EXECUTABLE_NAME}_l)
set(EXAMIN_LIB_NAME_DEBUG                  ${EXAMIN_EXECUTABLE_NAME}_l_${EXAMIN_DEBUG_SUFFIX})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${EXAMIN_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXAMIN_OUTPUT_DIRECTORY})


if(${EXAMIN_MAX_DIMENSION})
	message("true " ${EXAMIN_MAX_DIMENSION})
	add_definitions(-DEXAMIN_MAX_DIMENSION=${EXAMIN_MAX_DIMENSION})
else()
	message("false" ${EXAMIN_MAX_DIMENSION})
	add_definitions(-DEXAMIN_MAX_DIMENSION=10)
endif()


set(CMAKE_CXX_STANDARD 17)
project(${EXAMIN_PROJECT_NAME})
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
  if (EXAMIN_TARGET_ARCH MATCHES "x86")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4100")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
elseif (IS_INTEL_COMPILER)
  # TODO: add options for Intel compiler
endif()


include(cmake/examin_utils.cmake)
include(cmake/examin_mpi.cmake)
#include(cmake/examin_lib.cmake)
include(cmake/examin_doxygen.cmake)

examin_initialize()

if(NOT (CMAKE_BUILD_TYPE IN_LIST CMAKE_CONFIGURATION_TYPES))
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
  message(STATUS "Unknown build type has been changed to " ${CMAKE_BUILD_TYPE})
endif()


init_doxygen()
setup_mpi(EXAMIN_USE_MPI EXAMIN_MPI_LIB EXAMIN_MPI_INCLUDES)

foreach(includeFolder ${EXAMIN_MPI_INCLUDES})
  if (NOT EXISTS ${includeFolder})
    message(WARNING "MPI include directories is not found: " ${includeFolder})
  endif()
endforeach()

if (EXAMIN_MPI_LIB STREQUAL "")
  message(WARNING "MPI library is not found")
endif()




if (${EXAMIN_ONE_MPI_PROCESS_PER_NODE})
	add_definitions(-DONE_MPI_PROCESS_PER_NODE)
endif()



if (${EXAMIN_USE_MP})
  setup_mpir(EXAMIN_MPIR_LIB_DIR EXAMIN_MPIR_INCLUDES EXAMIN_MPIR_LIB)
  setup_mpfr(EXAMIN_MPFR_LIB_DIR EXAMIN_MPFR_INCLUDES EXAMIN_MPFR_LIB)
  setup_mpreal(EXAMIN_MPREAL_INCLUDES)
else()
  add_definitions(-DNO_MPREAL)
endif()

# set include directories for project
set(EXAMIN_EXAMPLES_SRC_DIR ${PROJECT_SOURCE_DIR}/examples)
set(EXAMIN_COMMON_SRC_DIR ${PROJECT_SOURCE_DIR}/globalizer)
set(EXAMIN_PARAMETERS_DIR ${EXAMIN_COMMON_SRC_DIR}/parameters)
set(EXAMIN_evolvent_DIR ${EXAMIN_COMMON_SRC_DIR}/evolvent)
set(EXAMIN_extended_DIR ${EXAMIN_COMMON_SRC_DIR}/extended)
set(EXAMIN_output_system_DIR ${EXAMIN_COMMON_SRC_DIR}/output_system)
set(EXAMIN_problem_DIR ${EXAMIN_COMMON_SRC_DIR}/problem)
set(EXAMIN_method_DIR ${EXAMIN_COMMON_SRC_DIR}/method)
set(EXAMIN_calculation_DIR ${EXAMIN_COMMON_SRC_DIR}/method/calculation)
set(EXAMIN_local_method_DIR ${EXAMIN_COMMON_SRC_DIR}/method/local_method)


include_directories(
  ${EXAMIN_MPI_INCLUDES}
  ${EXAMIN_MPIR_INCLUDES}
  ${EXAMIN_MPFR_INCLUDES}
  ${EXAMIN_MPREAL_INCLUDES}
  
  ${EXAMIN_PARAMETERS_DIR}/include
  ${EXAMIN_evolvent_DIR}/include
  ${EXAMIN_extended_DIR}/include
  ${EXAMIN_output_system_DIR}/include
  ${EXAMIN_problem_DIR}/include
  ${EXAMIN_method_DIR}/include
  ${EXAMIN_calculation_DIR}/include
  ${EXAMIN_local_method_DIR}/include
)

link_directories(
  ${EXAMIN_MPIR_LIB_DIR}
  ${EXAMIN_MPFR_LIB_DIR}
)

file(GLOB ROOT_HEADERS ${EXAMIN_COMMON_SRC_DIR}/include/*.h ${EXAMIN_COMMON_SRC_DIR}/include/*.hpp )
file(GLOB evolvent_HEADERS ${EXAMIN_evolvent_DIR}/include/*.h ${EXAMIN_evolvent_DIR}/include/*.hpp)
file(GLOB extended_HEADERS ${EXAMIN_extended_DIR}/include/*.h ${EXAMIN_extended_DIR}/include/*.hpp)
file(GLOB output_system_HEADERS ${EXAMIN_output_system_DIR}/include/*.h ${EXAMIN_output_system_DIR}/include/*.hpp)
file(GLOB problem_HEADERS ${EXAMIN_problem_DIR}/include/*.h ${EXAMIN_problem_DIR}/include/*.hpp)
file(GLOB method_HEADERS ${EXAMIN_method_DIR}/include/*.h ${EXAMIN_method_DIR}/include/*.hpp)
file(GLOB calculation_HEADERS ${EXAMIN_calculation_DIR}/include/*.h ${EXAMIN_calculation_DIR}/include/*.hpp)
file(GLOB local_method_HEADERS ${EXAMIN_local_method_DIR}/include/*.h ${EXAMIN_local_method_DIR}/include/*.hpp)						
file(GLOB PARAMETERS_HEADERS ${EXAMIN_PARAMETERS_DIR}/include/*.h ${EXAMIN_PARAMETERS_DIR}/include/*.hpp)

file(GLOB ROOT_SOURCES ${EXAMIN_COMMON_SRC_DIR}/src/*.c ${EXAMIN_COMMON_SRC_DIR}/src/*.cpp)
file(GLOB evolvent_SOURCES ${EXAMIN_evolvent_DIR}/src/*.c ${EXAMIN_evolvent_DIR}/src/*.cpp)
file(GLOB extended_SOURCES ${EXAMIN_extended_DIR}/src/*.c ${EXAMIN_extended_DIR}/src/*.cpp)
file(GLOB output_system_SOURCES ${EXAMIN_output_system_DIR}/src/*.c ${EXAMIN_output_system_DIR}/src/*.cpp)
file(GLOB problem_SOURCES ${EXAMIN_problem_DIR}/src/*.c ${EXAMIN_problem_DIR}/src/*.cpp)
file(GLOB method_SOURCES ${EXAMIN_method_DIR}/src/*.c ${EXAMIN_method_DIR}/src/*.cpp)
file(GLOB calculation_SOURCES ${EXAMIN_calculation_DIR}/src/*.c ${EXAMIN_calculation_DIR}/src/*.cpp)
file(GLOB local_method_SOURCES ${EXAMIN_local_method_DIR}/src/*.c ${EXAMIN_local_method_DIR}/src/*.cpp)


if (EXAMIN_USE_CUDA)
  find_package(CUDA REQUIRED)
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
${evolvent_SOURCES}
${extended_SOURCES}
${output_system_SOURCES}
${problem_SOURCES}
${method_SOURCES}
${calculation_SOURCES}
${local_method_SOURCES}
)
list(REMOVE_ITEM ALL_SRC_FILES ${EXAMIN_COMMON_SRC_DIR}/main.cpp)

add_library(${EXAMIN_LIB_NAME} STATIC ${ALL_SRC_FILES})
add_executable(${EXAMIN_PROJECT_NAME} ${EXAMIN_EXAMPLES_SRC_DIR}/main.cpp)


if (${EXAMIN_PYTHON})
  find_package (Python COMPONENTS Interpreter Development NumPy)

#set(EXTRA_DEFINES "")
#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  #set(EXTRA_DEFINES PYLIBNAME="${Python_LIBRARIES}")
  message("Python pname ${Python_LIBRARIES}")
  message("Python end")
  #message(${PYLIBNAME})
  add_definitions(-DUSE_PYTHON)
#endif()

  if (Python_Development_FOUND)
    message("Python Development FOUND")
  endif()
  
  if (Python_NumPy_FOUND)
    message("Python NumPy FOUND")
  endif()

else()
  message("Python not use")
  set(Python_INCLUDE_DIRS "")
  set(Python_NumPy_INCLUDE_DIRS "")
  set(Python_LIBRARIES "")
  
endif()

target_include_directories(${EXAMIN_PROJECT_NAME} PRIVATE ${EXAMIN_COMMON_SRC_DIR}
												  PRIVATE ${Python_INCLUDE_DIRS} 
												  PRIVATE ${Python_NumPy_INCLUDE_DIRS})
												  
include_directories(${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS})
												  
target_link_libraries(${EXAMIN_PROJECT_NAME} ${EXAMIN_LIB_NAME} ${Python_LIBRARIES}) #/opt/intel/impi/2017.1.132/intel64/lib/libmpi.so)


target_link_libraries(${EXAMIN_LIB_NAME}
  ${EXAMIN_MPI_LIB}
  ${EXAMIN_MPFR_LIB}
  ${EXAMIN_MPIR_LIB}
  ${DISLIN_LIB}
)



message("MPI lib: " ${EXAMIN_LIB_NAME})

set_target_properties(${EXAMIN_LIB_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${EXAMIN_LIB_NAME_DEBUG})
set_target_properties(${EXAMIN_LIB_NAME} PROPERTIES OUTPUT_NAME_RELEASE ${EXAMIN_LIB_NAME_RELEASE})

set_target_properties(${EXAMIN_PROJECT_NAME} PROPERTIES OUTPUT_NAME_DEBUG ${EXAMIN_EXECUTABLE_NAME_DEBUG})
set_target_properties(${EXAMIN_PROJECT_NAME} PROPERTIES OUTPUT_NAME_RELEASE ${EXAMIN_EXECUTABLE_NAME_RELEASE})

# add linker flags for GCC
if (IS_GNU_COMPILER)
  target_compile_options(${EXAMIN_LIB_NAME} INTERFACE -march=native -fopenmp)
  set_target_properties(${EXAMIN_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -ldl -fopenmp")

endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

#add tests
if(${EXAMIN_BUILD_TESTS})
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gtest)
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()




if(${EXAMIN_CMAKE_VERBOSE})
  print_build_config()
endif()


# create source groups for VS IDE projects
if (IS_MICROSOFT_COMPILER)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  #set_property(TARGET pugixml PROPERTY FOLDER "3rd-party")
  #if(${EXAMIN_BUILD_TESTS})
  #  set_property(TARGET gtests PROPERTY FOLDER "3rd-party")
  #endif()


  source_group("Header Files" FILES ${ROOT_HEADERS})
  source_group("Header Files\\Parameters" FILES ${PARAMETERS_HEADERS})
  source_group("Header Files\\evolvent" FILES ${evolvent_HEADERS}) 
  source_group("Header Files\\extended" FILES ${extended_HEADERS})
  source_group("Header Files\\output_system" FILES ${output_system_HEADERS})
  source_group("Header Files\\problem" FILES ${problem_HEADERS})
  source_group("Header Files\\method" FILES ${method_HEADERS})
  source_group("Header Files\\calculation" FILES ${calculation_HEADERS})
  source_group("Header Files\\local_method" FILES ${local_method_HEADERS})

  source_group("Source Files" FILES ${ROOT_SOURCES})
  source_group("Source Files\\evolvent" FILES ${evolvent_SOURCES})
  source_group("Source Files\\extended" FILES ${extended_SOURCES})
  source_group("Source Files\\output_system" FILES ${output_system_SOURCES})
  source_group("Source Files\\problem" FILES ${problem_SOURCES})
  source_group("Source Files\\method" FILES ${method_SOURCES})
  source_group("Source Files\\calculation" FILES ${calculation_SOURCES})
  source_group("Source Files\\local_method" FILES ${local_method_SOURCES})
  
  
  
  
endif()
