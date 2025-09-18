// terrain_gl
// @codedstructure 2023

#include <vector>
#include <GL/glew.h>

#include "terrain.h"


Terrain::Terrain(int level, int render_distance, ShaderProgram& program, Terrain* next_level_down) :
        render_distance(render_distance),
        // 0->1, 1->9, 2->25, 3->49, 4->81, etc
        layer_count(256), //((2 * render_distance) + 1) * ((2 * render_distance) + 1)),
        heightMap(grid_size, grid_scale, level),
        level(level),
        texId(0),
        next_terrain(next_level_down)
{
    layer_location = program.uniformLocation("u_layer");
    level_factor_location = program.uniformLocation("u_level_factor");
    grid_offset_location = program.uniformLocation("u_grid_offset");

    //static_assert(layer_count <= 256);  // OpenGL implementations must support at least 256 layers in 2D array textures
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
    glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // Constant border to exacerbate edge effects - we want to deal with them internally and
    // never hit the edge here.
    // TODO: consider REPEAT vs CONSTANT_BORDER
    glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CONSTANT_BORDER );
    glTexParameteri ( GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CONSTANT_BORDER );
    // The texture is calculated at a larger size than the rendered patch,
    // and the texture coordinates are shifted towards the centre of the
    // texture and reduce edge-effects. Needs to tie up with heightmap
    // generation and the vertex shader...
    adapted = grid_size * 1.25 + 1;
    glTexImage3D(
            GL_TEXTURE_2D_ARRAY, // target
            0, // mipmap level
            GL_R32F, // internal format
            adapted, // width
            adapted, // height
            layer_count, // depth (number of layers)
            0, // border
            GL_RED, // format
            GL_FLOAT,
            nullptr
    );

    // Index buffer for base grid
    glGenBuffers(1, &indicesIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint),
                 &heightMap.grid_indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Position VBO for base grid
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(GLfloat) * 3,
                 &heightMap.grid[0], GL_STATIC_DRAW);

    // We need a vertex array generated for the attrib array later on,
    // even though we never reference VAOId again.
    GLuint VAOId;
    glGenVertexArrays(1, &VAOId);
    glBindVertexArray(VAOId);
}

void Terrain::start_drawing() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);
}

int Terrain::draw_patch(int grid_x, int grid_y) {
    auto layer_idx = 0;
    auto layer = grid_layer_map.find({grid_x, grid_y});
    if (layer != grid_layer_map.end()) {
        layer_idx = layer->second;
    } else {
        // 1. find patch to replace
        // yup, this seems awful, but it does the job reasonably well.
        // (minor concession: ignore the least-random low-order bits)
        auto replace_layer = (rand() >> 8) % layer_count;

        // 2. create new patch for grid_x, grid_y and update texture array
        glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY, // target
                0, // mipmap level
                0, 0, // top-left coord
                replace_layer, // start layer
                adapted, // width
                adapted, // height
                1, // layer count (number of layers)
                GL_RED, // format
                GL_FLOAT,
                &heightMap.getPatchFor(grid_x, grid_y)[0]
        );

        // 3. update the heightmap index arrays
        auto grid = layer_grid_map[replace_layer];
        grid_layer_map.erase(grid);
        grid_layer_map[{grid_x, grid_y}] = replace_layer;
        layer_grid_map[replace_layer] = {grid_x, grid_y};
        layer_idx = replace_layer;
    }
    return layer_idx;
}

int floor_mult(float x, int mult) {
    // round down to multiple of mult
    return floor(x / mult) * mult;
}

