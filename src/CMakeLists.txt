cmake_minimum_required(VERSION 3.5)

project(Skilo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(../3rd/include)
link_directories(../3rd/lib)

file(GLOB SOURCES
    Storage/*.h
    Storage/*.cpp
)

add_executable(Skilo main.cpp ${SOURCES})

