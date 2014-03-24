CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(game CXX)

IF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-3 -std=c++11")
ENDIF()

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
    ${OVR_LIBRARIES}
    glew-static
    soil2)

INCLUDE_DIRECTORIES(
    ${OVR_SOURCE_DIR}/include
    ${tinyobjloader_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/include
    ${SDL2_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}/include
    ${GLmesh_SOURCE_DIR}/include
    ${soil2_SOURCE_DIR}/include
    ${SDL2plus_SOURCE_DIR}/include
    ${GLplus_SOURCE_DIR}/include)

SET(ASSETS
	box.obj box.mtl box.png
	object.vs object.fs
	barrel.vs barrel.fs
	blit.vs blit.fs
	overlaydebug.vs overlaydebug.fs)

FOREACH(assetFile ${ASSETS})
	CONFIGURE_FILE(${assetFile} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
ENDFOREACH()
