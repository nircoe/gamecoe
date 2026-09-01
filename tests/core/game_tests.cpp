#include <gtest/gtest.h>
#include <gamecoe/core/game.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <support/scene_id.hpp>
#include <support/test_utils.hpp>
#include <optional>
#include <vector>

using namespace gamecoe;

namespace gamecoe
{
    void test_prepare_to_play(game& g) { g.prepare_to_play(); }
} // namespace gamecoe

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

    void build_scene_hierarchy(command_buffer &buf)
    {
        command_buffer::placeholder parent = buf.spawn();
        command_buffer::placeholder child = buf.spawn();
        buf.add<marker>(child, marker{5});
        buf.set_parent(child, parent);
    }

    std::size_t count_scene_entities(game &g, scene_id id)
    {
        return g.scene_entities(id).size();
    }

    std::size_t count_active_scene_entities(game &g, scene_id id)
    {
        std::size_t count = 0;
        g.entities().for_each<components::scene_tag>(
            [id, &count](entity, const components::scene_tag &tag) { if (tag.id == id) ++count; });
        return count;
    }
} // namespace

class GameTests : public ::testing::Test
{
protected:
    std::optional<game> g;

    void SetUp() override
    {
        test_utils::init_headless_gl();
        auto result = game::create(::testing::UnitTest::GetInstance()->current_test_info()->name());
        SKIP_IF_NO_GAME(result);
        g.emplace(std::move(*result));
    }
};

TEST_F(GameTests, PrePlayCallsAreDeferredUntilPrepareToPlay)
{
    g->create_scene(scene_a, build_scene_a);

    // Pre-play: queued, not executed yet.
    g->load_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g->entities().size(), 0u);

    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(g->entities().size(), 0u);

    // Draining replays both queued ops in order.
    test_prepare_to_play(*g);
    EXPECT_EQ(g->status(scene_a), scene_status::active);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
}

TEST_F(GameTests, CreateSceneDuringPlayIsGuarded)
{
    test_prepare_to_play(*g);

#ifndef NDEBUG
    EXPECT_DEATH(g->create_scene(scene_a, build_scene_a), "cannot be created during game::play");
#else
    // Release: guard-return, the play-phase lock is what makes m_scenes storage-stable for the
    // rest of play() - this is the actual mechanism the three original UAF sites relied on.
    g->create_scene(scene_a, build_scene_a);
    EXPECT_FALSE(g->has_scene(scene_a));
#endif
}

TEST_F(GameTests, SceneLifecycleTransitions)
{
    g->create_scene(scene_a, build_scene_a);
    test_prepare_to_play(*g);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
    EXPECT_EQ(to_string(scene_a), "TestScene1");

    g->load_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::loaded);

    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::active);

    g->deactivate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::inactive);

    g->activate_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::active);

    g->unload_scene(scene_a);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
}

TEST_F(GameTests, LoadDefersFlushUntilActivate)
{
    g->create_scene(scene_a, build_scene_a);
    test_prepare_to_play(*g);
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
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = g->scene_entities(scene_a);
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
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = g->scene_entities(scene_a);
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
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);

    g->unload_scene(scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 0u);
    EXPECT_EQ(count_scene_entities(*g, scene_b), 1u);
    EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
}

TEST_F(GameTests, UnloadDestroysParentedSceneEntities)
{
    g->create_scene(scene_a, build_scene_a);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = g->scene_entities(scene_a);
    ASSERT_EQ(entities_a.size(), 3u);
    g->entities().set_parent(entities_a[1], entities_a[0]);

    // A parented child gets destroyed via its parent's cascade before unload_scene()'s own
    // loop reaches it - the count must still land on 3, not crash on the already-dead handle.
    g->unload_scene(scene_a);

    EXPECT_EQ(count_scene_entities(*g, scene_a), 0u);
    for (entity e : entities_a)
        EXPECT_FALSE(g->entities().valid(e));
}

