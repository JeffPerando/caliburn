cmake_minimum_required (VERSION 3.8)
project ("caliburn" LANGUAGES CXX)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

file(GLOB_RECURSE CALIBURN_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h)
message(STATUS "YO")
message(STATUS ${CALIBURN_SOURCES})
message(STATUS "OY")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_executable(${PROJECT_NAME} ${CALIBURN_SOURCES})
	set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
else()
	add_library(${PROJECT_NAME} SHARED)
endif()

include_directories(${PROJECT_NAME} PUBLIC include)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zi")
	target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()
