#include <gtest/gtest.h>
#include <gamecoe/core/game.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/component/transform.hpp>
#include <support/scene_id.hpp>
#include <support/test_utils.hpp>
#include <algorithm>
#include <optional>
#include <vector>

using namespace gamecoe;

#define SKIP_IF_NO_GAME(result) \
    SKIP_IF_NOT((result).has_value(), "game::create() failed - no display/GL context available")

#define EXPECT_SCENE_UNREGISTERED_DEATH(call) EXPECT_DEATH(call, "scene is not registered")

namespace
{
    constexpr scene_id scene_a = scene_id::TestScene1;
    constexpr scene_id scene_b = scene_id::TestScene2;

    void build_scene_a(command_buffer &buf) { buf.spawn(); buf.spawn(); buf.spawn(); }
    void build_scene_b(command_buffer &buf) { buf.spawn(); }

    struct marker { int value; };

    std::vector<entity> scene_entities(game &g, scene_id id)
    {
        std::vector<entity> result;
        g.entities().for_each_all<components::scene_tag>(
            [id, &result](entity e, const components::scene_tag &tag) { if (tag.id == id) result.push_back(e); });
        return result;
    }

    std::size_t count_scene_entities(game &g, scene_id id)
    {
        return scene_entities(g, id).size();
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

class GameTests : public ::testing::Test
{
protected:
    std::optional<game> g;

    void SetUp() override
    {
        auto result = game::create(::testing::UnitTest::GetInstance()->current_test_info()->name());
        SKIP_IF_NO_GAME(result);
        g.emplace(std::move(*result));
    }
};

TEST_F(GameTests, SceneLifecycleTransitions)
{
    g->create_scene(scene_a, build_scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(to_string(scene_a), "TestScene1");

    g->load_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::loaded);
    EXPECT_EQ(std::find(g->active_scenes().begin(), g->active_scenes().end(), scene_a), g->active_scenes().end());

    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::active);
    ASSERT_EQ(g->active_scenes().size(), 1u);
    EXPECT_EQ(g->active_scenes()[0], scene_a);

    g->deactivate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::inactive);
    EXPECT_EQ(std::find(g->active_scenes().begin(), g->active_scenes().end(), scene_a), g->active_scenes().end());

    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::active);
    EXPECT_NE(std::find(g->active_scenes().begin(), g->active_scenes().end(), scene_a), g->active_scenes().end());

    g->unload_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(std::find(g->active_scenes().begin(), g->active_scenes().end(), scene_a), g->active_scenes().end());
}

TEST_F(GameTests, LoadDefersFlushUntilActivate)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    EXPECT_EQ(g->entities().size(), 0u);

    g->activate_scene(scene_a);
    EXPECT_EQ(g->entities().size(), 3u);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);

    g->entities().for_each_all<components::scene_tag>(
        [this](entity e, const components::scene_tag &tag)
        {
            EXPECT_EQ(tag.id, scene_a);
            EXPECT_TRUE(g->entities().is_active(e));
        });
}

TEST_F(GameTests, DeactivateReactivateMovesPartitions)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = scene_entities(*g, scene_a);
    ASSERT_EQ(entities_a.size(), 3u);

    g->deactivate_scene(scene_a);
    for (entity e : entities_a)
    {
        EXPECT_TRUE(g->entities().valid(e));
        EXPECT_FALSE(g->entities().is_active(e));
    }
    EXPECT_EQ(count_active_scene_entities(*g, scene_a), 0u);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);

    g->activate_scene(scene_a);
    EXPECT_EQ(count_active_scene_entities(*g, scene_a), 3u);
}

TEST_F(GameTests, DeactivateReactivateSkipsIndividuallyDeactivated)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = scene_entities(*g, scene_a);
    ASSERT_EQ(entities_a.size(), 3u);

    entity individually_deactivated = entities_a[0];
    g->entities().deactivate(individually_deactivated);

    g->deactivate_scene(scene_a);
    g->activate_scene(scene_a);

    // paused_active only restores entities that were active before deactivate_scene, not this one
    EXPECT_FALSE(g->entities().is_active(individually_deactivated));
    EXPECT_EQ(count_active_scene_entities(*g, scene_a), 2u);
}

TEST_F(GameTests, UnloadDestroysSceneEntities)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);

    g->unload_scene(scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 0u);
    EXPECT_EQ(count_scene_entities(*g, scene_b), 1u);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
}

TEST_F(GameTests, UnloadThenReloadDoesNotDoubleQueue)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->unload_scene(scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 0u);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);

    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    // 3, not 6: proves unload_scene() cleared the pending command_buffer, not double-queued it
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
}

TEST_F(GameTests, CreateEntityTagsScene)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    components::transform t;
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);

    entity e = g->create_entity(scene_a, marker{7}, t);

    ASSERT_TRUE(g->entities().valid(e));
    ASSERT_NE(g->entities().get_component<components::scene_tag>(e), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(e)->id, scene_a);
    ASSERT_NE(g->entities().get_component<marker>(e), nullptr);
    EXPECT_EQ(g->entities().get_component<marker>(e)->value, 7);
    EXPECT_EQ(g->entities().transform(e).position, t.position);
    EXPECT_TRUE(g->entities().is_active(e));

    g->deactivate_scene(scene_a);
    EXPECT_FALSE(g->entities().is_active(e));

    g->activate_scene(scene_a);
    EXPECT_TRUE(g->entities().is_active(e));
}

