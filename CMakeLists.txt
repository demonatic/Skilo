cmake_minimum_required(VERSION 3.5)

project(Skilo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/network/Rinx)

include_directories(3rd/include)
link_directories(3rd/lib /usr/local/lib)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/test)


set(ALL_SRCS CACHE INTERNAL "" FORCE)
