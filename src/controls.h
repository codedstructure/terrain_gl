// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_CONTROLS_H
#define TERRAIN_GL_CONTROLS_H

#include <unordered_map>
#include <GLFW/glfw3.h>

using KeyType = decltype(GLFW_KEY_ESCAPE);

struct PressedKey {
    explicit PressedKey(KeyType key);

    void update(KeyType key, bool pressed) {
        if (key == key_value) {
            state = pressed;
        }
    }

    [[nodiscard]] bool pressed() const {
        return state;
    }
    PressedKey(const PressedKey&) = delete;
    PressedKey& operator=(const PressedKey&) = delete;

    KeyType key_value;
private:
    bool state = false;
};

struct Controls {
    Controls();
    Controls(const Controls&) = delete;
    Controls& operator=(const Controls&) = delete;

    PressedKey k_esc;
    PressedKey k_left;
    PressedKey k_right;
    PressedKey k_up;
    PressedKey k_down;
    PressedKey k_w;
    PressedKey k_a;
    PressedKey k_s;
    PressedKey k_d;
    PressedKey k_z;
    PressedKey k_space;

    float m_thrust = 0.;

    void key_callback(int key, int action);

private:
    void register_pk(PressedKey &pk);
    std::unordered_map<KeyType, PressedKey&> pressed_key_register;
};

#endif //TERRAIN_GL_CONTROLS_H
