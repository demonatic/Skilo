cmake_minimum_required(VERSION 3.5)

option(SKILO_BUILD_TESTS "Build test programs" OFF)

project(Skilo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RINX_ROOT ${CMAKE_SOURCE_DIR}/src/network/Rinx)

include_directories(3rd/include ${RINX_ROOT}/include ${RINX_ROOT}/include/3rd ${CMAKE_SOURCE_DIR}/src)
link_directories(/usr/local/lib)

FIND_PACKAGE(Snappy REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)

set(DEP_ROOT_DIR ${CMAKE_SOURCE_DIR}/external-${CMAKE_SYSTEM_NAME})
set(TRD_DIR ${CMAKE_SOURCE_DIR}/3rd)
if(NOT EXISTS ${DEP_ROOT_DIR})
    file(MAKE_DIRECTORY ${DEP_ROOT_DIR})
endif()

set(BUILD_DEPS "yes" CACHE STRING "")

include(cmake/g3logger.cmake)
include(cmake/libfor.cmake)
include(cmake/RocksDB.cmake)
include(cmake/RocaPinyin.cmake)
#include(cmake/CRoaring.cmake)

include_directories(${DEP_ROOT_DIR}/${G3LOG_NAME}/build/include)
include_directories(${DEP_ROOT_DIR}/${G3LOG_NAME}/src)
include_directories(${DEP_ROOT_DIR}/${ROCKSDB_NAME}/include)
include_directories(${DEP_ROOT_DIR}/${FOR_NAME})
include_directories(${DEP_ROOT_DIR}/${ROCAPinYin_NAME})

add_subdirectory(${RINX_ROOT}/)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
#if(SKILO_BUILD_TESTS)
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
#endif(SKILO_BUILD_TESTS)

find_path(SNAPPY_INCLUDE_DIR snappy.h PATHS
  /usr/local/include
  /opt/local/include
)

#find_library can't seem to find a 64-bit binary if the 32-bit isn't there
set(SNAPPY_LIB_NAMES libsnappy snappy)
set(SNAPPY_LIB_PATHS /usr/local/lib /opt/local/lib /usr/lib64)
find_library(SNAPPY_LIB NAMES ${SNAPPY_LIB_NAMES} PATHS ${SNAPPY_LIB_PATHS})

if (SNAPPY_LIB AND SNAPPY_INCLUDE_DIR)
  set(SNAPPY_FOUND TRUE)
  set(SNAPPY_LIBS ${SNAPPY_LIB})
else ()
  set(SNAPPY_FOUND FALSE)
endif ()

if (SNAPPY_FOUND)
  if (NOT SNAPPY_FIND_QUIETLY)
    message(STATUS "Found Snappy: ${SNAPPY_LIBS}")
  endif ()
else ()
  if (SNAPPY_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find the Snappy library.")
  endif ()
  message(STATUS "Snappy NOT found.")
endif ()

set(ROCKSDB_LIBS ${ROCKSDB_LIBRARIES} ${SNAPPY_LIB} ${ZLIB_LIBRARIES})
set(CORE_LIBS ${G3LOGGER_LIBRARIES} ${FOR_LIBRARIES} ${ROCKSDB_LIBS} ${ROCAPinYin_LIBRARIES} Rinx pthread dl CACHE INTERNAL "")
set(ALL_SRCS CACHE INTERNAL "" FORCE)
