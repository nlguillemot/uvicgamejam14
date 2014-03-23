CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(SDL2plus CXX)

IF (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-3 -std=c++11")
ENDIF ()

INCLUDE_DIRECTORIES(
    include
    ${glew_SOURCE_DIR}/include
    ${SDL2_SOURCE_DIR}/include)

ADD_LIBRARY(SDL2plus
    include/SDL2plus.hpp
    SDL2plus.cpp)

TARGET_LINK_LIBRARIES(SDL2plus
    glew-static
    SDL2-static)