TEST_F(GameTests, ActivateSceneFlushesHierarchyAndNonTransformComponents)
{
    g->create_scene(scene_a, build_scene_hierarchy);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    std::vector<entity> entities_a = g->scene_entities(scene_a);
    ASSERT_EQ(entities_a.size(), 2u);

    entity real_parent = entity::invalid();
    entity real_child = entity::invalid();
    for (entity e : entities_a)
        (g->entities().has_component<marker>(e) ? real_child : real_parent) = e;

    ASSERT_NE(real_parent, entity::invalid());
    ASSERT_NE(real_child, entity::invalid());
    ASSERT_TRUE(g->entities().has_component<components::parent>(real_child));
    EXPECT_EQ(g->entities().get_component<components::parent>(real_child)->handle, real_parent);
    EXPECT_EQ(g->entities().get_component<marker>(real_child)->value, 5);

    g->deactivate_scene(scene_a);
    EXPECT_FALSE(g->entities().is_active(real_parent));
    EXPECT_FALSE(g->entities().is_active(real_child));

    g->activate_scene(scene_a);
    EXPECT_TRUE(g->entities().is_active(real_parent));
    EXPECT_TRUE(g->entities().is_active(real_child));
}

TEST_F(GameTests, UnloadThenReloadDoesNotDoubleQueue)
{
    g->create_scene(scene_a, build_scene_a);
    test_prepare_to_play(*g);
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
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);

    components::transform t;
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);

    entity e = g->create_entity(scene_a, t, marker{7});

    ASSERT_TRUE(g->entities().valid(e));
    ASSERT_NE(g->entities().get_component<components::scene_tag>(e), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(e)->id, scene_a);
    ASSERT_NE(g->entities().get_component<marker>(e), nullptr);
    EXPECT_EQ(g->entities().get_component<marker>(e)->value, 7);
    EXPECT_EQ(g->entities().transform(e)->position, t.position);
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
    test_prepare_to_play(*g);

    EXPECT_DEATH(g->create_entity(static_cast<scene_id>(999)), "scene is not registered");

    g->load_scene(scene_a);
    EXPECT_DEATH(g->create_entity(scene_a), "scene is not active");

    g->load_scene(scene_b);
    g->activate_scene(scene_b);
    g->deactivate_scene(scene_b);
    EXPECT_DEATH(g->create_entity(scene_b), "scene is not active");
#endif

    // create_entity's Comps... pack rejects a transform passed through it (compile-time guard)
    // Uncommenting the line below must fail to compile:
    // g->create_entity(scene_a, components::transform{}, components::transform{});
}

TEST_F(GameTests, MultipleActiveScenesShareOneRegistry)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);

    std::size_t transform_count = 0;
    for ([[maybe_unused]] auto [e, t] : g->entities().extract<components::transform>())
        ++transform_count;
    EXPECT_EQ(transform_count, 4u);

    ASSERT_EQ(g->scene_entities(scene_a).size(), 3u);
    ASSERT_EQ(g->scene_entities(scene_b).size(), 1u);
}

TEST_F(GameTests, HasSceneAndWindow)
{
    EXPECT_FALSE(g->has_scene(scene_a));
    g->create_scene(scene_a, build_scene_a);
    EXPECT_TRUE(g->has_scene(scene_a));

    EXPECT_NE(g->window(), nullptr);
}

TEST_F(GameTests, SceneLayerRoundTripAndClamp)
{
    g->create_scene(scene_a, build_scene_a, 5);
    EXPECT_EQ(g->scene_layer(scene_a), 5);

    g->set_scene_layer(scene_a, -10);
    EXPECT_EQ(g->scene_layer(scene_a), -10);

    // Out of std::int8_t range [-128, 127] - clamped rather than silently truncated.
    g->set_scene_layer(scene_a, 500);
    EXPECT_EQ(g->scene_layer(scene_a), 127);

    g->create_scene(scene_b, build_scene_b, -500);
    EXPECT_EQ(g->scene_layer(scene_b), -128);
}

TEST_F(GameTests, CreateSceneGuarded)
{
    // Test 1: create_scene with a null builder
    {
#ifndef NDEBUG
        EXPECT_DEATH(g->create_scene(scene_b, nullptr), "scene builder is null");
#else
        // Release: guard-return means the scene was never registered.
        g->create_scene(scene_b, nullptr);
        EXPECT_EQ(g->status(scene_b), scene_status::unloaded);
#endif
    }

    // Test 2: create_scene with a duplicate (already registered) scene id
    {
        g->create_scene(scene_a, build_scene_a);

#ifndef NDEBUG
        EXPECT_DEATH(g->create_scene(scene_a, build_scene_b), "scene is already registered");
#else
        // Release: guard-return leaves the original registration untouched, second call is a no-op.
        g->create_scene(scene_a, build_scene_b);
        test_prepare_to_play(*g);
        g->load_scene(scene_a);
        g->activate_scene(scene_a);
        EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
#endif
    }
}

