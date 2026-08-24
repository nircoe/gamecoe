#include <gtest/gtest.h>
#include <gamecoe/core/game.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
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

    struct marker { int value; };

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

TEST_F(GameTests, CreateEntityTagsScene)
{
    auto result = game::create("GameTests.CreateEntityTagsScene");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.load_scene(scene_a);
    g.activate_scene(scene_a);

    components::transform t;
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);

    entity e = g.create_entity(scene_a, marker{7}, t);

    ASSERT_TRUE(g.entities().valid(e));
    ASSERT_NE(g.entities().get_component<components::scene_tag>(e), nullptr);
    EXPECT_EQ(g.entities().get_component<components::scene_tag>(e)->id, scene_a);
    ASSERT_NE(g.entities().get_component<marker>(e), nullptr);
    EXPECT_EQ(g.entities().get_component<marker>(e)->value, 7);
    EXPECT_EQ(g.entities().transform(e).position, t.position);
    EXPECT_TRUE(g.entities().is_active(e));

    g.deactivate_scene(scene_a);
    EXPECT_FALSE(g.entities().is_active(e));

    g.activate_scene(scene_a);
    EXPECT_TRUE(g.entities().is_active(e));
}

TEST_F(GameTests, CreateEntityRequiresActiveScene)
{
#ifndef NDEBUG
    // Default fork-based death tests inherit any thread state game::create() already spun up
    // (GLFW/logcoe) - a held lock at fork time can deadlock the child. threadsafe re-execs fresh.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    auto result = game::create("GameTests.CreateEntityRequiresActiveScene");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.create_scene(scene_b, build_scene_b);

    EXPECT_DEATH(g.create_entity(static_cast<scene_id>(999)), "scene is not registered");

    g.load_scene(scene_a);
    EXPECT_DEATH(g.create_entity(scene_a), "scene is not active");

    g.load_scene(scene_b);
    g.activate_scene(scene_b);
    g.deactivate_scene(scene_b);
    EXPECT_DEATH(g.create_entity(scene_b), "scene is not active");
#endif
}

TEST_F(GameTests, MultipleActiveScenesShareOneRegistry)
{
    auto result = game::create("GameTests.MultipleActiveScenesShareOneRegistry");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.create_scene(scene_b, build_scene_b);
    g.load_scene(scene_a);
    g.activate_scene(scene_a);
    g.load_scene(scene_b);
    g.activate_scene(scene_b);

    std::size_t transform_count = 0;
    for (auto [e, t] : g.entities().extract<components::transform>())
        ++transform_count;
    EXPECT_EQ(transform_count, 4u);

    std::vector<entity> scene_a_entities, scene_b_entities;
    g.entities().for_each_all<components::scene_tag>(
        [&](entity e, const components::scene_tag &tag)
        {
            if (tag.id == scene_a) scene_a_entities.push_back(e);
            else if (tag.id == scene_b) scene_b_entities.push_back(e);
        });
    ASSERT_EQ(scene_a_entities.size(), 3u);
    ASSERT_EQ(scene_b_entities.size(), 1u);

    entity parent_in_a = scene_a_entities[0];
    entity child_in_b = scene_b_entities[0];

    g.entities().set_parent(child_in_b, parent_in_a);

    ASSERT_NE(g.entities().get_component<components::parent>(child_in_b), nullptr);
    EXPECT_EQ(g.entities().get_component<components::parent>(child_in_b)->handle, parent_in_a);
    ASSERT_NE(g.entities().get_component<components::children>(parent_in_a), nullptr);
    ASSERT_EQ(g.entities().get_component<components::children>(parent_in_a)->handles.size(), 1u);
    EXPECT_EQ(g.entities().get_component<components::children>(parent_in_a)->handles[0], child_in_b);

    g.deactivate_scene(scene_a);
    EXPECT_EQ(g.status(scene_b), scene_status::active);
    EXPECT_EQ(count_active_scene_entities(g, scene_b), 0u);
    EXPECT_FALSE(g.entities().is_active(child_in_b));
}

