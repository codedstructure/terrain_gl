// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_TERRAIN_H
#define TERRAIN_GL_TERRAIN_H

#include <map>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "heightmap.h"
#include "shader.h"

const int grid_size = 64;    // edge length of each patch - must be multiple of 4, so 1.25 * grid_size is an int
const int grid_scale = 256;  // patch size in world units
const int numIndices = (grid_size - 1) * (grid_size - 1) * 2 * 3;

class Terrain {
public:
    Terrain(int level, int render_distance, ShaderProgram& program);
    int draw_patch(int grid_x, int grid_y);
    void start_drawing() const;
    unsigned long render_terrain_level(glm::vec2 player_pos, glm::vec2 player_dir);

    int render_distance;
    HeightMap<float> heightMap;
private:
    std::map<std::pair<int, int>, int> grid_layer_map;  // (x,y) -> layer
    std::map<int, std::pair<int, int>> layer_grid_map;  // layer -> (x,y)
    int layer_count;
    int adapted;
    int level;
    GLuint texId;
    GLuint indicesIBO;
    GLuint positionVBO;
    GLint layer_location;
    GLint level_factor_location;
    GLint grid_offset_location;
};

#endif //TERRAIN_GL_TERRAIN_H
