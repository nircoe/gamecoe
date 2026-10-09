#include <gtest/gtest.h>
#include <gamecoe/core/game.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <support/scene_id.hpp>
#include <support/test_utils.hpp>
#include <algorithm>
#include <optional>
#include <utility>
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
    constexpr scene_id scene_c = scene_id::TestScene3;

    void build_scene_a(command_buffer &buf) { buf.spawn(); buf.spawn(); buf.spawn(); }
    void build_scene_b(command_buffer &buf) { buf.spawn(); }
    void build_scene_c(command_buffer &buf) { buf.spawn(); buf.spawn(); }

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

    std::size_t count_scene_entities_including_frozen(game &g, scene_id id)
    {
        std::size_t count = 0;
        for ([[maybe_unused]] auto [e, tag] : g.entities().extract_with_frozen<components::scene_tag>())
            if (tag.id == id) ++count;
        return count;
    }

    std::pair<entity, entity> setup_two_scenes(game &g)
    {
        g.create_scene(scene_a, build_scene_a);
        g.create_scene(scene_b, build_scene_b);
        test_prepare_to_play(g);
        g.load_scene(scene_a);
        g.activate_scene(scene_a);
        g.load_scene(scene_b);
        g.activate_scene(scene_b);
        return { g.scene_entities(scene_a)[0], g.scene_entities(scene_b)[0] };
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

    // activate_scene() only lifts the scene pause. This entity's own deactivate() still holds.
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

TEST_F(GameTests, UnloadSparesEntitiesAdoptedIntoAnotherScene)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);

    std::vector<entity> entities_a = g->scene_entities(scene_a);
    ASSERT_EQ(entities_a.size(), 3u);
    entity a0 = entities_a[0], a1 = entities_a[1], a2 = entities_a[2];
    entity b0 = g->scene_entities(scene_b)[0];

    g->entities().set_parent(a0, b0);

    entity C = g->create_entity(scene_b);
    g->entities().set_parent(C, a1);

    g->unload_scene(scene_a);

    // a0 was moved into scene B, so unloading scene A must not destroy it.
    EXPECT_TRUE(g->entities().valid(a0));
    EXPECT_TRUE(g->entities().is_active(a0));
    ASSERT_NE(g->entities().get_component<components::scene_tag>(a0), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(a0)->id, scene_b);

    ASSERT_TRUE(g->entities().valid(b0));
    const components::children *b0_children = g->entities().get_component<components::children>(b0);
    ASSERT_NE(b0_children, nullptr);
    EXPECT_NE(std::find(b0_children->handles.begin(), b0_children->handles.end(), a0), b0_children->handles.end());

    EXPECT_FALSE(g->entities().valid(a1));
    EXPECT_FALSE(g->entities().valid(a2));
    EXPECT_FALSE(g->entities().valid(C));

    EXPECT_EQ(g->status(scene_b), scene_status::active);
    EXPECT_EQ(count_scene_entities(*g, scene_b), 2u);
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
    ASSERT_NE(g->entities().get_component<components::scene_tag>(real_parent), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(real_parent)->id, scene_a);
    ASSERT_NE(g->entities().get_component<components::scene_tag>(real_child), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(real_child)->id, scene_a);

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

TEST_F(GameTests, SetParentAcrossScenesAdoptsIntoParentScene)
{
    auto [P, C] = setup_two_scenes(*g);
    g->entities().set_parent(C, P);

    // C now carries scene A's tag, since it was reparented under P.
    ASSERT_NE(g->entities().get_component<components::scene_tag>(C), nullptr);
    EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
    EXPECT_EQ(count_scene_entities(*g, scene_a), 4u);
    EXPECT_EQ(count_scene_entities(*g, scene_b), 0u);

    g->deactivate_scene(scene_a);
    EXPECT_FALSE(g->entities().is_active(P));
    EXPECT_FALSE(g->entities().is_active(C));

    // C no longer carries scene B's tag, so deactivating B has no effect on it.
    g->deactivate_scene(scene_b);

    g->activate_scene(scene_a);
    EXPECT_TRUE(g->entities().is_active(P));
    EXPECT_TRUE(g->entities().is_active(C));
    EXPECT_EQ(g->status(scene_b), scene_status::inactive);

    g->activate_scene(scene_b);
    g->deactivate_scene(scene_b);
    // C belongs to scene A now, so deactivating scene B must not touch it.
    EXPECT_TRUE(g->entities().is_active(C));
}

