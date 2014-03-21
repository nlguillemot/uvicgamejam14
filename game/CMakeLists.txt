CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(game CXX)

IF (UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF ()

ADD_EXECUTABLE(game
    main.cpp)

TARGET_LINK_LIBRARIES(game
    lua
    SDL2-static
    SDL2main
    SDL2plus
    glew-static
    soil2
    GLplus)

INCLUDE_DIRECTORIES(
    ${lua_SOURCE_DIR}/include
    ${SDL2_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${soil2_SOURCE_DIR}/include
    ${SDL2plus_SOURCE_DIR}/include
    ${GLplus_SOURCE_DIR}/include)

CONFIGURE_FILE(mario.png ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
