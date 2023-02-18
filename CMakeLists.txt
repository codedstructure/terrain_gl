# terrain_gl CMakeLists.txt
# @codedstructure 2022

# This is intended to work in the following environments:
#  * MacOS, both Intel & Apple Silicon
#  * Linux
# With both JetBrains CLion & command line cmake+make

cmake_minimum_required(VERSION 3.22)
project(terrain_gl)
set(CMAKE_CXX_STANDARD 17)

add_executable(terrain_gl
        src/main.cpp
        src/player.cpp
        src/controls.cpp
        src/shader.cpp
        src/heightmap.cpp
        src/texture.cpp
        src/simplexnoise1234.cpp)

# MacOS (Apple Silicon):
#  arch -arm64 brew install cmake make glfw glew glm
# Note that 'make' becomes runnable as 'gmake'
#  arch -arm64 cmake --fresh . && arch -arm64 gmake

# Linux:
#  sudo apt install libglfw3-dev libglew-dev libglm-dev

# GLEW
find_package(GLEW 2.2 REQUIRED)
target_link_libraries(terrain_gl ${GLEW_LIBRARIES})

# GLM
find_package(glm REQUIRED)

# GLFW
find_package(glfw3 3.3 REQUIRED)
target_link_libraries(terrain_gl glfw)

# OpenGL
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
target_link_libraries(terrain_gl ${OPENGL_LIBRARY})