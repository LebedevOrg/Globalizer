cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME_STR output_system)
if(${EXAMIN_USE_OneAPI})
	project (${PROJECT_NAME_STR} CXX)
else()
	project (${PROJECT_NAME_STR})
endif()

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(${PROJECT_NAME_STR} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME_STR} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(${PROJECT_NAME_STR} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${EXAMIN_OUTPUT_DIRECTORY})

if(${EXAMIN_OUTPUT_LEVEL} MATCHES "L1")
  target_compile_definitions(${PROJECT_NAME_STR} PUBLIC -DEXAMIN_OUTPUT_L1)
elseif(${EXAMIN_OUTPUT_LEVEL} MATCHES "L2")
  target_compile_definitions(${PROJECT_NAME_STR} PUBLIC -DEXAMIN_OUTPUT_L2)
endif()

if(CMAKE_BUILD_TYPE MATCHES "DEBUG|Debug")
  target_compile_definitions(${PROJECT_NAME_STR} PUBLIC -D_DEBUG)
endif()

if (IS_MICROSOFT_COMPILER)
  set_property(TARGET ${PROJECT_NAME_STR} PROPERTY FOLDER "3rd-party")
endif()
