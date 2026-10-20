#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace inputcoe
{
    namespace detail
    {
        // Internal usage

        void mouse_position_callback(GLFWwindow *window, double xpos, double ypos);
        void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

        void update();
    } // namespace detail

    /// @brief Keyboard keys
    enum class key
    {
        a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
        num0, num1, num2, num3, num4, num5, num6, num7, num8, num9,
        numpad0, numpad1, numpad2, numpad3, numpad4,
        numpad5, numpad6, numpad7, numpad8, numpad9,
        numpad_decimal, numpad_divide, numpad_multiply, numpad_subtract, numpad_add, numpad_enter, numpad_equal,
        enter, escape, space, backspace, delete_,
        left, up, right, down,
        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
        tab, caps_lock, insert, home, end, page_up, page_down,
        left_shift, right_shift, left_ctrl, right_ctrl, left_alt, right_alt, left_super, right_super,
        apostrophe, comma, minus, period, slash, semicolon, equal,
        left_bracket, backslash, right_bracket, grave_accent,
        print_screen, scroll_lock, pause, menu, num_lock,
    };

    /// @brief Mouse buttons
    enum class mouse_button
    {
        left, middle, right
    };

    /// @brief Returns the mouse position
    /// @return glm::vec2 (x,y) in pixels
    [[nodiscard]] glm::vec2 mouse_position();
    /// @brief Returns the mouse position normalized
    /// @return glm::vec2 (x,y) normalized [0.0f, 1.0f]
    [[nodiscard]] glm::vec2 mouse_position_normalized();

    /// @brief Returns the mouse delta,
    ///        the difference between previous and current mouse position
    /// @return glm::vec2 (x,y) in pixels
    [[nodiscard]] glm::vec2 mouse_delta();
    /// @brief Returns the mouse delta normalized,
    ///        the difference between previous and current mouse position
    /// @return glm::vec2 (x,y) normalized [0.0f, 1.0f]
    [[nodiscard]] glm::vec2 mouse_delta_normalized();


    /// @brief Checks if this key was pressed in this frame
    /// @param key_ The keyboard key you want to check
    /// @return true if the key was pressed, otherwise false
    [[nodiscard]] bool pressed(key key_);

    /// @brief Checks if this mouse button was pressed in this frame
    /// @param button The mouse button you want to check
    /// @return true if the mouse button was pressed, otherwise false
    [[nodiscard]] bool pressed(mouse_button button);

    /// @brief Checks if this key was released in this frame
    /// @param key_ The keyboard key you want to check
    /// @return true if the key was released, otherwise false
    [[nodiscard]] bool released(key key_);

    /// @brief Checks if this mouse button was released in this frame
    /// @param button The mouse button you want to check
    /// @return true if the mouse button was released, otherwise false
    [[nodiscard]] bool released(mouse_button button);

    /// @brief Checks if this key is being held now
    /// @param key_ The keyboard key you want to check
    /// @return true if the key is being held, otherwise false
    [[nodiscard]] bool down(key key_);

    /// @brief Checks if this mouse button is being held now
    /// @param button The mouse button you want to check
    /// @return true if the mouse button is being held, otherwise false
    [[nodiscard]] bool down(mouse_button button);

    /// @brief Checks if this key is not being held now
    /// @param key_ The keyboard key you want to check
    /// @return true if the key is not being held, otherwise false
    [[nodiscard]] bool up(key key_);

    /// @brief Checks if this mouse button is not being held now
    /// @param button The mouse button you want to check
    /// @return true if the mouse button is not being held, otherwise false
    [[nodiscard]] bool up(mouse_button button);

} // namespace inputcoe
