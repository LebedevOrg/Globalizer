set(EXAMIN_PROJECT_NAME globalizer)
set(EXAMIN_MCO_PROJECT_NAME globalizer_mco)
cmake_minimum_required(VERSION 4.0)

option(EXAMIN_USE_OneAPI "Set ON to use OneAPI" OFF)

if(${EXAMIN_USE_OneAPI})
	cmake_minimum_required(VERSION 4.0)
    message("EXAMIN_USE_OneAPI")
	project(${EXAMIN_PROJECT_NAME} CXX)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
else()
	set(CMAKE_CXX_STANDARD 17)
	cmake_minimum_required(VERSION 4.0)
	message("EXAMIN VS")
	project(${EXAMIN_PROJECT_NAME})
endif()


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
option(EXAMIN_BUILD_PROBLEMS "Set OFF for not build problems" ON)
option(EXAMIN_USE_CUDA "Set ON to use CUDA" OFF)
option(EXAMIN_BUILD_PLOTTER "Set ON to build plotter" ON)
option(EXAMIN_BUILD_TESTS "Set ON to build tests" ON)
option(EXAMIN_CMAKE_VERBOSE "Set ON to show debug messages in CMake" OFF)
option(EXAMIN_USE_OPEN_CV "Set ON to use OpenCV" OFF)
set(EXAMIN_OPEN_CV_PATH "" CACHE STRING "Path to OpenCV build")
option(EXAMIN_BUILD_MCO "Set ON to build globalizer_mco" OFF)
option(EXAMIN_MAX_DIMENSION "Set MAX DIMENSION Globalizer" OFF)
option(EXAMIN_USE_BOOST "Set ON to use BOOST" OFF)

option(EXAMIN_LITE "Set ON to use shortened version" OFF)


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

if(${EXAMIN_USE_OneAPI})
	if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR ${CMAKE_CXX_COMPILER_ID} STREQUAL IntelDPCPP)
		if (UNIX)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsycl $ENV{DPC_CXX_FLAGS}")
			set(CMAKE_SHARED_LINKER_FLAGS  $ENV{DPC_LDD_FLAGS})
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /Zi")
		endif()
	endif()
	
	if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	  add_definitions(-fdiagnostics-color=always -D_GLIBCXX_USE_CXX11_ABI=0)
	endif()
else()
	set(CMAKE_CXX_STANDARD 11)
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

endif()
include(cmake/examin_utils.cmake)
include(cmake/examin_mpi.cmake)
include(cmake/examin_lib.cmake)
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

if(${EXAMIN_BUILD_PLOTTER})
  setup_dislin(DISLIN_LIB DISLIN_INCLUDE)
endif()



if (${EXAMIN_ONE_MPI_PROCESS_PER_NODE})
	add_definitions(-DONE_MPI_PROCESS_PER_NODE)
endif()

if (${EXAMIN_USE_OPEN_CV})
  #setup_OpenCV(EXAMIN_OPEN_CV_LIB_DIR EXAMIN_OPEN_CV_INCLUDES EXAMIN_OPEN_CV_LIB)
  
  find_package( OpenCV REQUIRED )

  set(EXAMIN_OPEN_CV_LIB_DIR "")
  set(EXAMIN_OPEN_CV_LIB ${OpenCV_LIBS})
  set(EXAMIN_OPEN_CV_INCLUDES ${OpenCV_INCLUDE_DIRS} )
  
  add_definitions(-DUSE_OpenCV)
else()
  set(EXAMIN_OPEN_CV_INCLUDES "")
  set(EXAMIN_OPEN_CV_LIB_DIR "")
  set(EXAMIN_OPEN_CV_LIB "")
endif()

if (${EXAMIN_USE_OneAPI})
  add_definitions(-DUSE_OneAPI)
endif()

if (${EXAMIN_LITE})
  add_definitions(-DEXAMIN_LITE)
endif()

if (${EXAMIN_USE_MP})
  setup_mpir(EXAMIN_MPIR_LIB_DIR EXAMIN_MPIR_INCLUDES EXAMIN_MPIR_LIB)
  setup_mpfr(EXAMIN_MPFR_LIB_DIR EXAMIN_MPFR_INCLUDES EXAMIN_MPFR_LIB)
  setup_mpreal(EXAMIN_MPREAL_INCLUDES)
else()
  add_definitions(-DNO_MPREAL)
endif()

#build output system
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/output_system)

