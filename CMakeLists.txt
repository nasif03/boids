cmake_minimum_required(VERSION 3.20)

project(boids)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

add_executable(boids
    src/main.cpp
    src/glad.c
)

target_include_directories(boids PRIVATE
    include
)

target_link_libraries(boids PRIVATE
    glfw
    opengl32
)