#include <gtest/gtest.h>
#include <gamecoe/core/game.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <support/scene_id.hpp>
#include <algorithm>
#include <vector>

using namespace gamecoe;

#define SKIP_IF_NO_GAME(result) \
    if (!(result).has_value()) \
    { \
        GTEST_SKIP() << "game::create() failed - no display/GL context available"; \
    }

namespace
{
    constexpr scene_id scene_a = scene_id::TestSceneA;
    constexpr scene_id scene_b = scene_id::TestSceneB;

    void build_scene_a(command_buffer &buf) { buf.spawn(); buf.spawn(); buf.spawn(); }
    void build_scene_b(command_buffer &buf) { buf.spawn(); }

    std::size_t count_scene_entities(game &g, scene_id id)
    {
        std::size_t count = 0;
        g.entities().for_each_all<components::scene_tag>(
            [id, &count](entity, const components::scene_tag &tag) { if (tag.id == id) ++count; });
        return count;
    }

    std::size_t count_active_scene_entities(game &g, scene_id id)
    {
        std::size_t count = 0;
        g.entities().for_each_all<components::scene_tag>(
            [&g, id, &count](entity e, const components::scene_tag &tag)
            {
                if (tag.id == id && g.entities().is_active(e)) ++count;
            });
        return count;
    }
} // namespace

class GameTests : public ::testing::Test { };

TEST_F(GameTests, SceneLifecycleTransitions)
{
    auto result = game::create("GameTests.SceneLifecycleTransitions");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    EXPECT_EQ(to_string(scene_a), "TestScene1");

    g.load_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::loaded);
    EXPECT_EQ(std::find(g.active_scenes().begin(), g.active_scenes().end(), scene_a), g.active_scenes().end());

    g.activate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::active);
    ASSERT_EQ(g.active_scenes().size(), 1u);
    EXPECT_EQ(g.active_scenes()[0], scene_a);

    g.deactivate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::inactive);
    EXPECT_EQ(std::find(g.active_scenes().begin(), g.active_scenes().end(), scene_a), g.active_scenes().end());

    g.activate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::active);
    EXPECT_NE(std::find(g.active_scenes().begin(), g.active_scenes().end(), scene_a), g.active_scenes().end());

    g.unload_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    EXPECT_EQ(std::find(g.active_scenes().begin(), g.active_scenes().end(), scene_a), g.active_scenes().end());
}

TEST_F(GameTests, LoadDefersFlushUntilActivate)
{
    auto result = game::create("GameTests.LoadDefersFlushUntilActivate");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.load_scene(scene_a);
    EXPECT_EQ(g.entities().size(), 0u);

    g.activate_scene(scene_a);
    EXPECT_EQ(g.entities().size(), 3u);
    EXPECT_EQ(count_scene_entities(g, scene_a), 3u);

    g.entities().for_each_all<components::scene_tag>(
        [&g](entity e, const components::scene_tag &tag)
        {
            EXPECT_EQ(tag.id, scene_a);
            EXPECT_TRUE(g.entities().is_active(e));
        });
}

TEST_F(GameTests, DeactivateReactivateMovesPartitions)
{
    {
        auto result = game::create("GameTests.DeactivateReactivateMovesPartitions.1");
        SKIP_IF_NO_GAME(result);
        game &g = *result;

        g.create_scene(scene_a, build_scene_a);
        g.load_scene(scene_a);
        g.activate_scene(scene_a);

        std::vector<entity> scene_entities;
        g.entities().for_each_all<components::scene_tag>(
            [&scene_entities](entity e, const components::scene_tag &tag)
            {
                if (tag.id == scene_a) scene_entities.push_back(e);
            });
        ASSERT_EQ(scene_entities.size(), 3u);

        g.deactivate_scene(scene_a);
        for (entity e : scene_entities)
        {
            EXPECT_TRUE(g.entities().valid(e));
            EXPECT_FALSE(g.entities().is_active(e));
        }
        EXPECT_EQ(count_active_scene_entities(g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities(g, scene_a), 3u);

        g.activate_scene(scene_a);
        EXPECT_EQ(count_active_scene_entities(g, scene_a), 3u);
    }

    {
        auto result = game::create("GameTests.DeactivateReactivateMovesPartitions.2");
        SKIP_IF_NO_GAME(result);
        game &g = *result;

        g.create_scene(scene_a, build_scene_a);
        g.load_scene(scene_a);
        g.activate_scene(scene_a);

        std::vector<entity> scene_entities;
        g.entities().for_each_all<components::scene_tag>(
            [&scene_entities](entity e, const components::scene_tag &tag)
            {
                if (tag.id == scene_a) scene_entities.push_back(e);
            });
        ASSERT_EQ(scene_entities.size(), 3u);

        entity individually_deactivated = scene_entities[0];
        g.entities().deactivate(individually_deactivated);

        g.deactivate_scene(scene_a);
        g.activate_scene(scene_a);

        // paused_active only restores entities that were active before deactivate_scene, not this one
        EXPECT_FALSE(g.entities().is_active(individually_deactivated));
        EXPECT_EQ(count_active_scene_entities(g, scene_a), 2u);
    }
}

TEST_F(GameTests, UnloadDestroysSceneEntities)
{
    {
        auto result = game::create("GameTests.UnloadDestroysSceneEntities.1");
        SKIP_IF_NO_GAME(result);
        game &g = *result;

        g.create_scene(scene_a, build_scene_a);
        g.create_scene(scene_b, build_scene_b);
        g.load_scene(scene_a);
        g.activate_scene(scene_a);
        g.load_scene(scene_b);
        g.activate_scene(scene_b);

        g.unload_scene(scene_a);
        EXPECT_EQ(count_scene_entities(g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities(g, scene_b), 1u);
        EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    }

    {
        auto result = game::create("GameTests.UnloadDestroysSceneEntities.2");
        SKIP_IF_NO_GAME(result);
        game &g = *result;

        g.create_scene(scene_a, build_scene_a);
        g.load_scene(scene_a);
        g.unload_scene(scene_a);
        EXPECT_EQ(count_scene_entities(g, scene_a), 0u);
        EXPECT_EQ(g.status(scene_a), scene_status::unloaded);

        g.load_scene(scene_a);
        g.activate_scene(scene_a);
        // 3, not 6: proves unload_scene() cleared the pending command_buffer, not double-queued it
        EXPECT_EQ(count_scene_entities(g, scene_a), 3u);
    }
}
