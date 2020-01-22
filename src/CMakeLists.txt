cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEP_ROOT_DIR ${CMAKE_SOURCE_DIR}/external-${CMAKE_SYSTEM_NAME})
set(TRD_DIR ${CMAKE_SOURCE_DIR}/3rd)
if(NOT EXISTS ${DEP_ROOT_DIR})
    file(MAKE_DIRECTORY ${DEP_ROOT_DIR})
endif()

#include(../cmake/RocksDB.cmake)
#include(../cmake/g3logger.cmake)
include(../cmake/CRoaring.cmake)

file(GLOB SOURCES
    storage/*.h
    storage/*.cpp
    core/*.h
    core/*.cpp
    core/index/*.h
    core/index/*.cpp
    util/*.hpp
)

add_executable(Skilo main.cpp ${SOURCES})

set(CORE_LIBS ${G3LOGGER_LIBRARIES} g3logger pthread rocksdb roaring)
target_link_libraries(Skilo ${CORE_LIBS})
