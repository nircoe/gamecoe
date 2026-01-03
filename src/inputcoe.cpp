#include <inputcoe.hpp>
#include <gamecoe/core/game.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <gamecoe_config.h>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

namespace
{
    using namespace inputcoe;

    glm::vec2 g_previousMousePosition = glm::vec2(0.0f, 0.0f);
    glm::vec2 g_currentMousePosition = glm::vec2(0.0f, 0.0f);

    glm::vec2 g_previousMousePositionNormalized = glm::vec2(0.0f, 0.0f);
    glm::vec2 g_currentMousePositionNormalized = glm::vec2(0.0f, 0.0f);

    std::unordered_map<Key, bool> g_previousPressedKeys = {};
    std::unordered_map<Key, bool> g_currentPressedKeys = {};

    std::unordered_map<MouseButton, bool> g_previousPressedMouseButtons = {};
    std::unordered_map<MouseButton, bool> g_currentPressedMouseButtons = {};

    std::unordered_map<int, Key> g_glfwToKey = {
        { GLFW_KEY_A, Key::A }, { GLFW_KEY_B, Key::B }, { GLFW_KEY_C, Key::C },
        { GLFW_KEY_D, Key::D }, { GLFW_KEY_E, Key::E }, { GLFW_KEY_F, Key::F },
        { GLFW_KEY_G, Key::G }, { GLFW_KEY_H, Key::H }, { GLFW_KEY_I, Key::I },
        { GLFW_KEY_J, Key::J }, { GLFW_KEY_K, Key::K }, { GLFW_KEY_L, Key::L },
        { GLFW_KEY_M, Key::M }, { GLFW_KEY_N, Key::N }, { GLFW_KEY_O, Key::O },
        { GLFW_KEY_P, Key::P }, { GLFW_KEY_Q, Key::Q }, { GLFW_KEY_R, Key::R },
        { GLFW_KEY_S, Key::S }, { GLFW_KEY_T, Key::T }, { GLFW_KEY_U, Key::U },
        { GLFW_KEY_V, Key::V }, { GLFW_KEY_W, Key::W }, { GLFW_KEY_X, Key::X },
        { GLFW_KEY_Y, Key::Y }, { GLFW_KEY_Z, Key::Z },

        { GLFW_KEY_0, Key::Num0 }, { GLFW_KEY_1, Key::Num1 },
        { GLFW_KEY_2, Key::Num2 }, { GLFW_KEY_3, Key::Num3 },
        { GLFW_KEY_4, Key::Num4 }, { GLFW_KEY_5, Key::Num5 },
        { GLFW_KEY_6, Key::Num6 }, { GLFW_KEY_7, Key::Num7 },
        { GLFW_KEY_8, Key::Num8 }, { GLFW_KEY_9, Key::Num9 },
        
        { GLFW_KEY_KP_0, Key::Numpad0 }, { GLFW_KEY_KP_1, Key::Numpad1 },
        { GLFW_KEY_KP_2, Key::Numpad2 }, { GLFW_KEY_KP_3, Key::Numpad3 },
        { GLFW_KEY_KP_4, Key::Numpad4 }, { GLFW_KEY_KP_5, Key::Numpad5 },
        { GLFW_KEY_KP_6, Key::Numpad6 }, { GLFW_KEY_KP_7, Key::Numpad7 },
        { GLFW_KEY_KP_8, Key::Numpad8 }, { GLFW_KEY_KP_9, Key::Numpad9 },
        { GLFW_KEY_KP_DECIMAL, Key::NumpadDecimal }, { GLFW_KEY_KP_DIVIDE, Key::NumpadDivide },
        { GLFW_KEY_KP_MULTIPLY, Key::NumpadMultiply }, { GLFW_KEY_KP_SUBTRACT, Key::NumpadSubtract },
        { GLFW_KEY_KP_ADD, Key::NumpadAdd }, { GLFW_KEY_KP_ENTER, Key::NumpadEnter },
        { GLFW_KEY_KP_EQUAL, Key::NumpadEqual },

        { GLFW_KEY_ENTER, Key::Enter },  { GLFW_KEY_ESCAPE, Key::Escape }, { GLFW_KEY_SPACE, Key::Space },
        { GLFW_KEY_BACKSPACE, Key::Backspace }, { GLFW_KEY_DELETE, Key::Delete },

        { GLFW_KEY_LEFT, Key::Left }, { GLFW_KEY_UP, Key::Up },
        { GLFW_KEY_RIGHT, Key::Right }, { GLFW_KEY_DOWN, Key::Down },

        { GLFW_KEY_F1, Key::F1 }, { GLFW_KEY_F2, Key::F2 }, { GLFW_KEY_F3, Key::F3 },
        { GLFW_KEY_F4, Key::F4 }, { GLFW_KEY_F5, Key::F5 }, { GLFW_KEY_F6, Key::F6 },
        { GLFW_KEY_F7, Key::F7 }, { GLFW_KEY_F8, Key::F8 }, { GLFW_KEY_F9, Key::F9 },
        { GLFW_KEY_F10, Key::F10 }, { GLFW_KEY_F11, Key::F11 }, { GLFW_KEY_F12, Key::F12 },

        { GLFW_KEY_TAB, Key::Tab }, { GLFW_KEY_CAPS_LOCK, Key::CapsLock },
        { GLFW_KEY_INSERT, Key::Insert }, { GLFW_KEY_HOME, Key::Home },
        { GLFW_KEY_END, Key::End }, { GLFW_KEY_PAGE_UP, Key::PageUp },
        { GLFW_KEY_PAGE_DOWN, Key::PageDown },

        { GLFW_KEY_LEFT_SHIFT, Key::LeftShift }, { GLFW_KEY_RIGHT_SHIFT, Key::RightShift },
        { GLFW_KEY_LEFT_CONTROL, Key::LeftCtrl }, { GLFW_KEY_RIGHT_CONTROL, Key::RightCtrl },
        { GLFW_KEY_LEFT_ALT, Key::LeftAlt }, { GLFW_KEY_RIGHT_ALT, Key::RightAlt },
        { GLFW_KEY_LEFT_SUPER, Key::LeftSuper }, { GLFW_KEY_RIGHT_SUPER, Key::RightSuper },

        { GLFW_KEY_APOSTROPHE, Key::Apostrophe }, { GLFW_KEY_COMMA, Key::Comma },
        { GLFW_KEY_MINUS, Key::Minus }, { GLFW_KEY_PERIOD, Key::Period },
        { GLFW_KEY_SLASH, Key::Slash }, { GLFW_KEY_SEMICOLON, Key::Semicolon },
        { GLFW_KEY_EQUAL, Key::Equal }, { GLFW_KEY_LEFT_BRACKET, Key::LeftBracket },
        { GLFW_KEY_BACKSLASH, Key::Backslash }, { GLFW_KEY_RIGHT_BRACKET, Key::RightBracket },
        { GLFW_KEY_GRAVE_ACCENT, Key::GraveAccent },

        { GLFW_KEY_PRINT_SCREEN, Key::PrintScreen }, { GLFW_KEY_SCROLL_LOCK, Key::ScrollLock },
        { GLFW_KEY_PAUSE, Key::Pause }, { GLFW_KEY_MENU, Key::Menu }, { GLFW_KEY_NUM_LOCK, Key::NumLock }
    };