TEST_F(GameTests, AdoptedOutOfPausedSceneLeavesOldSceneBehind)
{
    auto [P, C] = setup_two_scenes(*g);

    // Test 1: adopted out of a paused scene, the entity follows its new scene
    {
        g->deactivate_scene(scene_b);
        g->entities().set_parent(C, P);
        ASSERT_NE(g->entities().get_component<components::scene_tag>(C), nullptr);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
        EXPECT_TRUE(g->entities().is_active(C));
        EXPECT_EQ(count_scene_entities(*g, scene_a), 4u);
        EXPECT_EQ(count_scene_entities(*g, scene_b), 0u);

        g->activate_scene(scene_b);
        EXPECT_EQ(g->status(scene_b), scene_status::active);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
        EXPECT_TRUE(g->entities().is_active(C));

        g->deactivate_scene(scene_a);
        EXPECT_FALSE(g->entities().is_active(C));
        g->activate_scene(scene_a);
        EXPECT_TRUE(g->entities().is_active(C));
    }

    // Test 2: unloading the old paused scene leaves the adopted entity alone
    {
        g->deactivate_scene(scene_b);
        g->unload_scene(scene_b);
        EXPECT_TRUE(g->entities().valid(C));
        EXPECT_TRUE(g->entities().is_active(C));
        ASSERT_NE(g->entities().get_component<components::scene_tag>(C), nullptr);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
        EXPECT_EQ(g->status(scene_b), scene_status::unloaded);

        g->deactivate_scene(scene_a);
        EXPECT_FALSE(g->entities().is_active(C));
        g->activate_scene(scene_a);
        EXPECT_TRUE(g->entities().is_active(C));
    }

    // Test 3: reloading a scene that was unloaded while paused starts unpaused
    {
        g->load_scene(scene_b);
        g->activate_scene(scene_b);
        EXPECT_EQ(count_active_scene_entities(*g, scene_b), 1u);
        entity C2 = g->scene_entities(scene_b)[0];
        EXPECT_TRUE(g->entities().is_active(C2));

        g->deactivate_scene(scene_b);
        EXPECT_FALSE(g->entities().is_active(C2));
        g->activate_scene(scene_b);
        EXPECT_TRUE(g->entities().is_active(C2));
    }
}

TEST_F(GameTests, AdoptedIntoPausedSceneWaitsForResume)
{
    auto [P, C] = setup_two_scenes(*g);

    // Test 1: an entity adopted into a paused scene stays inactive until the scene resumes
    {
        g->deactivate_scene(scene_a);
        g->entities().set_parent(C, P);
        ASSERT_NE(g->entities().get_component<components::scene_tag>(C), nullptr);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
        EXPECT_FALSE(g->entities().is_active(C));
        EXPECT_EQ(count_scene_entities(*g, scene_b), 0u);

        g->entities().remove_parent(C);
        EXPECT_FALSE(g->entities().is_active(C));

        g->activate_scene(scene_a);
        EXPECT_TRUE(g->entities().is_active(C));
    }
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

#ifndef NDEBUG
TEST_F(GameTests, SceneEntitiesUnregisteredSceneIsGuarded)
{
    EXPECT_DEATH(g->scene_entities(scene_a), "game::scene_entities\\(\\): scene is not registered");
}
#else
TEST_F(GameTests, SceneEntitiesUnregisteredSceneReturnsEmpty)
{
    EXPECT_TRUE(g->scene_entities(scene_a).empty());
}
#endif

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

TEST_F(GameTests, FreezeSceneBasics)
{
    setup_two_scenes(*g);
    std::vector<entity> entities_a = g->scene_entities(scene_a);

    // Test 1: a frozen scene stays active but is hidden from extract() and for_each()
    {
        g->freeze_scene(scene_a);
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->is_scene_frozen(scene_b));
        EXPECT_EQ(g->status(scene_a), scene_status::active);
        for (entity e : entities_a)
            EXPECT_TRUE(g->entities().is_active(e));
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities_including_frozen(*g, scene_a), 3u);
        EXPECT_EQ(count_scene_entities(*g, scene_a), 3u);
        EXPECT_EQ(count_active_scene_entities(*g, scene_b), 1u);
    }

    // Test 2: freezing or unfreezing twice is a no-op
    {
        g->freeze_scene(scene_a);
        EXPECT_TRUE(g->is_scene_frozen(scene_a));

        g->unfreeze_scene(scene_a);
        g->unfreeze_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
    }

    // Test 3: unfreezing makes the scene visible again
    {
        g->freeze_scene(scene_a);
        g->unfreeze_scene(scene_a);
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 3u);
    }

    // Test 4: an entity created into a frozen scene is frozen at once
    {
        g->freeze_scene(scene_a);
        entity e = g->create_entity(scene_a);
        EXPECT_TRUE(g->entities().valid(e));
        EXPECT_TRUE(g->entities().is_active(e));
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities_including_frozen(*g, scene_a), 4u);

        g->unfreeze_scene(scene_a);
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 4u);
    }
}

