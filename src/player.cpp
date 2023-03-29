// terrain_gl
// @codedstructure 2023

#include <algorithm>
#include "player.h"

#ifndef M_PI_2f
const float M_PI_2f = 3.1415927 / 2.;
#endif

using glm::vec3, glm::vec4, glm::mat4;

Player player;

void Player::update() {
    if (controls.k_space.pressed()) {
        return;
    }

    /*
    std::cout << "HEADING: " << m_heading.x << " " << m_heading.y << " " << m_heading.z << "\n";
    std::cout << "POSITION: " << m_position.x << " " << m_position.y << " " << m_position.z << "\n";
    std::cout << "VELOCITY: " << m_velocity.x << " " << m_velocity.y << " " << m_velocity.z << "\n";
    std::cout << "UP: " << m_up.x << " " << m_up.y << " " << m_up.z << "\n";
    std::cout << "THRUST: " << m_thrust << "\n";
    std::cout << "PITCH: " << m_pitch << "\n";
    std::cout << "ROLL: " << m_roll << "\n";
    */
    const vec4 world_up = {0., 1., 0., 1.};
    //auto across = glm::cross(vec3(world_up), m_heading);
    auto across = glm::normalize(glm::cross(vec3(m_up), m_heading));
    mat4 roll_rotation = glm::rotate(glm::mat4(1.), m_roll, m_heading);
    mat4 pitch_rotation = glm::rotate(glm::mat4(1.), m_pitch, across);
    m_up = world_up * roll_rotation * pitch_rotation;
    auto up = glm::cross(across, m_heading);

    /*if (k_a && m_thrust < 10.) {
        m_thrust += 0.01;
    } else if (k_z && m_thrust > 0.01) {
        m_thrust -= 0.01;
    } else {
        m_thrust *= 0.99;
    }*/

    float speed = length(m_velocity);
    //std::cout << "FORWARD: " << forward_speed << "\n";
    //if (forward_speed > 0.001) {
    float forward_speed = dot(m_heading, m_velocity);
    //}
    //std::cout << "FORWARD: " << forward_speed << "\n";
    const float mass = 10.f;
    const vec3 gravity{0., -0.01, 0.};
    vec3 weight = mass * gravity;
    vec3 thrust = m_heading * controls.m_thrust;

    // both lift and drag are proportional to speed^2 * air density.
    // air density here is modeled as decreasing linearly with altitude.
    float density = std::max(0.f, 1.f - m_position.y/3.f);
    vec3 lift = 2000.f * m_up * (forward_speed * forward_speed) * density;
    //std::cout << "LIFT_UP: " << lift.y << "\n";
    vec3 drag = -100.f * (m_velocity * speed) * density;
    vec3 force = weight + thrust + lift + drag;
    //std::cout << "FORCE: " << force.x << " " << force.y << " " << force.z << "\n";

    // Stability is good...
    m_roll *= 0.99;
    m_pitch *= 0.9;

    m_velocity += force / mass / 1000.0f;
    m_position += m_velocity;

    if (controls.k_up.pressed() && m_pitch < 0.9) {
        m_pitch += 0.01;
    }
    if (controls.k_down.pressed() && m_pitch > -0.9) {
        m_pitch -= 0.01;
    }
    if (controls.k_left.pressed() && m_roll < M_PI_2f) {
        m_roll += 0.01;
    }
    if (controls.k_right.pressed() && m_roll > -M_PI_2f) {
        m_roll -= 0.01;
    }

    if (controls.k_w.pressed()) {
        m_position += m_heading * (controls.k_shift.pressed() ? controls.value_a : 0.1f);
    }
    if (controls.k_s.pressed()) {
        m_position -= m_heading * (controls.k_shift.pressed() ? controls.value_a : 0.1f);
    }
    if (controls.k_a.pressed()) {
        m_position += across * (controls.k_shift.pressed() ? controls.value_a : 0.1f);
    }
    if (controls.k_d.pressed()) {
        m_position -= across * (controls.k_shift.pressed() ? controls.value_a : 0.1f);
    }
    if (controls.k_q.pressed()) {
        m_heading += across * 0.01f;
        m_heading = glm::normalize(m_heading);
    }
    if (controls.k_e.pressed()) {
        m_heading -= across * 0.01f;
        m_heading = glm::normalize(m_heading);
    }


    mat4 rotation = mat4(1.f);
    rotation = glm::rotate(rotation, -m_roll/50.f, m_heading);
    rotation = glm::rotate(rotation, m_pitch/50.f, across);
    //rotation = glm::rotate(rotation, -m_roll/100.f, up);
    m_heading = glm::normalize(vec3(rotation * vec4(m_heading, 1.)));

    if (m_position.y < 0.5) {
        m_position.y = 0.5;
        if (m_velocity.y < 0.) {
            m_velocity.y = 0.;
        }
    }
}

glm::mat4 Player::getViewMatrix() const {
    return glm::lookAt(
            m_position,           // Camera position
            m_position + m_heading, // Where the camera looks at
            m_up // glm::vec3(0.0f, 1.0f, 0.0f)  // vUp
    );
}
