cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES
    core/*.h
    core/*.cpp
    core/schema/*.h
    core/schema/*.cpp
    core/index/*.h
    core/index/*.hpp
    core/index/*.cpp
    core/search/*.h
    core/search/*.hpp
    core/search/*.cpp
    core/network/*.h
    core/network/*.cpp
    storage/*.h
    storage/*.cpp
    storage/*.hpp
    utility/*.h
    utility/*.cpp
    utility/*.hpp
    Skilo.h
    Skilo.cpp
    SkiloConfig.h
    SkiloConfig.cpp
)

MESSAGE("src core libs=" ${CORE_LIBS})
set(ALL_SRCS ${ALL_SRCS} ${SOURCES} CACHE INTERNAL "")

add_executable(Skilo main.cpp ${SOURCES})
target_link_libraries(Skilo ${CORE_LIBS})
