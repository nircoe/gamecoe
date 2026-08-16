#include <gtest/gtest.h>
#include <gamecoe/core/window.hpp>
#include <GLFW/glfw3.h>

using namespace gamecoe;

#define SKIP_IF_NO_WINDOW(result) \
    if (!(result).has_value()) \
    { \
        GTEST_SKIP() << "window::create() failed - no display/GL context available"; \
    }

//==============================================================================
//                    WindowTests - window class tests
//==============================================================================

class WindowTests : public ::testing::Test
{
protected:
    static void SetUpTestSuite() { glfwInit(); }
    static void TearDownTestSuite() { glfwTerminate(); }
};

//==============================================================================
//                        Aspect Ratio
//==============================================================================

TEST_F(WindowTests, AspectRatio)
{
    // Test 1: 800x600
    {
        auto result = window::create("WindowTests.AspectRatio.1", 800, 600);
        SKIP_IF_NO_WINDOW(result);

        EXPECT_FLOAT_EQ(result->aspect_ratio(), 800.0f / 600.0f);
    }

    // Test 2: 1920x1080
    {
        auto result = window::create("WindowTests.AspectRatio.2", 1920, 1080);
        SKIP_IF_NO_WINDOW(result);

        EXPECT_FLOAT_EQ(result->aspect_ratio(), 1920.0f / 1080.0f);
    }
}

//==============================================================================
//                        Move Construction
//==============================================================================

TEST_F(WindowTests, MoveConstruction)
{
    auto result = window::create("WindowTests.MoveConstruction", 640, 480);
    SKIP_IF_NO_WINDOW(result);

    window moved(std::move(*result));

    EXPECT_FLOAT_EQ(moved.aspect_ratio(), 640.0f / 480.0f);
}

//==============================================================================
//                        Move Assignment (double-destroy regression guard)
//==============================================================================

TEST_F(WindowTests, MoveAssignmentNoDoubleDestroy)
{
    auto result1 = window::create("WindowTests.MoveAssignmentNoDoubleDestroy.1", 320, 240);
    SKIP_IF_NO_WINDOW(result1);

    auto result2 = window::create("WindowTests.MoveAssignmentNoDoubleDestroy.2", 800, 600);
    SKIP_IF_NO_WINDOW(result2);

    window w1(std::move(*result1));
    window w2(std::move(*result2));

    w1 = std::move(w2);
    w1.active();

    // Different sizes so this proves state actually transferred, not just that nothing
    // crashed. The still-implicit part: a clean return here lets both destructors run at
    // scope exit without a double glfwDestroyWindow() on the same GLFWwindow*.
    EXPECT_FLOAT_EQ(w1.aspect_ratio(), 800.0f / 600.0f);
}
