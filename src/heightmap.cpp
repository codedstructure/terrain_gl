// terrain_gl
// @codedstructure 2023

#include <vector>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <cmath>

#include "heightmap.h"

// Approach to heightmap rendering here adapted from Chapter 14 of
// "OpenGL ES 3.0 Programming Guide Second Edition",
// Ginsburg & Purnomo, 2014 Pearson Education Ltd.

template<typename T>
HeightMap<T>::HeightMap(int size, int grid_scale) :
    size(size),
    grid_scale(grid_scale)
{
}

template<typename T>
void HeightMap<T>::generate() {
    const float stepSize = size - 1;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid.push_back((static_cast<float>(i) / stepSize) * grid_scale);
            grid.push_back(0.0f);
            grid.push_back((static_cast<float>(j) / stepSize) * grid_scale);
        }
    }
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            // Each cell in the grid has two triangles
            grid_indices.push_back(j + i * size);
            grid_indices.push_back(j + i * size + 1);
            grid_indices.push_back(j + (i+1) * size + 1);

            grid_indices.push_back(j + i * size);
            grid_indices.push_back(j + (i+1) * size + 1);
            grid_indices.push_back(j + (i+1) * size);
        }
    }
    for (int x=0; x<size; x++) {
        for (int y=0; y<size; y++) {
            auto fx = 2. * float(x)/(size - 1) - 1; // -1..1
            auto fy = 2. * float(y)/(size - 1) - 1; // -1..1
            heights.push_back(
                    static_cast<T>(
                            (fx * fx + fy * fy) - 3
                    )
            );
        }
    }
}

// explicit instantiation of available types
template class HeightMap<uint8_t>;
template class HeightMap<float>;

