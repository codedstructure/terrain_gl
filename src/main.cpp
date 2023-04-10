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
#include <iostream>

#include "player.h"
#include "shader.h"
#include "texture.h"
#include "terrain.h"

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
    int width = 1024;
    int height = 786;

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
    const int HEIGHTMAP_TEX_ID = 0;
    const int STONE_TEX_ID = 1;
    const int render_distance = 2;  // number of patches away to render (0 = only current patch)

    GLint time_location, mvp_location, heightmap_location,
          layer_location, texture_sampler_location, grid_scale_location,
          grid_offset_location, background_location, viewpos_location,
          value_a_location, value_b_location, level_factor_location;

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
    heightmap_location = program.uniformLocation("u_heightmap");
    texture_sampler_location = program.uniformLocation("u_texture");
    grid_scale_location = program.uniformLocation("u_grid_scale");
    background_location = program.uniformLocation("u_background");
    viewpos_location = program.uniformLocation("u_viewpos");
    value_a_location = program.uniformLocation("u_value_a");
    value_b_location = program.uniformLocation("u_value_b");
    program.activate();

    Terrain terrain4(3, render_distance, program);
    Terrain terrain3(2, render_distance, program);
    Terrain terrain2(1, render_distance, program);
    Terrain terrain(0, render_distance, program);

    Texture stone(STONE_TEX_ID, "images/stone-texture.jpg");

    glm::vec3 background_colour{0.6, 0.6, 0.6};

    glEnable(GL_DEPTH_TEST);

    glClearColor(
            background_colour.r,
            background_colour.g,
            background_colour.b,
            1.0);

    long frame_counter = 0;
    double worstFrameTime = 0;
    double frameTime = 0;
    while (!glfwWindowShouldClose(window))
    {
        auto frameStart = glfwGetTime();
        frame_counter += 1;
        if (player.controls.k_esc.pressed()) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            continue;
        }

        // TODO - pass in dt since last frame, as well as current height
        player.update();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc( GL_LEQUAL);

        glm::vec2 player_pos(player.m_position.x, player.m_position.z);
        player_pos /= grid_scale;
        glm::vec2 player_dir = glm::normalize(glm::vec2(player.m_heading.x, player.m_heading.z));

        auto height = terrain.heightMap.heightAt(player_pos.x, player_pos.y);

        glm::mat4 projection = glm::perspective(
                glm::radians(75.0f),  // field of view
                float(ctx.width) / float(ctx.height),  // aspect ratio
                0.1f,
                100000.0f);
        glm::mat4 model = glm::translate(
                glm::vec3(
                        0, // -grid_scale / 2.f,
                        0.f,
                        0 // -grid_scale / 2.f
                )
        );
        if (player.m_position.y < 20 + height) {
            player.m_position.y = height + 20;
        }
        glm::mat4 mvp = projection * player.getViewMatrix() * model;

        // Render the heightmap

        glUniform1i(heightmap_location, HEIGHTMAP_TEX_ID);
        glUniform1i(texture_sampler_location, STONE_TEX_ID);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3fv(viewpos_location, 1, glm::value_ptr(player.m_position));
        glUniform1f(time_location, static_cast<GLfloat>(glfwGetTime()));
        glUniform1f(grid_scale_location, grid_scale);
        glUniform1f(value_a_location, player.controls.value_a);
        glUniform1f(value_b_location, player.controls.value_b);
        glUniform3fv(background_location, 1, glm::value_ptr(background_colour));

        glm::vec2 view_from(player_pos - player_dir);
        glm::vec2 player_loc(view_from.x, view_from.y);
        auto frame_triangles = 0;

        frame_triangles += terrain4.render_terrain_level(player_pos, player_dir);
        frame_triangles += terrain3.render_terrain_level(player_pos, player_dir);
        frame_triangles += terrain2.render_terrain_level(player_pos, player_dir);
        frame_triangles += terrain.render_terrain_level(player_pos, player_dir);

        auto thisFrameTime = glfwGetTime() - frameStart;
        frameTime += thisFrameTime;
        if (thisFrameTime > worstFrameTime) {
            worstFrameTime = thisFrameTime;
        }
        if (frame_counter >= 60) {
            std::cout << frame_triangles << " " << frameTime / 60 << " (worst: " << worstFrameTime << ")\n";
            worstFrameTime = 0;
            frame_counter = 0;
            frameTime = 0;
            std::cout << player_pos.x << ","<< player_pos.y << ": (" << player.m_position.x << "," << player.m_position.y <<"," << player.m_position.z <<")\n";
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
