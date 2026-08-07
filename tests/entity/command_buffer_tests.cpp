#include <gtest/gtest.h>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include "../test_utils.hpp"

using namespace gamecoe;
using namespace test_utils;

//==============================================================================
//                    Test Component Types
//==============================================================================

struct Tag
{
    int value;
};

struct follow_target
{
    entity target;
};

//==============================================================================
//                    CommandBufferTests - Command buffer tests
//==============================================================================

class CommandBufferTests : public ::testing::Test
{
protected:
    entities mgr;
    command_buffer buf;
};

//==============================================================================
//                        Spawn And Flush
//==============================================================================

TEST_F(CommandBufferTests, SpawnAndFlush)
{
    // Test 1: nothing exists until flush
    {
        buf.spawn();
        EXPECT_EQ(mgr.size(), 0);
        buf.clear();
    }

    // Test 2: spawn with explicit transform flushes to an entity with that transform
    {
        components::transform t;
        t.position = glm::vec3(1.0f, 2.0f, 3.0f);
        t.rotation = glm::angleAxis(0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        t.scale = glm::vec3(2.0f, 2.0f, 2.0f);

        buf.spawn(t);
        buf.flush(mgr);

        ASSERT_EQ(mgr.size(), 1);
        entity e = entity::invalid();
        mgr.for_each<components::transform>([&e]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            e = ent;
        });

        expect_vec3_near(mgr.transform(e).position, t.position);
        expect_quat_near(mgr.transform(e).rotation, t.rotation);
        expect_vec3_near(mgr.transform(e).scale, t.scale);
    }

    // Test 3: spawn with no args flushes to a default-constructed transform
    {
        mgr.clear();
        buf.spawn();
        buf.flush(mgr);

        ASSERT_EQ(mgr.size(), 1);
        entity e = entity::invalid();
        mgr.for_each<components::transform>([&e]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            e = ent;
        });

        expect_vec3_near(mgr.transform(e).position, glm::vec3(0.0f));
        expect_quat_near(mgr.transform(e).rotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
        expect_vec3_near(mgr.transform(e).scale, glm::vec3(1.0f));
    }

    // Test 4: plain add<T> roundtrips a local test component onto the flushed entity
    {
        mgr.clear();
        command_buffer::placeholder p = buf.spawn();
        buf.add<Tag>(p, Tag{42});
        buf.flush(mgr);

        ASSERT_EQ(mgr.size(), 1);
        entity e = entity::invalid();
        mgr.for_each<components::transform>([&e]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            e = ent;
        });

        ASSERT_TRUE(mgr.has_component<Tag>(e));
        EXPECT_EQ(mgr.get_component<Tag>(e)->value, 42);
    }

    // Test 5: buffer is empty after flush; a second flush creates nothing new
    {
        mgr.clear();
        buf.spawn();
        buf.flush(mgr);

        EXPECT_TRUE(buf.empty());
        EXPECT_EQ(mgr.size(), 1);

        buf.flush(mgr);
        EXPECT_EQ(mgr.size(), 1);
    }
}

//==============================================================================
//                        Scene Tagging
//==============================================================================

TEST_F(CommandBufferTests, SceneTagging)
{
    // Test 1: flush with a scene stamps scene_tag on the created entity
    {
        buf.spawn();
        buf.flush(mgr, scene_id{1});

        ASSERT_EQ(mgr.size(), 1);
        entity e = entity::invalid();
        mgr.for_each<components::transform>([&e]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            e = ent;
        });

        ASSERT_TRUE(mgr.has_component<components::scene_tag>(e));
        EXPECT_EQ(mgr.get_component<components::scene_tag>(e)->id, scene_id{1});
    }

    // Test 2: flush with no scene leaves the entity untagged - default is global
    {
        mgr.clear();
        buf.spawn();
        buf.flush(mgr);

        ASSERT_EQ(mgr.size(), 1);
        entity e = entity::invalid();
        mgr.for_each<components::transform>([&e]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            e = ent;
        });

        EXPECT_FALSE(mgr.has_component<components::scene_tag>(e));
    }
}

//==============================================================================
//                        Callable Resolution
//==============================================================================

TEST_F(CommandBufferTests, CallableResolution)
{
    // Test 1: callable add<T> resolves a placeholder to a real entity reference
    {
        mgr.clear();
        command_buffer::placeholder p_a = buf.spawn();
        command_buffer::placeholder p_b = buf.spawn();
        buf.add(p_a, [=](const command_buffer::resolver& r) { return follow_target{r.resolve(p_b)}; });
        buf.flush(mgr);

        ASSERT_EQ(mgr.size(), 2);
        std::vector<entity> entities_list;
        mgr.for_each<components::transform>([&entities_list]([[maybe_unused]] entity ent, [[maybe_unused]] const components::transform &tr)
        {
            entities_list.push_back(ent);
        });

        // Identify entity_a (has follow_target) and entity_b (doesn't)
        entity entity_a = entity::invalid();
        entity entity_b = entity::invalid();
        for (entity e : entities_list)
        {
            if (mgr.has_component<follow_target>(e))
                entity_a = e;
            else
                entity_b = e;
        }

        ASSERT_TRUE(mgr.has_component<follow_target>(entity_a));
        EXPECT_EQ(mgr.get_component<follow_target>(entity_a)->target, entity_b);
        EXPECT_NE(mgr.get_component<follow_target>(entity_a)->target, entity::invalid());
        
        // buf.add<components::parent>(p_a, components::parent{}); // compile error: hierarchy components are managed
    }
}
