#include <inputcoe.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

namespace
{
    using namespace inputcoe;

    glm::vec2 g_previous_mouse_position = glm::vec2(0.0f, 0.0f);
    glm::vec2 g_current_mouse_position = glm::vec2(0.0f, 0.0f);

    glm::vec2 g_previous_mouse_position_normalized = glm::vec2(0.0f, 0.0f);
    glm::vec2 g_current_mouse_position_normalized = glm::vec2(0.0f, 0.0f);

    std::unordered_map<key, bool> g_previous_pressed_keys = {};
    std::unordered_map<key, bool> g_current_pressed_keys = {};

    std::unordered_map<mouse_button, bool> g_previous_pressed_mouse_buttons = {};
    std::unordered_map<mouse_button, bool> g_current_pressed_mouse_buttons = {};

    std::unordered_map<int, key> g_glfw_to_key = {
        { GLFW_KEY_A, key::a }, { GLFW_KEY_B, key::b }, { GLFW_KEY_C, key::c },
        { GLFW_KEY_D, key::d }, { GLFW_KEY_E, key::e }, { GLFW_KEY_F, key::f },
        { GLFW_KEY_G, key::g }, { GLFW_KEY_H, key::h }, { GLFW_KEY_I, key::i },
        { GLFW_KEY_J, key::j }, { GLFW_KEY_K, key::k }, { GLFW_KEY_L, key::l },
        { GLFW_KEY_M, key::m }, { GLFW_KEY_N, key::n }, { GLFW_KEY_O, key::o },
        { GLFW_KEY_P, key::p }, { GLFW_KEY_Q, key::q }, { GLFW_KEY_R, key::r },
        { GLFW_KEY_S, key::s }, { GLFW_KEY_T, key::t }, { GLFW_KEY_U, key::u },
        { GLFW_KEY_V, key::v }, { GLFW_KEY_W, key::w }, { GLFW_KEY_X, key::x },
        { GLFW_KEY_Y, key::y }, { GLFW_KEY_Z, key::z },

        { GLFW_KEY_0, key::num0 }, { GLFW_KEY_1, key::num1 },
        { GLFW_KEY_2, key::num2 }, { GLFW_KEY_3, key::num3 },
        { GLFW_KEY_4, key::num4 }, { GLFW_KEY_5, key::num5 },
        { GLFW_KEY_6, key::num6 }, { GLFW_KEY_7, key::num7 },
        { GLFW_KEY_8, key::num8 }, { GLFW_KEY_9, key::num9 },

        { GLFW_KEY_KP_0, key::numpad0 }, { GLFW_KEY_KP_1, key::numpad1 },
        { GLFW_KEY_KP_2, key::numpad2 }, { GLFW_KEY_KP_3, key::numpad3 },
        { GLFW_KEY_KP_4, key::numpad4 }, { GLFW_KEY_KP_5, key::numpad5 },
        { GLFW_KEY_KP_6, key::numpad6 }, { GLFW_KEY_KP_7, key::numpad7 },
        { GLFW_KEY_KP_8, key::numpad8 }, { GLFW_KEY_KP_9, key::numpad9 },
        { GLFW_KEY_KP_DECIMAL, key::numpad_decimal }, { GLFW_KEY_KP_DIVIDE, key::numpad_divide },
        { GLFW_KEY_KP_MULTIPLY, key::numpad_multiply }, { GLFW_KEY_KP_SUBTRACT, key::numpad_subtract },
        { GLFW_KEY_KP_ADD, key::numpad_add }, { GLFW_KEY_KP_ENTER, key::numpad_enter },
        { GLFW_KEY_KP_EQUAL, key::numpad_equal },

        { GLFW_KEY_ENTER, key::enter },  { GLFW_KEY_ESCAPE, key::escape }, { GLFW_KEY_SPACE, key::space },
        { GLFW_KEY_BACKSPACE, key::backspace }, { GLFW_KEY_DELETE, key::delete_ },

        { GLFW_KEY_LEFT, key::left }, { GLFW_KEY_UP, key::up },
        { GLFW_KEY_RIGHT, key::right }, { GLFW_KEY_DOWN, key::down },

        { GLFW_KEY_F1, key::f1 }, { GLFW_KEY_F2, key::f2 }, { GLFW_KEY_F3, key::f3 },
        { GLFW_KEY_F4, key::f4 }, { GLFW_KEY_F5, key::f5 }, { GLFW_KEY_F6, key::f6 },
        { GLFW_KEY_F7, key::f7 }, { GLFW_KEY_F8, key::f8 }, { GLFW_KEY_F9, key::f9 },
        { GLFW_KEY_F10, key::f10 }, { GLFW_KEY_F11, key::f11 }, { GLFW_KEY_F12, key::f12 },

        { GLFW_KEY_TAB, key::tab }, { GLFW_KEY_CAPS_LOCK, key::caps_lock },
        { GLFW_KEY_INSERT, key::insert }, { GLFW_KEY_HOME, key::home },
        { GLFW_KEY_END, key::end }, { GLFW_KEY_PAGE_UP, key::page_up },
        { GLFW_KEY_PAGE_DOWN, key::page_down },

        { GLFW_KEY_LEFT_SHIFT, key::left_shift }, { GLFW_KEY_RIGHT_SHIFT, key::right_shift },
        { GLFW_KEY_LEFT_CONTROL, key::left_ctrl }, { GLFW_KEY_RIGHT_CONTROL, key::right_ctrl },
        { GLFW_KEY_LEFT_ALT, key::left_alt }, { GLFW_KEY_RIGHT_ALT, key::right_alt },
        { GLFW_KEY_LEFT_SUPER, key::left_super }, { GLFW_KEY_RIGHT_SUPER, key::right_super },

        { GLFW_KEY_APOSTROPHE, key::apostrophe }, { GLFW_KEY_COMMA, key::comma },
        { GLFW_KEY_MINUS, key::minus }, { GLFW_KEY_PERIOD, key::period },
        { GLFW_KEY_SLASH, key::slash }, { GLFW_KEY_SEMICOLON, key::semicolon },
        { GLFW_KEY_EQUAL, key::equal }, { GLFW_KEY_LEFT_BRACKET, key::left_bracket },
        { GLFW_KEY_BACKSLASH, key::backslash }, { GLFW_KEY_RIGHT_BRACKET, key::right_bracket },
        { GLFW_KEY_GRAVE_ACCENT, key::grave_accent },

        { GLFW_KEY_PRINT_SCREEN, key::print_screen }, { GLFW_KEY_SCROLL_LOCK, key::scroll_lock },
        { GLFW_KEY_PAUSE, key::pause }, { GLFW_KEY_MENU, key::menu }, { GLFW_KEY_NUM_LOCK, key::num_lock }
    };

