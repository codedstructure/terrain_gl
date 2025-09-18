// terrain_gl
// @codedstructure 2023

#include <vector>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

#include "heightmap.h"
#include "simplexnoise1234.h"

// Approach to heightmap rendering here adapted from Chapter 14 of
// "OpenGL ES 3.0 Programming Guide Second Edition",
// Ginsburg & Purnomo, 2014 Pearson Education Ltd.

template<typename T>
HeightMap<T>::HeightMap(int size, int grid_scale, int level) :
    size(size),
    grid_scale(grid_scale),
    level_factor(1 << level)
{
    // vertices, with initial height (y) set to 0.
    // this will make a grid of (n+1) * (n+1) vertices, so there are n*n
    // cells, each with two triangles.
    const float stepSize = size;
    for (int i = 0; i <= size; i++) {
        for (int j = 0; j <= size; j++) {
            grid.push_back((static_cast<float>(i) / stepSize) * grid_scale * level_factor);
            grid.push_back(0.0f);
            grid.push_back((static_cast<float>(j) / stepSize) * grid_scale * level_factor);
        }
    }

    // indices for the vertices above
    // number of vertices along one edge of the grid
    const int vertexEdgeCount = (size + 1);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // Each cell in the grid has two triangles
            grid_indices.push_back(j + i * vertexEdgeCount);
            grid_indices.push_back(j + i * vertexEdgeCount + 1);
            grid_indices.push_back(j + (i+1) * vertexEdgeCount + 1);

            grid_indices.push_back(j + i * vertexEdgeCount);
            grid_indices.push_back(j + (i+1) * vertexEdgeCount + 1);
            grid_indices.push_back(j + (i+1) * vertexEdgeCount);
        }
    }

    // skirts: additional vertices along the edges down to y = -1
    for (int edgeIdx = 0; edgeIdx < 4; edgeIdx++) {
        for (int j = 0; j < size; j++) {
            float x = (edgeIdx % 2 == 0) ? (static_cast<float>(j) / stepSize) : 0.0;
            float y = (edgeIdx % 2 == 1) ? (static_cast<float>(j) / stepSize) : 0.0;
            if (edgeIdx == 2 || edgeIdx == 1) {
                x = 1.0 - x;
            }
            if (edgeIdx == 3 || edgeIdx == 2) {
                y = 1.0 - y;
            }
            grid.push_back(x * grid_scale * level_factor);
            // set 'height' below zero; vertex shader will ignore these
            grid.push_back(-1.0f);
            grid.push_back(y * grid_scale * level_factor);
        }
    }

    const int skirtStartIdx = (size + 1) * (size + 1);
    for (int edgeIdx = 0; edgeIdx < 4; edgeIdx++) {
        for (int j = 0; j < size; j++) {
            int a = skirtStartIdx + edgeIdx * size + j;
            int b = skirtStartIdx + (edgeIdx * size + j + 1) % (4 * size);
            int x = grid[a * 3] * stepSize / grid_scale / level_factor;
            int y = grid[a * 3 + 2] * stepSize / grid_scale / level_factor;
            int c = y + x * (size + 1);
            int dx = edgeIdx == 0 ? 1 : edgeIdx == 2 ? -1 : 0;
            int dy = edgeIdx == 1 ? 1 : edgeIdx == 3 ? -1 : 0;
            int d = (y + dy) + (x + dx) * (size + 1);

            // Each edge 'cell' in the skirt has two triangles
            grid_indices.push_back(a);
            grid_indices.push_back(b);
            grid_indices.push_back(c);

            grid_indices.push_back(b);
            grid_indices.push_back(c);
            grid_indices.push_back(d);
         }
     }
}

template<typename T>
std::pair<int, int> HeightMap<T>::getPatchCoords(float x, float y) {
    x = floor(float(x) / level_factor) * level_factor;
    y = floor(float(y) / level_factor) * level_factor;

    return {x, y};
}

template<typename T>
std::vector<T>& HeightMap<T>::getPatchFor(float fx, float fy) {
    //std::cout << "getPatchFor("<<fx<<","<<fy<<")\n";
    auto [x, y] = getPatchCoords(fx, fy);
    auto key = std::make_pair(x, y);
    auto found_patch = patches.find(key);
    if (found_patch != patches.end()) {
        return found_patch->second;
    }

    auto new_patch = new std::vector<T>;
    generatePatch(x, y, *new_patch);
    patches.emplace(key, *new_patch);

    return *new_patch;
}

template<typename T>
void HeightMap<T>::generatePatch(int grid_x, int grid_y, std::vector<T>& target) {
    std::cout << "generatePatch("<<grid_x<<","<<grid_y<<")\n";
    // size must be a multiple of 8 so these are integers
    auto low = -size * 0.125;
    auto high = size * 1.125;
    auto step_size = float(level_factor) / size;

    for (int y = low; y <= high; y++) {
        for (int x = low; x <= high; x++) {

            float fx = float(x) * step_size + grid_x;
            float fy = float(y) * step_size + grid_y;

            T value = heightAt(fx, fy);
            target.push_back(value);
        }
    }
}

template<typename T>
T HeightMap<T>::heightAt(float x, float y) {
    float value = 0;
    float scale = 30;
    float detail = 1. / 16;
    for (int octave = 0; octave < 10; octave++) {
        value += SimplexNoise1234::noise((x*detail), (y*detail)) * scale;
        scale /= 2;
        detail *= 2;
    }

    //value = (int(x)+int(y)) % 2 == 0 ? int(x) : int (y); // remainder(x+ y, 1) * 20 : -10;

    return value * (grid_scale / 64.f) + 5;
}

// explicit instantiation of available types
template class HeightMap<uint8_t>;
template class HeightMap<float>;