TEST_F(GameTests, FreezeScenesListOverloads)
{
    setup_two_scenes(*g);

    g->freeze_scenes({scene_a, scene_b});
    EXPECT_TRUE(g->is_scene_frozen(scene_a));
    EXPECT_TRUE(g->is_scene_frozen(scene_b));

    g->unfreeze_scenes({scene_a});
    EXPECT_FALSE(g->is_scene_frozen(scene_a));
    EXPECT_TRUE(g->is_scene_frozen(scene_b));

    // The repeated id is the already-frozen no-op.
    g->freeze_scenes({scene_a, scene_a});
    EXPECT_TRUE(g->is_scene_frozen(scene_a));

    g->freeze_scenes({});
    g->unfreeze_scenes({});
    EXPECT_TRUE(g->is_scene_frozen(scene_a));
    EXPECT_TRUE(g->is_scene_frozen(scene_b));

    g->unfreeze_scenes({scene_a, scene_b});
    EXPECT_FALSE(g->is_scene_frozen(scene_a));
    EXPECT_FALSE(g->is_scene_frozen(scene_b));
}

TEST_F(GameTests, FreezeAllExceptSnapshot)
{
    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    g->create_scene(scene_c, build_scene_c);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);
    g->load_scene(scene_c);

    // Test 1: only active scenes are frozen, the kept one and the merely loaded one are not
    {
        g->freeze_all_except(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
        EXPECT_FALSE(g->is_scene_frozen(scene_c));
    }

    // Test 2: a scene activated afterwards is not frozen
    {
        g->activate_scene(scene_c);
        EXPECT_FALSE(g->is_scene_frozen(scene_c));
    }

    // Test 3: the list overload keeps every listed scene, and a kept frozen scene stays frozen
    {
        g->freeze_all_except({scene_a, scene_b});
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
        EXPECT_TRUE(g->is_scene_frozen(scene_c));
    }

    // Test 4: an empty list or an unregistered id keeps nothing
    {
        g->unfreeze_all();
        g->freeze_all_except({});
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
        EXPECT_TRUE(g->is_scene_frozen(scene_c));

        g->unfreeze_all();
        g->freeze_all_except(static_cast<scene_id>(999));
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
        EXPECT_TRUE(g->is_scene_frozen(scene_c));
    }
}

TEST_F(GameTests, UnfreezeAll)
{
    setup_two_scenes(*g);

    // Test 1: clears every frozen scene
    {
        g->freeze_scenes({scene_a, scene_b});
        g->unfreeze_all();
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->is_scene_frozen(scene_b));
    }

    // Test 2: clears a flag set on an inactive scene through entities()
    {
        g->deactivate_scene(scene_b);
        g->entities().set_scene_frozen(scene_b, true);
        ASSERT_TRUE(g->is_scene_frozen(scene_b));

        g->unfreeze_all();
        EXPECT_FALSE(g->is_scene_frozen(scene_b));
    }

    // Test 3: with nothing frozen it does nothing
    {
        g->unfreeze_all();
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->is_scene_frozen(scene_b));
    }
}

