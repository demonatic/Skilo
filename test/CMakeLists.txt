cmake_minimum_required(VERSION 3.5)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(libart)

set(ALL_SRCS ${ALL_SRCS} CACHE INTERNAL "")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test)

#add_executable(skilo_test ${ALL_SRCS} art_tst.cpp scalar_tst.cpp schema_tst.cpp document_tst.cpp
#                tokenizer_tst.cpp collection_manager_tst.cpp hit_collector_tst.cpp skilo_server_tst.cpp
#                    tst_config_reader.cpp auto_suggest_tst.cpp)
add_executable(skilo_test ${ALL_SRCS} art_tst.cpp)
target_link_libraries(skilo_test gtest gtest_main ${CORE_LIBS})

