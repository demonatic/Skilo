cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEP_ROOT_DIR ${CMAKE_SOURCE_DIR}/external-${CMAKE_SYSTEM_NAME})
set(TRD_DIR ${CMAKE_SOURCE_DIR}/3rd)
if(NOT EXISTS ${DEP_ROOT_DIR})
    file(MAKE_DIRECTORY ${DEP_ROOT_DIR})
endif()

file(GLOB SOURCES
    core/*.h
    core/*.cpp
    core/schema/*.h
    core/schema/*.cpp
    core/index/*.h
    core/index/*.hpp
    core/index/*.cpp
    core/search/*.h
    core/search/*.cpp
    storage/*.h
    storage/*.cpp
    util/*.h
    util/*.cpp
    util/*.hpp
)

set(BUILD_DEPS "yes" CACHE STRING "")
set(ALL_SRCS ${ALL_SRCS} ${SOURCES} CACHE INTERNAL "description")
set(CORE_LIBS ${G3LOGGER_LIBRARIES} g3logger pthread rocksdb roaring for)

#include(../cmake/RocksDB.cmake)
#include(../cmake/g3logger.cmake)
include(../cmake/CRoaring.cmake)
include(../cmake/libfor.cmake)

add_executable(Skilo main.cpp ${SOURCES})
target_link_libraries(Skilo ${CORE_LIBS})
