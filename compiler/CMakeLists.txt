cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GENERATOR "Ninja")

project ("caliburn" LANGUAGES CXX)

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE CALIBURN_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h)
message(STATUS "YO")
message(STATUS ${CALIBURN_SOURCES})
message(STATUS "OY")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build)

if(CBRN_OUT STREQUAL "EXE")
	message(STATUS "Compiling Caliburn EXE")
	add_executable(${PROJECT_NAME} ${CALIBURN_SOURCES})
	set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
elseif(CBRN_OUT STREQUAL "DLL")
	message(STATUS "Compiling Caliburn DLL")
	add_library(${PROJECT_NAME} SHARED ${CALIBURN_SOURCES})
else()
	message(STATUS "Please set Caliburn's output via CBRN_OUT; Valid values are DLL and EXE")
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

include_directories(${PROJECT_NAME} PUBLIC include)

if(MSVC)
	message(STATUS "MSVC detected")
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zi")
	target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()
