cmake_minimum_required(VERSION 3.5)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(libart)

set(ALL_SRCS ${ALL_SRCS} libart/art.c art_tst.cpp scalar_tst.cpp schema_tst.cpp CACHE INTERNAL "description")
set(CORE_LIBS ${G3LOGGER_LIBRARIES} g3logger pthread rocksdb roaring for)

#message("SRCS: " ${ALL_SRCS})

add_executable(skilo_test ${ALL_SRCS} libart/art.c art_tst.cpp scalar_tst.cpp schema_tst.cpp
    document_tst.cpp tokenizer_tst.cpp collection_manager_tst.cpp hit_collector_tst.cpp)
set_source_files_properties(libart/art.c PROPERTIES LANGUAGE CXX )


target_link_libraries(skilo_test gtest gtest_main ${CORE_LIBS})

