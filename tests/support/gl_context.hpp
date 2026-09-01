#pragma once

#include <gtest/gtest.h>
#include <gamecoe/core/window.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <support/test_utils.hpp>
#include <optional>

#define SKIP_IF_NO_GL() SKIP_IF_NOT(s_gl_ready, "no display/GL context available")

//==============================================================================
//              GLContextTest - shared GL context fixture base
//==============================================================================

class GLContextTest : public ::testing::Test
{
protected:
    static inline std::optional<gamecoe::window> s_window;
    static inline bool s_gl_ready = false;

    // Not a gtest global Environment: WindowTests already calls glfwTerminate() per-suite,
    // so a shared global context would be torn down while sibling suites still run.
    static void SetUpTestSuite()
    {
        test_utils::init_headless_gl();
        auto result = gamecoe::window::create("GLContextTest", 320, 240);
        if (result.has_value())
        {
            s_window = std::move(*result);
            s_gl_ready = gladLoadGL(glfwGetProcAddress);
        }
        else
        {
            s_gl_ready = false;
        }
    }

    static void TearDownTestSuite()
    {
        s_window.reset();
        s_gl_ready = false;
        glfwTerminate();
    }
};
