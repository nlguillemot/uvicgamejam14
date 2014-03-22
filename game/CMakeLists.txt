CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(game CXX)

IF (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-3 -std=c++11")
ENDIF ()

ADD_EXECUTABLE(game
    main.cpp)

TARGET_LINK_LIBRARIES(game
    GLplus
    tinyobjloader
    GLmesh
    lua
    SDL2-static
    SDL2main
    SDL2plus
    glew-static
    soil2)

INCLUDE_DIRECTORIES(
    ${tinyobjloader_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/include
    ${SDL2_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}/include
    ${GLmesh_SOURCE_DIR}/include
    ${soil2_SOURCE_DIR}/include
    ${SDL2plus_SOURCE_DIR}/include
    ${GLplus_SOURCE_DIR}/include)

CONFIGURE_FILE(cube.obj ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
CONFIGURE_FILE(cube.mtl ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)


