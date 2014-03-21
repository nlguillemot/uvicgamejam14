cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(glm)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(include/glm)