TEST_F(GameTests, CreateEntityRequiresActiveScene)
{
#ifndef NDEBUG
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);

    EXPECT_DEATH(g->create_entity(static_cast<scene_id>(999)), "scene is not registered");

    g->load_scene(scene_a);
    EXPECT_DEATH(g->create_entity(scene_a), "scene is not active");

    g->load_scene(scene_b);
    g->activate_scene(scene_b);
    g->deactivate_scene(scene_b);
    EXPECT_DEATH(g->create_entity(scene_b), "scene is not active");
#endif
}

TEST_F(GameTests, MultipleActiveScenesShareOneRegistry)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);

    std::size_t transform_count = 0;
    for (auto [e, t] : g->entities().extract<components::transform>())
        ++transform_count;
    EXPECT_EQ(transform_count, 4u);

    ASSERT_EQ(scene_entities(*g, scene_a).size(), 3u);
    ASSERT_EQ(scene_entities(*g, scene_b).size(), 1u);
}

TEST_F(GameTests, CreateSceneDuplicateIdGuarded)
{
    g->create_scene(scene_a, build_scene_a);

#ifndef NDEBUG
    EXPECT_DEATH(g->create_scene(scene_a, build_scene_b), "scene is already registered");
#else
    // Release: guard-return leaves the original registration untouched, second call is a no-op.
    g->create_scene(scene_a, build_scene_b);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
#endif
}

TEST_F(GameTests, CreateSceneNullBuilderGuarded)
{
#ifndef NDEBUG
    EXPECT_DEATH(g->create_scene(scene_a, nullptr), "scene builder is null");
#else
    // Release: guard-return means the scene was never registered.
    g->create_scene(scene_a, nullptr);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, LoadSceneUnregisteredGuarded)
{
#ifndef NDEBUG
    EXPECT_SCENE_UNREGISTERED_DEATH(g->load_scene(scene_a));
#else
    // Release: guard-return, no entities flushed for an unregistered scene.
    g->load_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g->entities().size(), 0u);
#endif
}

TEST_F(GameTests, ActivateSceneUnregisteredGuarded)
{
#ifndef NDEBUG
    EXPECT_SCENE_UNREGISTERED_DEATH(g->activate_scene(scene_a));
#else
    // Release: guard-return, no insertion into m_active_scenes for an unregistered scene.
    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_TRUE(g->active_scenes().empty());
#endif
}

TEST_F(GameTests, DeactivateSceneUnregisteredGuarded)
{
#ifndef NDEBUG
    EXPECT_SCENE_UNREGISTERED_DEATH(g->deactivate_scene(scene_a));
#else
    // Release: guard-return, no-op for an unregistered scene.
    g->deactivate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, UnloadSceneUnregisteredGuarded)
{
#ifndef NDEBUG
    EXPECT_SCENE_UNREGISTERED_DEATH(g->unload_scene(scene_a));
#else
    // Release: guard-return, no-op for an unregistered scene.
    g->unload_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
}

TEST_F(GameTests, LoadSceneNotUnloadedGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);

#ifndef NDEBUG
    EXPECT_DEATH(g->load_scene(scene_a), "scene is not unloaded");
#else
    // Release: guard-return, second load doesn't double-flush the pending command_buffer.
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
#endif
}

TEST_F(GameTests, ActivateSceneAlreadyActiveGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

#ifndef NDEBUG
    EXPECT_DEATH(g->activate_scene(scene_a), "scene is not loaded or inactive");
#else
    // Release: guard-return, second activate doesn't duplicate the m_active_scenes entry.
    g->activate_scene(scene_a);
    ASSERT_EQ(g->active_scenes().size(), 1u);
    EXPECT_EQ(g->active_scenes()[0], scene_a);
#endif
}

TEST_F(GameTests, DeactivateSceneNotActiveGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);

#ifndef NDEBUG
    EXPECT_DEATH(g->deactivate_scene(scene_a), "scene is not active");
#else
    // Release: guard-return, scene stays loaded, nothing to deactivate since it was never flushed.
    g->deactivate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::loaded);
    EXPECT_EQ(g->entities().size(), 0u);
#endif
}

TEST_F(GameTests, UnloadSceneAlreadyUnloadedGuarded)
{
    g->create_scene(scene_a, build_scene_a);

#ifndef NDEBUG
    EXPECT_DEATH(g->unload_scene(scene_a), "scene is already unloaded");
#else
    // Release: guard-return, no-op on an already-unloaded scene.
    g->unload_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g->entities().size(), 0u);
#endif
}

TEST_F(GameTests, StatusUnregisteredIdGuarded)
{
#ifndef NDEBUG
    EXPECT_SCENE_UNREGISTERED_DEATH(g->status(scene_a));
#else
    // Release: guard-return, unregistered id is a well-defined unloaded rather than UB.
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
}

TEST(GameMoveTests, MoveConstructorNoDoubleDestroy)
{
    auto result = game::create("GameMoveTests.MoveConstructorNoDoubleDestroy", 320, 240, colorcoe::red());
    SKIP_IF_NO_GAME(result);

    game moved(std::move(*result));

    // Different from create()'s default background_color, so this proves state actually
    // transferred, not just that nothing crashed - both destructors run cleanly at scope
    // exit with no double glfwTerminate()/logcoe::shutdown()/soundcoe::shutdown().
    EXPECT_EQ(moved.background_color(), colorcoe::red());
}