    std::unordered_map<int, MouseButton> g_glfwToMouseButton = {
        { GLFW_MOUSE_BUTTON_LEFT, MouseButton::Left },
        { GLFW_MOUSE_BUTTON_MIDDLE, MouseButton::Middle },
        { GLFW_MOUSE_BUTTON_RIGHT, MouseButton::Right }
    };
} // anonymous namespace

namespace inputcoe
{
    namespace detail
    {
        void mousePositionCallback(GLFWwindow *window, double xpos, double ypos)
        {
            g_currentMousePosition[0] = xpos;
            g_currentMousePosition[1] = ypos;

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            g_currentMousePositionNormalized[0] = xpos / width;
            g_currentMousePositionNormalized[1] = ypos / height;
        }

        void keyCallback([[maybe_unused]] GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
        {
            if (!g_glfwToKey.contains(key))
            {
#if GAMECOE_USE_LOGCOE
                return logcoe::warning("inputcoe: Unsupported key, look at inputcoe.hpp for supported keys");
#else
                return;
#endif
            }
            
            g_currentPressedKeys[g_glfwToKey[key]] = (action == GLFW_PRESS || action == GLFW_REPEAT); // false if GLFW_RELEASE
        }

        void mouseButtonCallback([[maybe_unused]] GLFWwindow *window, int button, int action, [[maybe_unused]] int mods)
        {
            if (!g_glfwToMouseButton.contains(button))
            {
#if GAMECOE_USE_LOGCOE
                return logcoe::warning("inputcoe: Unsupported mouse button, look at inputcoe.hpp for supported mouse buttons");
#else
                return;
#endif
            }

            g_currentPressedMouseButtons[g_glfwToMouseButton[button]] = action == GLFW_PRESS; // false if GLFW_RELEASE
        }

        void update()
        {
            g_previousMousePosition = g_currentMousePosition;
            g_previousMousePositionNormalized = g_currentMousePositionNormalized;

            g_previousPressedKeys = g_currentPressedKeys;
            g_previousPressedMouseButtons = g_currentPressedMouseButtons;

            glfwPollEvents();
        }
    } // namespace detail

    glm::vec2 mousePosition()
    {
        return g_currentMousePosition;
    }

    glm::vec2 mousePositionNormalized()
    {
        return g_currentMousePositionNormalized;
    }

    glm::vec2 mouseDelta()
    {
        return g_currentMousePosition - g_previousMousePosition;
    }

    glm::vec2 mouseDeltaNormalized()
    {
        return g_currentMousePositionNormalized - g_previousMousePositionNormalized;
    }

    bool pressed(Key key)
    {
        return !g_previousPressedKeys[key] && g_currentPressedKeys[key];
    }

    bool pressed(MouseButton button)
    {
        return !g_previousPressedMouseButtons[button] && g_currentPressedMouseButtons[button];
    }

    bool released(Key key)
    {
        return g_previousPressedKeys[key] && !g_currentPressedKeys[key];
    }

    bool released(MouseButton button)
    {
        return g_previousPressedMouseButtons[button] && !g_currentPressedMouseButtons[button];
    }

    bool down(Key key)
    {
        return g_currentPressedKeys[key];
    }

    bool down(MouseButton button)
    {
        return g_currentPressedMouseButtons[button];
    }

    bool up(Key key)
    {
        return !g_currentPressedKeys[key];
    }

    bool up(MouseButton button)
    {
        return !g_currentPressedMouseButtons[button];
    }

} // namespace inputcoe
