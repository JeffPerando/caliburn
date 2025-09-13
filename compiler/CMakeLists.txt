
cmake_minimum_required(VERSION 3.14)

project("caliburn")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR "Ninja")

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE CALIBURN_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h)

message(STATUS "Compiling Caliburn DLL")
add_library(${PROJECT_NAME} SHARED ${CALIBURN_SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC include)

if(MSVC)
	message(STATUS "MSVC detected")
	target_compile_options(${PROJECT_NAME} PUBLIC "/std:c++17")
	target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
	target_link_options(${PROJECT_NAME} PUBLIC "/INCREMENTAL")
endif()

enable_testing()

add_executable(CaliburnTests
	${CALIBURN_SOURCES}
	tests/tokenizer_tests.cpp
)

target_compile_options(CaliburnTests PUBLIC "/std:c++17")
target_compile_options(CaliburnTests PUBLIC "/Zc:__cplusplus")

target_link_libraries(
	CaliburnTests
	PRIVATE
	caliburn
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(CaliburnTests)