# set include directories for project
set(EXAMIN_COMMON_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(EXAMIN_PARAMETERS_DIR ${PROJECT_SOURCE_DIR}/parameters)

include_directories(
  ${EXAMIN_OPEN_CV_INCLUDES}
  ${EXAMIN_PARAMETERS_DIR}
  ${EXAMIN_MPI_INCLUDES}
  ${EXAMIN_MPIR_INCLUDES}
  ${EXAMIN_MPFR_INCLUDES}
  ${EXAMIN_MPREAL_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/Eigen/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/cppoptlib/include
)

link_directories(
  ${EXAMIN_OPEN_CV_LIB_DIR}
  ${EXAMIN_MPIR_LIB_DIR}
  ${EXAMIN_MPFR_LIB_DIR}
)

file(GLOB ROOT_HEADERS ${EXAMIN_COMMON_SRC_DIR}/*.h ${EXAMIN_COMMON_SRC_DIR}/*.hpp)
file(GLOB PARAMETERS_HEADERS ${EXAMIN_PARAMETERS_DIR}/*.h ${EXAMIN_PARAMETERS_DIR}/*.hpp)
file(GLOB ROOT_SOURCES ${EXAMIN_COMMON_SRC_DIR}/*.c ${EXAMIN_COMMON_SRC_DIR}/*.cpp)

if (EXAMIN_USE_CUDA)
  find_package(CUDA REQUIRED)
endif()

# build main library and executable
set(ALL_SRC_FILES ${ROOT_HEADERS} ${PARAMETERS_HEADERS} ${ROOT_SOURCES})
list(REMOVE_ITEM ALL_SRC_FILES ${EXAMIN_COMMON_SRC_DIR}/main.cpp)
list(REMOVE_ITEM ALL_SRC_FILES ${EXAMIN_COMMON_SRC_DIR}/main_mco.cpp)

add_library(${EXAMIN_LIB_NAME} STATIC ${ALL_SRC_FILES})
if (${EXAMIN_BUILD_PLOTTER})
  add_executable(${EXAMIN_PROJECT_NAME} ${EXAMIN_COMMON_SRC_DIR}/main.cpp ${PROJECT_SOURCE_DIR}/plotter/isolinesPlotter.cpp)
  add_definitions(-DUSE_PLOTTER)
else()
  add_executable(${EXAMIN_PROJECT_NAME} ${EXAMIN_COMMON_SRC_DIR}/main.cpp)
endif()


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
                                                  PRIVATE ${DISLIN_INCLUDE}
                                                  PRIVATE ${PROJECT_SOURCE_DIR}/plotter
												  PRIVATE ${Python_INCLUDE_DIRS} 
												  PRIVATE ${Python_NumPy_INCLUDE_DIRS})
												  
include_directories(${Python_INCLUDE_DIRS} ${Python_NumPy_INCLUDE_DIRS})
												  
target_link_libraries(${EXAMIN_PROJECT_NAME} ${EXAMIN_LIB_NAME} ${Python_LIBRARIES}) #/opt/intel/impi/2017.1.132/intel64/lib/libmpi.so)

if (${EXAMIN_BUILD_MCO})
  add_executable(${EXAMIN_MCO_PROJECT_NAME} ${EXAMIN_COMMON_SRC_DIR}/main_mco.cpp)
  target_include_directories(${EXAMIN_MCO_PROJECT_NAME} PRIVATE ${EXAMIN_COMMON_SRC_DIR})
  target_link_libraries(${EXAMIN_MCO_PROJECT_NAME} ${EXAMIN_LIB_NAME})
endif()

target_link_libraries(${EXAMIN_LIB_NAME}
  ${EXAMIN_OPEN_CV_LIB}
  ${EXAMIN_MPI_LIB}
  ${EXAMIN_MPFR_LIB}
  ${EXAMIN_MPIR_LIB}
  ${DISLIN_LIB}
  output_system
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
  if (${EXAMIN_BUILD_MCO})
    set_target_properties(${EXAMIN_MCO_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -ldl -fopenmp")
  endif()
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

# add plotter
if(${EXAMIN_BUILD_PLOTTER})
  add_subdirectory(${PROJECT_SOURCE_DIR}/plotter)
endif()

#add xml parser lib
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pugixml)
target_link_libraries(${EXAMIN_LIB_NAME} pugixml)

# add problems
if (${EXAMIN_BUILD_PROBLEMS})
    #build 3rd party problems generators
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/grishagin)
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/gkls)

    #build shared libraries
    subdirlist(SUBDIRS ${PROJECT_SOURCE_DIR}/problems)
    foreach(subdir ${SUBDIRS})
        add_subdirectory(${subdir})
    endforeach()
endif()

if(${EXAMIN_CMAKE_VERBOSE})
  print_build_config()
endif()


# create source groups for VS IDE projects
if (IS_MICROSOFT_COMPILER)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_property(TARGET pugixml PROPERTY FOLDER "3rd-party")
  if(${EXAMIN_BUILD_TESTS})
    set_property(TARGET gtests PROPERTY FOLDER "3rd-party")
  endif()
  if(${EXAMIN_BUILD_PROBLEMS})
    set_property(TARGET gkls_functions PROPERTY FOLDER "3rd-party")
    set_property(TARGET grishagin_functions PROPERTY FOLDER "3rd-party")
  endif()

  source_group("Header Files" FILES ${ROOT_HEADERS})
  source_group("Header Files\\Parameters" FILES ${PARAMETERS_HEADERS})
  source_group("Source Files" FILES ${ROOT_SOURCES})
endif()
