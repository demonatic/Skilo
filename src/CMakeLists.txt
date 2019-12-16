cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include(../cmake/RocksDB.cmake)

include_directories(../3rd/include)
link_directories(../3rd/lib)

file(GLOB SOURCES
    Storage/*.h
    Storage/*.cpp
    Core/*.h
    Core/*.cpp
)

add_executable(Skilo main.cpp ${SOURCES})

