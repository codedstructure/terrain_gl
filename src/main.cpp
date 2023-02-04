// terrain_gl
// @codedstructure 2023

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <cstdlib>
#include <vector>
#include <iostream>

#include "player.h"
#include "shader.h"
#include "heightmap.h"

extern Player player;


static void error_callback(int error, const char *description)
{
    std::cerr << "Error: " << description << std::endl;
}

void check_error() {
    if (int e = glGetError() != 0) {
        std::cerr << "Error: " << glewGetErrorString(e) << "\n";
    }
}

struct Context
{
    int width = 800;
    int height = 600;

    void setSize(int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, width, height);
    }
};

int main()
{
    GLFWwindow *window;
    static Context ctx;
    const int grid_size = 128;  // edge length of each patch
    const int grid_scale = 30;  // patch size in world units
    const int render_distance = 6;  // number of patches away to render
    GLint time_location, mvp_location, sampler_location, grid_scale_location, grid_offset_location, background_location;
    HeightMap<float> heightMap(grid_size, grid_scale);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    // 3.2 - 4.1 are supported for macOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // The following two lines are needed for macOS, which doesn't support
    // the compatibility profile for recent OpenGL versions.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(ctx.width, ctx.height, "terrain_gl", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(
            window,
            static_cast<GLFWkeyfun>([](GLFWwindow *window, int key, int scancode, int action, int mods) {
                player.controls.key_callback(key, action);
            }));
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(
            window,
            static_cast<GLFWwindowsizefun>(
                    [](GLFWwindow *window, int new_width, int new_height) {
                        ctx.setSize(new_width, new_height);
                    }));

    ShaderProgram program("shaders/heightmap");
    time_location = program.uniformLocation("u_time");
    mvp_location = program.uniformLocation("u_mvpMatrix");
    sampler_location = program.uniformLocation("u_heightmap");
    grid_scale_location = program.uniformLocation("u_grid_scale");
    grid_offset_location = program.uniformLocation("u_grid_offset");
    background_location = program.uniformLocation("u_background");
    program.activate();

    // Index buffer for base grid
    const int numIndices = (grid_size - 1) * (grid_size - 1) * 2 * 3;
    GLuint indicesIBO;
    glGenBuffers(1, &indicesIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint),
                 &heightMap.grid_indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Position VBO for base grid
    GLuint positionVBO;
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, grid_size * grid_size * sizeof(GLfloat) * 3,
                 &heightMap.grid[0], GL_STATIC_DRAW);

    // We need a vertex array generated for the attrib array later on,
    // even though we never reference VAOId again.
    GLuint VAOId;
    glGenVertexArrays(1, &VAOId);
    glBindVertexArray(VAOId);

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // Mirror texture lookups to avoid edge-effects
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );

    glm::vec3 background_colour{0.1, 0.1, 0.1};

    glEnable(GL_DEPTH_TEST);

    long frame_counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        frame_counter += 1;
        if (player.controls.k_esc.pressed()) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            continue;
        }

        // TODO - pass in dt since last frame, as well as current height
        player.update();

        glClearColor(
                background_colour.r,
                background_colour.g,
                background_colour.b,
                1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc( GL_LEQUAL);

        glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),  // field of view
                float(ctx.width) / float(ctx.height),  // aspect ratio
                0.001f,
                1000.0f);
        glm::mat4 model = glm::translate(
                glm::vec3(
                        -grid_scale / 2.f,
                        0.f,
                        -grid_scale / 2.f
                )
        );
        glm::mat4 mvp = projection * player.getViewMatrix() * model;

        // Render the heightmap
        glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesIBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);

        glUniform1i(sampler_location, 0);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1f(time_location, static_cast<GLfloat>(glfwGetTime()));
        glUniform1f(grid_scale_location, grid_scale);
        glUniform3fv(background_location, 1, glm::value_ptr(background_colour));

        glm::vec2 player_pos(player.m_position.x, player.m_position.z);
        player_pos /= grid_scale;
        glm::vec2 player_dir = glm::normalize(glm::vec2(player.m_heading.x, player.m_heading.z));
        glm::vec2 view_from(player_pos - player_dir);
        glm::vec2 player_loc(view_from.x, view_from.y);
        auto frame_triangles = 0;
        for (int grid_x = int(player_pos.x - render_distance); grid_x <= int(player_pos.x + render_distance); grid_x++) {
            for (int grid_y = int(player_pos.y - render_distance); grid_y <= int(player_pos.y + render_distance); grid_y++) {
                glm::vec2 grid_offset{grid_x, grid_y};
                // Draw the grid square if it's vaguely "in front of us" (cos(theta) > X) and within a reasonable
                // distance.
                if (glm::dot(player_dir, glm::normalize(glm::vec2(grid_offset - player_loc))) > 0.7 &&
                    glm::distance(grid_offset, player_loc) < render_distance) {
                    // TODO: make this more efficient, probably by using multiple textures
                    // (or multi-layer textures?) updated outside this loop.
                    glTexImage2D(
                            GL_TEXTURE_2D, // target
                            0, // level
                            //GL_R8, // internal format
                            GL_R32F, // internal format
                            grid_size, //width
                            grid_size, //height
                            0, // border
                            GL_RED, // format
                            //GL_UNSIGNED_BYTE,
                            GL_FLOAT,
                            &heightMap.getPatch(grid_x, grid_y)[0]
                    );
                    glUniform2fv(grid_offset_location, 1, glm::value_ptr(grid_offset));
                    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
                    frame_triangles += numIndices / 3;
                }
            }
        }
        if (frame_counter % 60 == 0) {
            std::cout << frame_triangles << "\n";
        }
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Get some indication if things aren't going as they should
        check_error();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
