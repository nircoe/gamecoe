#include <gtest/gtest.h>
#include <gamecoe/component/shape_renderer.hpp>
#include <colorcoe.hpp>

using namespace gamecoe;

//==============================================================================
//              ShapeRendererTests - shape_renderer component tests
//==============================================================================

class ShapeRendererTests : public ::testing::Test
{
protected:
    components::shape_renderer sr;
};

//==============================================================================
//                        Default State
//==============================================================================

TEST_F(ShapeRendererTests, DefaultState)
{
    // Test 1: Default-constructed shape_renderer has invalid kind and default (black) color
    {
        EXPECT_EQ(sr.kind, shape::invalid);
        EXPECT_EQ(sr.color, Color());
    }
}

//==============================================================================
//                        Factory Functions
//==============================================================================

TEST_F(ShapeRendererTests, FactoryFunctions)
{
    // Test 1: triangle() produces the correct kind and color
    {
        auto s = components::shape_renderer::triangle(colorcoe::red());
        EXPECT_EQ(s.kind, shape::triangle);
        EXPECT_EQ(s.color, colorcoe::red());
    }

    // Test 2: rectangle() produces the correct kind and color
    {
        auto s = components::shape_renderer::rectangle(colorcoe::green());
        EXPECT_EQ(s.kind, shape::rectangle);
        EXPECT_EQ(s.color, colorcoe::green());
    }

    // Test 3: box() produces the correct kind and color
    {
        auto s = components::shape_renderer::box(colorcoe::blue());
        EXPECT_EQ(s.kind, shape::box);
        EXPECT_EQ(s.color, colorcoe::blue());
    }

    // Test 4: circle() produces the correct kind and color
    {
        auto s = components::shape_renderer::circle(colorcoe::yellow());
        EXPECT_EQ(s.kind, shape::circle);
        EXPECT_EQ(s.color, colorcoe::yellow());
    }

    // Test 5: sphere() produces the correct kind and color
    {
        auto s = components::shape_renderer::sphere(colorcoe::white());
        EXPECT_EQ(s.kind, shape::sphere);
        EXPECT_EQ(s.color, colorcoe::white());
    }
}
