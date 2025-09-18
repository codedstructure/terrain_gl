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

const int grid_size = 64;   // edge length of each patch - must be multiple of 8, so 0.125 * grid_size is an int
const int grid_scale = 64;  // patch size in world units
const int skirtQuads = 4 * grid_size; // extra vertices for the skirts
const int skirtVertices = 4 * (grid_size + 1);
const int numIndices = (grid_size * grid_size + skirtQuads) * 2 * 3;
const int numVertices = (grid_size+1) * (grid_size+1) + skirtVertices;

class Terrain {
public:
    Terrain(int level, int render_distance, ShaderProgram& program, Terrain* next_level_down);
    int draw_patch(int grid_x, int grid_y);
    void start_drawing() const;
    unsigned long render_terrain_top_level(glm::vec3 player_pos, glm::vec3 player_dir);
    unsigned long render_terrain_sub_level(std::vector<glm::vec2> grid_offsets, int patch_increment, glm::vec3 player_pos, glm::vec3 player_dir);
    unsigned long render_terrain_grid_square(glm::vec3 player_pos, glm::vec3 player_dir, glm::vec2 grid_offset, int patch_increment);

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

    Terrain* next_terrain;
};

#endif //TERRAIN_GL_TERRAIN_H
