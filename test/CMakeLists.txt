cmake_minimum_required(VERSION 3.20)
project(superfamicheck_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  Include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.3
  )
  FetchContent_MakeAvailable(Catch2)
endif()

set(SOURCES test.cpp ../src/sfcRom.cpp)
add_executable(test ${SOURCES})
target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
