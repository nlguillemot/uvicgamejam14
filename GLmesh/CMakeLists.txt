CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(GLmesh CXX)

FIND_LIBRARY(OpenGL_LIBRARY OpenGL)

IF (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-3 -std=c++11")
ENDIF ()

INCLUDE_DIRECTORIES(
    include
    ${tinyobjloader_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
    ${GLplus_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include)

ADD_LIBRARY(GLmesh
    include/GLmesh.hpp
    GLmesh.cpp)

TARGET_LINK_LIBRARIES(GLmesh
    tinyobjloader
    GLplus
    glew-static)

TARGET_LINK_LIBRARIES(GLmesh ${OPENGL_LIBRARIES})
