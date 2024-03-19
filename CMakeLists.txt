cmake_minimum_required(VERSION 3.20)
project(superfamicheck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
endif()

if(MSVC)
  add_definitions(-DNOMINMAX)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

set(SOURCES src/superfamicheck.cpp src/sfcRom.cpp)
add_executable(superfamicheck ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
  set_property(TARGET superfamicheck PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