TEST_F(GameTests, UnregisteredSceneGuarded)
{
    // Test 1: load_scene on an unregistered scene
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

    // Test 2: activate_scene on an unregistered scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->activate_scene(scene_a));
#else
        // Release: guard-return, no insertion into m_active_scenes for an unregistered scene.
        g->activate_scene(scene_a);
        EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
    }

    // Test 3: deactivate_scene on an unregistered scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->deactivate_scene(scene_a));
#else
        // Release: guard-return, no-op for an unregistered scene.
        g->deactivate_scene(scene_a);
        EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
    }

    // Test 4: unload_scene on an unregistered scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->unload_scene(scene_a));
#else
        // Release: guard-return, no-op for an unregistered scene.
        g->unload_scene(scene_a);
        EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
    }

    // Test 5: status on an unregistered scene id
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->status(scene_a));
#else
        // Release: guard-return, unregistered id is a well-defined unloaded rather than UB.
        EXPECT_EQ(g->status(scene_a), scene_status::unloaded);
#endif
    }
}

TEST_F(GameTests, RepeatedLoadOrActivateGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    test_prepare_to_play(*g);

    // Test 1: load_scene called again while already loaded
    {
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

    // Test 2: activate_scene called again while already active
    {
        g->load_scene(scene_b);
        g->activate_scene(scene_b);

#ifndef NDEBUG
        EXPECT_DEATH(g->activate_scene(scene_b), "scene is not loaded or inactive");
#else
        // Release: guard-return, second activate doesn't duplicate the m_active_scenes entry.
        g->activate_scene(scene_b);
        EXPECT_EQ(g->status(scene_b), scene_status::active);
#endif
    }
}

TEST_F(GameTests, InvalidDeactivateOrUnloadGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    test_prepare_to_play(*g);

    // Test 1: deactivate_scene on a scene that's only loaded, not active
    {
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

    // Test 2: unload_scene on an already-unloaded scene
    {
#ifndef NDEBUG
        EXPECT_DEATH(g->unload_scene(scene_b), "scene is already unloaded");
#else
        // Release: guard-return, no-op on an already-unloaded scene.
        g->unload_scene(scene_b);
        EXPECT_EQ(g->status(scene_b), scene_status::unloaded);
        EXPECT_EQ(g->entities().size(), 0u);
#endif
    }
}

TEST_F(GameTests, SecondCreateFailsWhileFirstAlive)
{
#ifndef NDEBUG
    EXPECT_DEATH((void)game::create("SecondCreateFailsWhileFirstAlive.second"), "a game instance is already alive");
#else
    auto result2 = game::create("SecondCreateFailsWhileFirstAlive.second");
    ASSERT_FALSE(result2.has_value());
    EXPECT_EQ(result2.error().code, error_code::game_already_alive);
#endif
}

TEST(GameMoveTests, MoveConstructorNoDoubleDestroy)
{
    test_utils::init_headless_gl();
    auto result = game::create("GameMoveTests.MoveConstructorNoDoubleDestroy", 320, 240, colorcoe::red());
    SKIP_IF_NO_GAME(result);

    game moved(std::move(*result));

    // Different from create()'s default background_color, so this proves state actually
    // transferred, not just that nothing crashed - both destructors run cleanly at scope
    // exit with no double glfwTerminate()/logcoe::shutdown()/soundcoe::shutdown().
    EXPECT_EQ(moved.background_color(), colorcoe::red());
}

TEST(GameMoveTests, MovedFromGameGuardsAgainstUse)
{
    test_utils::init_headless_gl();
    auto result = game::create("GameMoveTests.MovedFromGameGuardsAgainstUse");
    SKIP_IF_NO_GAME(result);

    game moved(std::move(*result));
    game &source = *result;

    EXPECT_EQ(source.window(), nullptr);

#ifndef NDEBUG
    EXPECT_DEATH(source.set_background_color(colorcoe::blue()), "called on a moved-from game");
    EXPECT_DEATH(source.play(), "called on a moved-from game");
#else
    source.set_background_color(colorcoe::blue());
    source.play();
#endif
}
