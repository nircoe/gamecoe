#include <gtest/gtest.h>
#include <gamecoe/component/scene_tag.hpp>

using namespace gamecoe;

namespace gamecoe
{
    enum class scene_id : std::uint16_t
    {
        TestSceneA = 1,
        TestSceneB = 2,
    };
} // namespace gamecoe

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
    // Test 1: Assigning TestSceneA and verifying the value
    {
        tag.id = scene_id::TestSceneA;
        EXPECT_EQ(tag.id, scene_id::TestSceneA);
        EXPECT_EQ(static_cast<std::uint16_t>(tag.id), 1);
    }

    // Test 2: Reassigning to TestSceneB and verifying the new value
    {
        tag.id = scene_id::TestSceneB;
        EXPECT_EQ(tag.id, scene_id::TestSceneB);
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
