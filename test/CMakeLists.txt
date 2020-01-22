cmake_minimum_required(VERSION 3.5)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(libart)

add_executable(skilo_test libart/art.c art_tst.cpp)
target_link_libraries(skilo_test gtest gtest_main)
set_source_files_properties(libart/art.c PROPERTIES LANGUAGE CXX )

gtest_discover_tests(skilo_test)
