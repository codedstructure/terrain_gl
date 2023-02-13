// terrain_gl
// @codedstructure 2023

#include <map>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include "controls.h"


PressedKey::PressedKey(KeyType key) :
key_value(key) {
//    pressed_key_register.emplace(key, *this);
}

Controls::Controls() :
    k_esc(GLFW_KEY_ESCAPE),
    k_left(GLFW_KEY_LEFT),
    k_right(GLFW_KEY_RIGHT),
    k_up(GLFW_KEY_UP),
    k_down(GLFW_KEY_DOWN),
    k_w(GLFW_KEY_W),
    k_a(GLFW_KEY_A),
    k_s(GLFW_KEY_S),
    k_d(GLFW_KEY_D),
    k_z(GLFW_KEY_Z),
    k_space(GLFW_KEY_SPACE)
{
    register_pk(k_esc);
    register_pk(k_left);
    register_pk(k_right);
    register_pk(k_up);
    register_pk(k_down);
    register_pk(k_w);
    register_pk(k_a);
    register_pk(k_s);
    register_pk(k_d);
    register_pk(k_z);
    register_pk(k_space);
}

void Controls::register_pk(PressedKey &pk) {
    pressed_key_register.emplace(pk.key_value, pk);
}

void Controls::key_callback(int key, int action)
{
    bool pressed;
    if (action == GLFW_PRESS)
    {
        pressed = true;
    }
    else if (action == GLFW_RELEASE)
    {
        pressed = false;
    }
    else {
        // Ensure we don't clear key state in the event of unexpected actions.
        return;
    }

    auto key_press = pressed_key_register.find(key);
    if (key_press != pressed_key_register.end()) {
        key_press->second.update(key, pressed);
    }

    if (pressed) {
        switch (key) {
            case GLFW_KEY_1:
            case GLFW_KEY_KP_1:
                m_thrust = 1;
                break;
            case GLFW_KEY_2:
            case GLFW_KEY_KP_2:
                m_thrust = 2;
                break;
            case GLFW_KEY_3:
            case GLFW_KEY_KP_3:
                m_thrust = 3;
                break;
            case GLFW_KEY_4:
            case GLFW_KEY_KP_4:
                m_thrust = 4;
                break;
            case GLFW_KEY_5:
            case GLFW_KEY_KP_5:
                m_thrust = 5;
                break;
            case GLFW_KEY_6:
            case GLFW_KEY_KP_6:
                m_thrust = 6;
                break;
            case GLFW_KEY_7:
            case GLFW_KEY_KP_7:
                m_thrust = 7;
                break;
            case GLFW_KEY_8:
            case GLFW_KEY_KP_8:
                m_thrust = 8;
                break;
            case GLFW_KEY_9:
            case GLFW_KEY_KP_9:
                m_thrust = 9;
                break;
            case GLFW_KEY_0:
            case GLFW_KEY_KP_0:
                m_thrust = 0;
                break;
            default:
                // Do nothing
                break;
        }
    }
}
