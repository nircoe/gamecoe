#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace inputcoe
{
    namespace detail
    {
        // Internal usage

        void mousePositionCallback(GLFWwindow *window, double xpos, double ypos);
        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

        void update();
    } // namespace detail

    /// @brief Keyboard keys
    enum class Key
    {
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadDecimal, NumpadDivide, NumpadMultiply, NumpadSubtract, NumpadAdd, NumpadEnter, NumpadEqual,
        Enter, Escape, Space, Backspace, Delete,
        Left, Up, Right, Down,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        Tab, CapsLock, Insert, Home, End, PageUp, PageDown,
        LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt, LeftSuper, RightSuper,
        Apostrophe, Comma, Minus, Period, Slash, Semicolon, Equal,
        LeftBracket, Backslash, RightBracket, GraveAccent,
        PrintScreen, ScrollLock, Pause, Menu, NumLock,
    };

    /// @brief Mouse buttons
    enum class MouseButton
    {
        Left, Middle, Right
    };

    /// @brief Returns the mouse position
    /// @return glm::vec2 (x,y) in pixels
    glm::vec2 mousePosition();
    /// @brief Returns the mouse position normalized
    /// @return glm::vec2 (x,y) normalized [0.0f, 1.0f]
    glm::vec2 mousePositionNormalized();

    /// @brief Returns the mouse delta, 
    ///        the difference between previous and current mouse position
    /// @return glm::vec2 (x,y) in pixels
    glm::vec2 mouseDelta();
    /// @brief Returns the mouse delta normalized, 
    ///        the difference between previous and current mouse position
    /// @return glm::vec2 (x,y) normalized [0.0f, 1.0f]
    glm::vec2 mouseDeltaNormalized();

    
    /// @brief Checks if this key was pressed in this frame
    /// @param key The keyboard key you want to check
    /// @return true if the key was pressed, otherwise false
    bool pressed(Key key);

    /// @brief Checks if this mouse button was pressed in this frame
    /// @param button The mouse button you want to check
    /// @return true if the mouse button was pressed, otherwise false
    bool pressed(MouseButton button);

    /// @brief Checks if this key was released in this frame
    /// @param key The keyboard key you want to check
    /// @return true if the key was released, otherwise false
    bool released(Key key);

    /// @brief Checks if this mouse button was released in this frame
    /// @param button The mouse button you want to check
    /// @return true if the mouse button was released, otherwise false
    bool released(MouseButton button);

    /// @brief Checks if this key is being held now
    /// @param key The keyboard key you want to check
    /// @return true if the key is being held, otherwise false
    bool down(Key key);

    /// @brief Checks if this mouse button is being held now
    /// @param button The mouse button you want to check
    /// @return true if the mouse button is being held, otherwise false
    bool down(MouseButton button);

    /// @brief Checks if this key is not being held now
    /// @param key The keyboard key you want to check
    /// @return true if the key is not being held, otherwise false
    bool up(Key key);

    /// @brief Checks if this mouse button is not being held now
    /// @param button The mouse button you want to check
    /// @return true if the mouse button is not being held, otherwise false
    bool up(MouseButton button);

} // namespace inputcoe
