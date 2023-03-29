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
    const float stepSize = size - 1;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid.push_back((static_cast<float>(i) / stepSize) * grid_scale * level_factor);
            grid.push_back(0.0f);
            grid.push_back((static_cast<float>(j) / stepSize) * grid_scale * level_factor);
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
    auto adapted = size * 1.25;
    auto step_size = float(level_factor) / size;
    for (int y=0; y<adapted; y++) {
        for (int x=0; x<adapted; x++) {
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
