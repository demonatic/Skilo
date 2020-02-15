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
    core/network/*.h
    core/network/*.cpp
    storage/*.h
    storage/*.cpp
    utility/*.h
    utility/*.cpp
    utility/*.hpp
    Skilo.h
    Skilo.cpp
    SkiloConfig.h
    SkiloConfig.cpp
)

set(BUILD_DEPS "yes" CACHE STRING "")
set(ALL_SRCS ${ALL_SRCS} ${SOURCES} CACHE INTERNAL "")
set(CORE_LIBS ${G3LOGGER_LIBRARIES} g3logger pthread rocksdb roaring for Rinx)

#include(../cmake/RocksDB.cmake)
#include(../cmake/g3logger.cmake)
include(../cmake/CRoaring.cmake)
include(../cmake/libfor.cmake)

add_executable(Skilo main.cpp ${SOURCES})
target_link_libraries(Skilo ${CORE_LIBS} boost_regex)