    std::unordered_map<int, mouse_button> g_glfw_to_mouse_button = {
        { GLFW_MOUSE_BUTTON_LEFT, mouse_button::left },
        { GLFW_MOUSE_BUTTON_MIDDLE, mouse_button::middle },
        { GLFW_MOUSE_BUTTON_RIGHT, mouse_button::right }
    };
} // anonymous namespace

namespace inputcoe
{
    namespace detail
    {
        void mouse_position_callback(GLFWwindow *window, double xpos, double ypos)
        {
            g_current_mouse_position[0] = static_cast<float>(xpos);
            g_current_mouse_position[1] = static_cast<float>(ypos);

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            g_current_mouse_position_normalized[0] = static_cast<float>(xpos) / width;
            g_current_mouse_position_normalized[1] = static_cast<float>(ypos) / height;
        }

        void key_callback([[maybe_unused]] GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
        {
            if (!g_glfw_to_key.contains(key))
                return logcoe::warning("inputcoe: Unsupported key, look at inputcoe.hpp for supported keys");

            g_current_pressed_keys[g_glfw_to_key[key]] = (action == GLFW_PRESS || action == GLFW_REPEAT); // false if GLFW_RELEASE
        }

        void mouse_button_callback([[maybe_unused]] GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
        {
            if (!g_glfw_to_mouse_button.contains(button))
                return logcoe::warning("inputcoe: Unsupported mouse button, look at inputcoe.hpp for supported mouse buttons");

            g_current_pressed_mouse_buttons[g_glfw_to_mouse_button[button]] = action == GLFW_PRESS; // false if GLFW_RELEASE
        }

        void update()
        {
            g_previous_mouse_position = g_current_mouse_position;
            g_previous_mouse_position_normalized = g_current_mouse_position_normalized;

            g_previous_pressed_keys = g_current_pressed_keys;
            g_previous_pressed_mouse_buttons = g_current_pressed_mouse_buttons;

            glfwPollEvents();
        }
    } // namespace detail

    glm::vec2 mouse_position()
    {
        return g_current_mouse_position;
    }

    glm::vec2 mouse_position_normalized()
    {
        return g_current_mouse_position_normalized;
    }

    glm::vec2 mouse_delta()
    {
        return g_current_mouse_position - g_previous_mouse_position;
    }

    glm::vec2 mouse_delta_normalized()
    {
        return g_current_mouse_position_normalized - g_previous_mouse_position_normalized;
    }

    bool pressed(key key_)
    {
        return !g_previous_pressed_keys[key_] && g_current_pressed_keys[key_];
    }

    bool pressed(mouse_button button)
    {
        return !g_previous_pressed_mouse_buttons[button] && g_current_pressed_mouse_buttons[button];
    }

    bool released(key key_)
    {
        return g_previous_pressed_keys[key_] && !g_current_pressed_keys[key_];
    }

    bool released(mouse_button button)
    {
        return g_previous_pressed_mouse_buttons[button] && !g_current_pressed_mouse_buttons[button];
    }

    bool down(key key_)
    {
        return g_current_pressed_keys[key_];
    }

    bool down(mouse_button button)
    {
        return g_current_pressed_mouse_buttons[button];
    }

    bool up(key key_)
    {
        return !g_current_pressed_keys[key_];
    }

    bool up(mouse_button button)
    {
        return !g_current_pressed_mouse_buttons[button];
    }

} // namespace inputcoe