TEST_F(GameTests, CreateSceneDuplicateIdGuarded)
{
    auto result = game::create("GameTests.CreateSceneDuplicateIdGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.create_scene(scene_a, build_scene_b), "scene is already registered");
#else
    // Release: guard-return leaves the original registration untouched, second call is a no-op.
    g.create_scene(scene_a, build_scene_b);
    g.load_scene(scene_a);
    g.activate_scene(scene_a);
    EXPECT_EQ(count_scene_entities(g, scene_a), 3u);
#endif
}

TEST_F(GameTests, CreateSceneNullBuilderGuarded)
{
    auto result = game::create("GameTests.CreateSceneNullBuilderGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.create_scene(scene_a, nullptr), "scene builder is null");
#else
    // Release: guard-return means the scene was never registered.
    g.create_scene(scene_a, nullptr);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, LoadSceneUnregisteredGuarded)
{
    auto result = game::create("GameTests.LoadSceneUnregisteredGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.load_scene(scene_a), "scene is not registered");
#else
    // Release: guard-return, no entities flushed for an unregistered scene.
    g.load_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g.entities().size(), 0u);
#endif
}

TEST_F(GameTests, ActivateSceneUnregisteredGuarded)
{
    auto result = game::create("GameTests.ActivateSceneUnregisteredGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.activate_scene(scene_a), "scene is not registered");
#else
    // Release: guard-return, no insertion into m_active_scenes for an unregistered scene.
    g.activate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    EXPECT_TRUE(g.active_scenes().empty());
#endif
}

TEST_F(GameTests, DeactivateSceneUnregisteredGuarded)
{
    auto result = game::create("GameTests.DeactivateSceneUnregisteredGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.deactivate_scene(scene_a), "scene is not registered");
#else
    // Release: guard-return, no-op for an unregistered scene.
    g.deactivate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, UnloadSceneUnregisteredGuarded)
{
    auto result = game::create("GameTests.UnloadSceneUnregisteredGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.unload_scene(scene_a), "scene is not registered");
#else
    // Release: guard-return, no-op for an unregistered scene.
    g.unload_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, LoadSceneNotUnloadedGuarded)
{
    auto result = game::create("GameTests.LoadSceneNotUnloadedGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.load_scene(scene_a);

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.load_scene(scene_a), "scene is not unloaded");
#else
    // Release: guard-return, second load doesn't double-flush the pending command_buffer.
    g.load_scene(scene_a);
    g.activate_scene(scene_a);
    EXPECT_EQ(count_scene_entities(g, scene_a), 3u);
#endif
}

TEST_F(GameTests, ActivateSceneAlreadyActiveGuarded)
{
    auto result = game::create("GameTests.ActivateSceneAlreadyActiveGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.load_scene(scene_a);
    g.activate_scene(scene_a);

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.activate_scene(scene_a), "scene is not loaded or inactive");
#else
    // Release: guard-return, second activate doesn't duplicate the m_active_scenes entry.
    g.activate_scene(scene_a);
    ASSERT_EQ(g.active_scenes().size(), 1u);
    EXPECT_EQ(g.active_scenes()[0], scene_a);
#endif
}

TEST_F(GameTests, DeactivateSceneNotActiveGuarded)
{
    auto result = game::create("GameTests.DeactivateSceneNotActiveGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);
    g.load_scene(scene_a);

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.deactivate_scene(scene_a), "scene is not active");
#else
    // Release: guard-return, scene stays loaded, nothing to deactivate since it was never flushed.
    g.deactivate_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::loaded);
    EXPECT_EQ(g.entities().size(), 0u);
#endif
}

TEST_F(GameTests, UnloadSceneAlreadyUnloadedGuarded)
{
    auto result = game::create("GameTests.UnloadSceneAlreadyUnloadedGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

    g.create_scene(scene_a, build_scene_a);

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.unload_scene(scene_a), "scene is already unloaded");
#else
    // Release: guard-return, no-op on an already-unloaded scene.
    g.unload_scene(scene_a);
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g.entities().size(), 0u);
#endif
}

TEST_F(GameTests, StatusUnregisteredIdGuarded)
{
    auto result = game::create("GameTests.StatusUnregisteredIdGuarded");
    SKIP_IF_NO_GAME(result);
    game &g = *result;

#ifndef NDEBUG
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH(g.status(scene_a), "scene is not registered");
#else
    // Release: guard-return, unregistered id is a well-defined unloaded rather than UB.
    EXPECT_EQ(g.status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, MoveConstructorNoDoubleDestroy)
{
    auto result = game::create("GameTests.MoveConstructorNoDoubleDestroy", 320, 240, colorcoe::red());
    SKIP_IF_NO_GAME(result);

    game moved(std::move(*result));

    // Different from create()'s default background_color, so this proves state actually
    // transferred, not just that nothing crashed - both destructors run cleanly at scope
    // exit with no double glfwTerminate()/logcoe::shutdown()/soundcoe::shutdown().
    EXPECT_EQ(moved.background_color(), colorcoe::red());
}
