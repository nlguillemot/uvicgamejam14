PROJECT(glew C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(include ${OPENGL_INCLUDE_DIR})

ADD_DEFINITIONS(-DGLEW_STATIC)
ADD_LIBRARY(glew-static ${CMAKE_CURRENT_SOURCE_DIR}/src/glew.c)
TARGET_LINK_LIBRARIES(glew-static ${OPENGL_LIBRARIES})
