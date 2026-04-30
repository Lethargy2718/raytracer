cmake_minimum_required(VERSION 3.10)

project(raytracer)

file(GLOB SOURCES
        "*.cpp"
)

add_executable(raytracer ${SOURCES}
        Timer.hpp)