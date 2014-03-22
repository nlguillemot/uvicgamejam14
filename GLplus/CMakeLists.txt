CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(GLplus CXX)

FIND_LIBRARY(OpenGL_LIBRARY OpenGL)

IF (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-3 -std=c++11")
ENDIF ()

INCLUDE_DIRECTORIES(
    include
    ${OPENGL_INCLUDE_DIR}
    ${glew_SOURCE_DIR}/include
    ${soil2_SOURCE_DIR}/include)

ADD_LIBRARY(GLplus
    include/GLplus.hpp
    GLplus.cpp)

TARGET_LINK_LIBRARIES(GLplus
    soil2
    glew-static)

TARGET_LINK_LIBRARIES(GLplus ${OPENGL_LIBRARIES})
