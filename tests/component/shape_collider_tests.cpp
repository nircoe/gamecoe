#include <gtest/gtest.h>
#include <gamecoe/component/shape_collider.hpp>

using namespace gamecoe;

//==============================================================================
//              ShapeColliderTests - shape_collider component tests
//==============================================================================

class ShapeColliderTests : public ::testing::Test
{
protected:
    components::shape_collider sc;
};

//==============================================================================
//                        Default State
//==============================================================================

TEST_F(ShapeColliderTests, DefaultState)
{
    // Test 1: Default-constructed shape_collider has invalid kind
    {
        EXPECT_EQ(sc.kind, shape::invalid);
    }
}

//==============================================================================
//                        Factory Functions
//==============================================================================

TEST_F(ShapeColliderTests, FactoryFunctions)
{
    // Test 1: triangle() produces the correct kind
    {
        auto s = components::shape_collider::triangle();
        EXPECT_EQ(s.kind, shape::triangle);
    }

    // Test 2: rectangle() produces the correct kind
    {
        auto s = components::shape_collider::rectangle();
        EXPECT_EQ(s.kind, shape::rectangle);
    }

    // Test 3: box() produces the correct kind
    {
        auto s = components::shape_collider::box();
        EXPECT_EQ(s.kind, shape::box);
    }

    // Test 4: circle() produces the correct kind
    {
        auto s = components::shape_collider::circle();
        EXPECT_EQ(s.kind, shape::circle);
    }

    // Test 5: sphere() produces the correct kind
    {
        auto s = components::shape_collider::sphere();
        EXPECT_EQ(s.kind, shape::sphere);
    }
}
