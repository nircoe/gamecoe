#include <gtest/gtest.h>
#include <gamecoe/component/scene_tag.hpp>
#include <support/scene_id.hpp>

using namespace gamecoe;

//==============================================================================
//                   SceneTagTests - scene_tag component tests
//==============================================================================

class SceneTagTests : public ::testing::Test
{
protected:
    components::scene_tag tag;
};

//==============================================================================
//                        Default State
//==============================================================================

TEST_F(SceneTagTests, DefaultState)
{
    // Test 1: Default-constructed scene_tag holds default-constructed scene_id
    {
        EXPECT_EQ(tag.id, scene_id{});
        EXPECT_EQ(static_cast<std::uint16_t>(tag.id), 0);
    }
}

//==============================================================================
//                      Assigning Scene IDs
//==============================================================================

TEST_F(SceneTagTests, AssignSceneId)
{
    // Test 1: Assigning TestScene1 and verifying the value
    {
        tag.id = scene_id::TestScene1;
        EXPECT_EQ(tag.id, scene_id::TestScene1);
        EXPECT_EQ(static_cast<std::uint16_t>(tag.id), 1);
    }

    // Test 2: Reassigning to TestScene2 and verifying the new value
    {
        tag.id = scene_id::TestScene2;
        EXPECT_EQ(tag.id, scene_id::TestScene2);
        EXPECT_EQ(static_cast<std::uint16_t>(tag.id), 2);
    }
}

//==============================================================================
//                   Raw Value Construction
//==============================================================================

TEST_F(SceneTagTests, RawValueConstruction)
{
    // Test 1: Constructing scene_tag with a raw scene_id value (no enumerator)
    {
        components::scene_tag other{scene_id{42}};
        EXPECT_EQ(other.id, scene_id{42});
        EXPECT_EQ(static_cast<std::uint16_t>(other.id), 42);
    }
}