TEST_F(GameTests, DeactivateAndUnloadClearFreeze)
{
    setup_two_scenes(*g);

    // Test 1: deactivate then activate returns an unfrozen, fully visible scene
    {
        g->freeze_scene(scene_a);
        g->deactivate_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->entities().is_scene_frozen(scene_a));

        g->activate_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 3u);
    }

    // Test 2: unload then reload returns an unfrozen, fully visible scene
    {
        std::vector<entity> old_entities = g->scene_entities(scene_a);
        g->freeze_scene(scene_a);
        g->unload_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->entities().is_scene_frozen(scene_a));
        for (entity e : old_entities)
            EXPECT_FALSE(g->entities().valid(e));

        g->load_scene(scene_a);
        g->activate_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 3u);
    }

    // Test 3: unloading an inactive or a loaded scene leaves another scene's freeze alone
    {
        g->freeze_scene(scene_b);

        g->deactivate_scene(scene_a);
        g->unload_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));

        g->load_scene(scene_a);
        g->unload_scene(scene_a);
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
    }
}

TEST_F(GameTests, FreezeGuarded)
{
    const scene_id unregistered = static_cast<scene_id>(999);

    g->create_scene(scene_a, build_scene_a);
    g->create_scene(scene_b, build_scene_b);
    g->create_scene(scene_c, build_scene_c);
    test_prepare_to_play(*g);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    g->load_scene(scene_b);
    g->activate_scene(scene_b);
    g->load_scene(scene_c);

    // Test 1: freeze_scene on an unregistered scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->freeze_scene(unregistered));
#else
        // Release: guard-return, nothing gets frozen.
        g->freeze_scene(unregistered);
        EXPECT_FALSE(g->entities().is_scene_frozen(unregistered));
#endif
    }

    // Test 2: freeze_scene on a loaded-only scene and on an inactive scene
    {
        g->deactivate_scene(scene_b);

#ifndef NDEBUG
        EXPECT_DEATH(g->freeze_scene(scene_c), "scene is not active");
        EXPECT_DEATH(g->freeze_scene(scene_b), "scene is not active");
#else
        // Release: guard-return, both stay unfrozen and keep their status.
        g->freeze_scene(scene_c);
        g->freeze_scene(scene_b);
        EXPECT_FALSE(g->entities().is_scene_frozen(scene_c));
        EXPECT_FALSE(g->entities().is_scene_frozen(scene_b));
        EXPECT_EQ(g->status(scene_c), scene_status::loaded);
        EXPECT_EQ(g->status(scene_b), scene_status::inactive);
#endif
    }

    // Test 3: unfreeze_scene on an unregistered, a loaded-only and an inactive scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->unfreeze_scene(unregistered));
        EXPECT_DEATH(g->unfreeze_scene(scene_c), "scene is not active");
        EXPECT_DEATH(g->unfreeze_scene(scene_b), "scene is not active");
#else
        // Release: guard-return, a flag set through entities() on a non-active scene is left alone.
        g->entities().set_scene_frozen(scene_c, true);
        g->unfreeze_scene(unregistered);
        g->unfreeze_scene(scene_c);
        g->unfreeze_scene(scene_b);
        EXPECT_TRUE(g->entities().is_scene_frozen(scene_c));
        EXPECT_EQ(g->status(scene_c), scene_status::loaded);
        EXPECT_EQ(g->status(scene_b), scene_status::inactive);
