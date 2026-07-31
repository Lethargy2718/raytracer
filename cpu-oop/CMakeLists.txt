cmake_minimum_required(VERSION 3.10)
project(raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(raytracer ${SOURCES})

target_include_directories(raytracer SYSTEM PRIVATE
        ${CMAKE_SOURCE_DIR}/external
)

target_include_directories(raytracer PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/scene
        ${CMAKE_SOURCE_DIR}/src/material
        ${CMAKE_SOURCE_DIR}/src/texture
        ${CMAKE_SOURCE_DIR}/src/io
        ${CMAKE_SOURCE_DIR}/src/render
)

target_compile_definitions(raytracer PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
)

if (MSVC)
    target_compile_options(raytracer PRIVATE /O2)
else()
    target_compile_options(raytracer PRIVATE -O3)
endif()