unsigned long Terrain::render_terrain_top_level(glm::vec3 player_pos, glm::vec3 player_dir) {

    int max_extent = (1 + render_distance) * heightMap.level_factor;
    int patch_increment = heightMap.level_factor;

    unsigned long frame_triangles = 0;

    int min_x = floor_mult(player_pos.x - max_extent, heightMap.level_factor * 2);
    int max_x = floor_mult(player_pos.x + max_extent, heightMap.level_factor * 2);
    int min_y = floor_mult(player_pos.z - max_extent, heightMap.level_factor * 2);
    int max_y = floor_mult(player_pos.z + max_extent, heightMap.level_factor * 2);

    std::vector<glm::vec2> sub_level_grid_offsets;
    for (int grid_y = min_y; grid_y <= max_y + patch_increment; grid_y += patch_increment) {
        for (int grid_x = min_x; grid_x <= max_x + patch_increment; grid_x += patch_increment) {
            glm::vec2 grid_offset{grid_x, grid_y};
            float dist = glm::length(player_pos-glm::vec3(grid_offset.x, 0.0, grid_offset.y));
            if (dist > 5*patch_increment){
                continue;
            }
            if (next_terrain != nullptr &&
                dist < patch_increment*2) {
                sub_level_grid_offsets.push_back(grid_offset);
                continue;
            }

            frame_triangles += render_terrain_grid_square(player_pos, player_dir, grid_offset, patch_increment);
        }
    }

    if (next_terrain != nullptr) {
        frame_triangles += next_terrain->render_terrain_sub_level(
                sub_level_grid_offsets,
                patch_increment,
                player_pos,
                player_dir
        );
    }
    return frame_triangles;
}

unsigned long Terrain::render_terrain_sub_level(std::vector<glm::vec2> grid_offsets, int patch_increment, glm::vec3 player_pos, glm::vec3 player_dir){
    unsigned long frame_triangles = 0;
    std::vector<glm::vec2> sub_level_grid_offsets;
    int new_patch_increment = heightMap.level_factor;
    int ratio = patch_increment/new_patch_increment;// always 2 for now

    for (int i = 0; i < grid_offsets.size(); i++){
        for (int j = 0; j < ratio; j++){
            for (int k = 0; k < ratio; k++){
                glm::vec2 grid_offset = grid_offsets[i] + glm::vec2(j*new_patch_increment,k*new_patch_increment);
                float dist = glm::length(player_pos-glm::vec3(grid_offset.x, 0.0, grid_offset.y));
                if (next_terrain != nullptr && dist < new_patch_increment*2){
                    sub_level_grid_offsets.push_back(grid_offset);
                    continue;
                }


                frame_triangles += render_terrain_grid_square(player_pos, player_dir, grid_offset ,new_patch_increment);
            }
        }
    }
    //std::cout << "\n";
    if (next_terrain != nullptr){
        frame_triangles += next_terrain->render_terrain_sub_level(sub_level_grid_offsets, new_patch_increment, player_pos, player_dir);
    }

    return frame_triangles;
}

unsigned long Terrain::render_terrain_grid_square(glm::vec3 player_pos, glm::vec3 player_dir, glm::vec2 grid_offset, int patch_increment){
    unsigned long frame_triangles = 0;

    // Draw the grid square if it's vaguely "in front of us" (cos(theta) > X) and within a reasonable
    // distance.

    float min_val = 0.5;

    bool corner_tested = (glm::dot(player_dir, glm::normalize(glm::vec3(grid_offset.x,0.0,grid_offset.y) - player_pos)) > min_val)
        ||(glm::dot(player_dir, glm::normalize(glm::vec3(grid_offset.x,0.0,grid_offset.y+patch_increment) - player_pos)) > min_val)
        ||(glm::dot(player_dir, glm::normalize(glm::vec3(grid_offset.x+patch_increment,0.0,grid_offset.y) - player_pos)) > min_val)
        ||(glm::dot(player_dir, glm::normalize(glm::vec3(grid_offset.x+patch_increment,0.0,grid_offset.y+patch_increment) - player_pos)) > min_val);

    if (corner_tested) {

        start_drawing();
        auto [g_x, g_y] = heightMap.getPatchCoords(grid_offset.x, grid_offset.y);
        auto layer_idx = draw_patch(g_x, g_y);
        grid_offset = {g_x, g_y};
        glUniform1i(layer_location, layer_idx);
        glUniform1i(level_factor_location, static_cast<GLint>(heightMap.level_factor));

        glUniform2fv(grid_offset_location, 1, glm::value_ptr(grid_offset));
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
        frame_triangles += numIndices / 3;
    }

    return frame_triangles;
}