#endif
    }

    // Test 4: a bad element in the list overloads
    {
        g->activate_scene(scene_b);
        g->unfreeze_all();

#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH((g->freeze_scenes({scene_a, unregistered, scene_b})));
        EXPECT_SCENE_UNREGISTERED_DEATH((g->unfreeze_scenes({scene_a, unregistered, scene_b})));
#else
        // Release: the bad element is skipped and the rest are still applied.
        g->freeze_scenes({scene_a, unregistered, scene_b});
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
        EXPECT_TRUE(g->is_scene_frozen(scene_b));
        EXPECT_FALSE(g->entities().is_scene_frozen(unregistered));

        g->unfreeze_scenes({scene_a, unregistered, scene_b});
        EXPECT_FALSE(g->is_scene_frozen(scene_a));
        EXPECT_FALSE(g->is_scene_frozen(scene_b));
#endif
    }

    // Test 5: is_scene_frozen on an unregistered scene
    {
#ifndef NDEBUG
        EXPECT_SCENE_UNREGISTERED_DEATH(g->is_scene_frozen(unregistered));
#else
        // Release: guard-return, an unregistered scene is not frozen.
        EXPECT_FALSE(g->is_scene_frozen(unregistered));
#endif
    }
}

TEST_F(GameTests, PrePlayFreezeIsGuarded)
{
    g->create_scene(scene_a, build_scene_a);
    g->load_scene(scene_a);
    g->activate_scene(scene_a);
    ASSERT_EQ(g->status(scene_a), scene_status::unloaded);

#ifndef NDEBUG
    EXPECT_DEATH(g->freeze_scene(scene_a), "scene is not active");
#else
    // Release: guard-return, nothing is queued or applied.
    g->freeze_scene(scene_a);
    EXPECT_FALSE(g->entities().is_scene_frozen(scene_a));
#endif

    // No scene is active yet, so these are silent no-ops in both builds.
    g->freeze_all_except(scene_a);
    g->unfreeze_all();
    g->freeze_scenes({});

    test_prepare_to_play(*g);
    EXPECT_EQ(g->status(scene_a), scene_status::active);
    EXPECT_FALSE(g->is_scene_frozen(scene_a));
    EXPECT_EQ(count_active_scene_entities(*g, scene_a), 3u);
}

TEST_F(GameTests, FreezeCoexistsWithOtherSceneApis)
{
    auto [P, C] = setup_two_scenes(*g);

    // Test 1: changing the layer of a frozen scene keeps it frozen and active
    {
        g->freeze_scene(scene_a);
        g->set_scene_layer(scene_a, 5);
        EXPECT_EQ(g->scene_layer(scene_a), 5);
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
        EXPECT_EQ(g->status(scene_a), scene_status::active);
    }

    // Test 2: an entity adopted into a frozen scene is frozen, and visible again once adopted out
    {
        g->entities().set_parent(C, P);
        ASSERT_NE(g->entities().get_component<components::scene_tag>(C), nullptr);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_a);
        EXPECT_TRUE(g->entities().is_active(C));
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities_including_frozen(*g, scene_a), 4u);

        entity new_parent = g->create_entity(scene_b);
        g->entities().set_parent(C, new_parent);
        EXPECT_EQ(g->entities().get_component<components::scene_tag>(C)->id, scene_b);
        EXPECT_EQ(count_active_scene_entities(*g, scene_a), 0u);
        EXPECT_EQ(count_scene_entities_including_frozen(*g, scene_a), 3u);
        EXPECT_EQ(count_active_scene_entities(*g, scene_b), 2u);
    }

    // Test 3: unloading another scene leaves the freeze alone
    {
        g->unload_scene(scene_b);
        EXPECT_TRUE(g->is_scene_frozen(scene_a));
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

TEST(GameMoveTests, FrozenStateSurvivesGameMove)
{
    test_utils::init_headless_gl();
    auto result = game::create("GameMoveTests.FrozenStateSurvivesGameMove");
    SKIP_IF_NO_GAME(result);

    setup_two_scenes(*result);
    result->freeze_scene(scene_a);

    game moved(std::move(*result));

    EXPECT_TRUE(moved.is_scene_frozen(scene_a));
    EXPECT_FALSE(moved.is_scene_frozen(scene_b));

    std::size_t seen_in_a = 0;
    for ([[maybe_unused]] auto [e, tag] : moved.entities().extract<components::scene_tag>())
        if (tag.id == scene_a) ++seen_in_a;
    EXPECT_EQ(seen_in_a, 0u);
    EXPECT_EQ(count_scene_entities_including_frozen(moved, scene_a), 3u);
}
