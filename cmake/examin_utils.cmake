function (get_gcc_version arg_verstr)
  string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${CMAKE_CXX_COMPILER_VERSION})
  list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
  list(LENGTH GCC_VERSION_COMPONENTS VERSION_STR_LEN)
  if(VERSION_STR_LEN GREATER 1)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
  else()
    set(GCC_MINOR 0)
  endif()
  set(${arg_verstr} ${GCC_MAJOR}.${GCC_MINOR} PARENT_SCOPE)
endfunction (get_gcc_version)

macro(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child} AND NOT (${child} MATCHES "[.]"))
        list(APPEND dirlist ${curdir}/${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(replace_compiler_flag old_value new_value)
  foreach(flag_var
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES ${old_value})
        string(REGEX REPLACE ${old_value} ${new_value} ${flag_var} "${${flag_var}}")
      endif()
    endforeach(flag_var)
endmacro()

function (examin_initialize)
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(IS_GNU_COMPILER True PARENT_SCOPE)
    get_gcc_version(GCC_VER)
    set(EXAMIN_GCC_VERSION gcc-${GCC_VER} PARENT_SCOPE)
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(IS_INTEL_COMPILER True PARENT_SCOPE)
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(IS_MICROSOFT_COMPILER True PARENT_SCOPE)
    if (${MSVC90})
      set(EXAMIN_VC_VERSION "vc9" PARENT_SCOPE)
    elseif (${MSVC10})
      set(EXAMIN_VC_VERSION "vc10" PARENT_SCOPE)
    elseif (${MSVC11})
      message(FATAL_ERROR "Microsoft Visual Studio 2012 is not supported")
    elseif (${MSVC12})
      set(EXAMIN_VC_VERSION "vc12" PARENT_SCOPE)
    elseif (${MSVC14})
      set(EXAMIN_VC_VERSION "vc14" PARENT_SCOPE)
    endif()
  else()
    message(FATAL_ERROR "Unsupported compiler")
  endif()

  if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(EXAMIN_TARGET_ARCH "x64" PARENT_SCOPE)
  elseif (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(EXAMIN_TARGET_ARCH "x86" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unsupported architecture")
  endif()
endfunction (examin_initialize)

function (set_path arg_var arg_value)
  file(TO_CMAKE_PATH ${arg_value} VAR_CMAKE_PATH)
  set(${arg_var} ${VAR_CMAKE_PATH} PARENT_SCOPE)
endfunction (set_path)

function (select_platform_depended_library arg_lib arg_envlibx86 arg_envlibx64)
  if (EXAMIN_TARGET_ARCH MATCHES "x86")
    set_path(LIB ${arg_envlibx86})
  elseif (EXAMIN_TARGET_ARCH MATCHES "x64")
    set_path(LIB ${arg_envlibx64})
  endif()
  set(${arg_lib} ${LIB} PARENT_SCOPE)
endfunction (select_platform_depended_library)

function (get_platform_depended_library_path arg_out)
  if (WIN32)
    if (IS_MICROSOFT_COMPILER)
      set(${arg_out} lib/win/${EXAMIN_VC_VERSION}/${EXAMIN_TARGET_ARCH} PARENT_SCOPE)
    elseif (IS_INTEL_COMPILER)
      message(FATAL_ERROR "Intel compiler support not implemented for Windows")
    endif()
  elseif (UNIX)
#    if (IS_GNU_COMPILER)
      set(${arg_out} lib/lnx PARENT_SCOPE)
#    elseif (IS_INTEL_COMPILER)
#      message(FATAL_ERROR "Intel compiler support not implemented for UNIX")
#    endif()
  else()
    message(FATAL_ERROR "Unsupported operating system")
  endif()
endfunction (get_platform_depended_library_path)

function (get_libname_from_path arg_path arg_name arg_dir)
  get_filename_component(LIBFILENAME ${arg_path} NAME_WE)
  get_filename_component(LIBDIRPATH ${arg_path} DIRECTORY)
  set_path(LIBDIR ${LIBDIRPATH})
  string(REGEX REPLACE "^lib" "" LIBNAME ${LIBFILENAME})
  set(${arg_name} ${LIBNAME} PARENT_SCOPE)
  set(${arg_dir} ${LIBDIR} PARENT_SCOPE)
endfunction (get_libname_from_path)

macro(print_build_config)
  message("CXX compiler: " ${CMAKE_CXX_COMPILER})
  message("Common compiler flags: " ${CMAKE_CXX_FLAGS})
  message("Build type: " ${CMAKE_BUILD_TYPE})
  message("Target arch: " ${EXAMIN_TARGET_ARCH})

  get_directory_property( DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
  message( "Compile time definitions: " ${DirDefs})

endmacro()

# examin_define_problem(<name> [<USE_CONF> <CUDA> <LINK_LIBS> <libs list> <INCLUDE_DIRS> <dirs list>
#                         <COMPILE_OPTIONS> <options list>])
macro(examin_define_problem _name)

  set(BUILD_PROBLEM "${_name}_build")
  option(${BUILD_PROBLEM} "Build test programs" OFF)

  if(${BUILD_PROBLEM} OR BUILD_ALL_TASK)
  
	#parse arguments
	set(options USE_CONF CUDA)
	set(multiValueArgs INCLUDE_DIRS LINK_LIBS COMPILE_OPTIONS)
	cmake_parse_arguments(examin_define_problem "${options}" "" "${multiValueArgs}" ${ARGN})
	
	#set up current project
	set(PROJECT_NAME_STR ${_name})
	if(${EXAMIN_USE_OneAPI})
		project (${PROJECT_NAME_STR} CXX)
	else()
		project (${PROJECT_NAME_STR})
	endif()
	
	set(PROBLEM_OUTPUT_DIRECTORY ${EXAMIN_OUTPUT_DIRECTORY})
	
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	include_directories(${EXAMIN_COMMON_SRC_DIR})
	file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
	
	if(${examin_define_problem_CUDA})
		include_directories(${CMAKE_SOURCE_DIR}/cuda_interop)
		include_directories(${CUDA_INCLUDE_DIRS})
		file(GLOB CUDA_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cu)
		cuda_add_library(${PROJECT_NAME_STR} SHARED ${SRC_FILES} ${CUDA_SRC_FILES})
	else()
		add_library(${PROJECT_NAME_STR} SHARED ${SRC_FILES})
	endif()
	
	set_target_properties(${PROJECT_NAME_STR} PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY ${PROBLEM_OUTPUT_DIRECTORY})
	
	#setup compiler
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		set_property(TARGET ${PROJECT_NAME_STR} PROPERTY FOLDER "problems")
	endif()
	
	if(examin_define_problem_COMPILE_OPTIONS)
		target_compile_options(${PROJECT_NAME_STR} PUBLIC ${examin_define_problem_COMPILE_OPTIONS})
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		target_compile_options(${PROJECT_NAME_STR} PUBLIC -std=c++11)
	endif()
	
	#copy config file to the binary folder, link xml parser
	if(${examin_define_problem_USE_CONF})
		target_link_libraries(${PROJECT_NAME_STR} pugixml)
		set(CONFIG_OUTPUT_DIR ${PROBLEM_OUTPUT_DIRECTORY})
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME_STR}_conf.xml
		${CONFIG_OUTPUT_DIR}/${PROJECT_NAME_STR}_conf.xml)
	endif()
	
	#resolve additional dependencies
	target_link_libraries(${PROJECT_NAME_STR} ${examin_define_problem_LINK_LIBS})
	target_include_directories(${PROJECT_NAME_STR} PUBLIC ${examin_define_problem_INCLUDE_DIRS})

  endif(${BUILD_PROBLEM} OR BUILD_ALL_TASK)
endmacro()
