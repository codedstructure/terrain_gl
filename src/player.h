// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_PLAYER_H
#define TERRAIN_GL_PLAYER_H

#include <glm/geometric.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "controls.h"


struct Player
{
    void update();
    [[nodiscard]] glm::mat4 getViewMatrix() const;

    glm::vec3 m_velocity = {0., 0., 0.};
    glm::vec3 m_position = {0., 0.2, 0.};
    glm::vec3 m_heading = {0., 0., -1.,};
    glm::vec3 m_up = {0., 1., 0.};
    float m_roll = 0.;
    float m_pitch = 0.;

    Controls controls;
};

#endif //TERRAIN_GL_PLAYER_H
