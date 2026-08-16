#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <chrono>
#include <vector>
#include "../test_utils.hpp"

using namespace gamecoe;
using namespace test_utils;

//==============================================================================
//                    Test Component Types
//==============================================================================

struct Position
{
    float x, y, z;
};

struct Velocity
{
    float dx, dy, dz;
};

//==============================================================================
//                    EntitiesTests - Entities manager tests
//==============================================================================

class EntitiesTests : public ::testing::Test
{
protected:
    entities mgr;
};

//==============================================================================
//                        Entity Lifecycle
//==============================================================================

TEST_F(EntitiesTests, EntityLifecycle)
{
    // Test 1: Create and validate single entity
    {
        entity e = mgr.create();

        EXPECT_TRUE(mgr.valid(e));
        EXPECT_TRUE(e != entity::invalid());
        EXPECT_EQ(mgr.size(), 1);
    }

    // Test 2: Create multiple entities
    {
        mgr.clear();

        entity e0 = mgr.create();
        entity e1 = mgr.create();
        entity e2 = mgr.create();

        EXPECT_EQ(mgr.size(), 3);
        EXPECT_TRUE(mgr.valid(e0));
        EXPECT_TRUE(mgr.valid(e1));
        EXPECT_TRUE(mgr.valid(e2));

        // IDs should be distinct
        EXPECT_NE(e0, e1);
        EXPECT_NE(e1, e2);

        // Handles should be ordered by creation (ID-major layout)
        EXPECT_LT(e0, e1);
        EXPECT_LT(e1, e2);
    }

    // Test 3: Destroy entity
    {
        mgr.clear();
        entity e = mgr.create();
        EXPECT_EQ(mgr.size(), 1);

        mgr.destroy(e);

        EXPECT_FALSE(mgr.valid(e));
        EXPECT_EQ(mgr.size(), 0);
    }

    // Test 4: Recycle entity ID with incremented generation
    {
        mgr.clear();
        entity e0 = mgr.create();
        std::uint32_t original_id = e0.id();
        std::uint16_t original_gen = e0.generation();

        mgr.destroy(e0);

        entity e1 = mgr.create();

        // Same ID recycled, generation incremented
        EXPECT_EQ(e1.id(), original_id);
        EXPECT_EQ(e1.generation(), original_gen + 1);

        // Old handle is stale, new one is valid
        EXPECT_FALSE(mgr.valid(e0));
        EXPECT_TRUE(mgr.valid(e1));
    }

    // Test 5: Clear all entities
    {
        mgr.clear();
        entity e0 = mgr.create();
        entity e1 = mgr.create();
        entity e2 = mgr.create();

        mgr.clear();

        EXPECT_EQ(mgr.size(), 0);
        EXPECT_FALSE(mgr.valid(e0));
        EXPECT_FALSE(mgr.valid(e1));
        EXPECT_FALSE(mgr.valid(e2));
    }
}

//==============================================================================
//                        Component Operations
//==============================================================================

TEST_F(EntitiesTests, ComponentOperations)
{
    // Test 1: Add, has, get, remove component
    {
        entity e = mgr.create();

        // Add
        mgr.add_component<Position>(e, Position{1.0f, 2.0f, 3.0f});
        EXPECT_TRUE(mgr.has_component<Position>(e));

        // Get (mutable)
        Position *pos = mgr.get_component<Position>(e);
        EXPECT_NE(pos, nullptr);
        EXPECT_EQ(pos->x, 1.0f);

        // Modify and verify
        pos->x = 99.0f;
        EXPECT_EQ(mgr.get_component<Position>(e)->x, 99.0f);

        // Remove
        mgr.remove_component<Position>(e);
        EXPECT_FALSE(mgr.has_component<Position>(e));
        EXPECT_EQ(mgr.get_component<Position>(e), nullptr);

        // Entity still valid after component removal
        EXPECT_TRUE(mgr.valid(e));
    }

    // Test 2: Get component with const manager
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{5.0f, 6.0f, 7.0f});

        const entities &const_mgr = mgr;
        const Position *pos = const_mgr.get_component<Position>(e);

        EXPECT_NE(pos, nullptr);
        EXPECT_EQ(pos->x, 5.0f);
        static_assert(std::is_same_v<decltype(pos), const Position *>);
    }

    // Test 3: Get component returns nullptr for missing/invalid
    {
        mgr.clear();
        entity e = mgr.create();
        entity invalid = entity::invalid();

        // Entity without the component
        EXPECT_EQ(mgr.get_component<Position>(e), nullptr);

        // Invalid entity handle
        EXPECT_EQ(mgr.get_component<Position>(invalid), nullptr);
    }
}

//==============================================================================
//                        Multi-Component Cleanup
//==============================================================================

TEST_F(EntitiesTests, DestroyRemovesAllComponents)
{
    entity e = mgr.create();

    mgr.add_component<Position>(e, Position{1.0f, 0.0f, 0.0f});
    mgr.add_component<Velocity>(e, Velocity{0.1f, 0.0f, 0.0f});

    EXPECT_TRUE(mgr.has_component<Position>(e));
    EXPECT_TRUE(mgr.has_component<Velocity>(e));

    mgr.destroy(e);

    // Both components should be gone along with the entity
    EXPECT_FALSE(mgr.valid(e));
    EXPECT_FALSE(mgr.has_component<Position>(e));
    EXPECT_FALSE(mgr.has_component<Velocity>(e));
}

//==============================================================================
//                        Iteration
//==============================================================================

TEST_F(EntitiesTests, ForEach)
{
    for (int i = 0; i < 5; ++i)
    {
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    // Mutable for_each
    int count = 0;
    mgr.for_each<Position>([&count]([[maybe_unused]] entity e, Position &pos)
    {
        pos.x += 10.0f;
        ++count;
    });
    EXPECT_EQ(count, 5);

    // Const for_each — verify mutations persisted
    const entities &const_mgr = mgr;
    float sum = 0.0f;
    const_mgr.for_each<Position>([&sum]([[maybe_unused]] entity e, const Position &pos)
    {
        sum += pos.x;
    });
    // 0+10 + 1+10 + 2+10 + 3+10 + 4+10 = 60
    EXPECT_EQ(sum, 60.0f);
}

//==============================================================================
//                        Bulk Operations Performance
//==============================================================================

TEST_F(EntitiesTests, BulkOperations)
{
    const std::size_t NUM_ENTITIES = 1000;

    mgr.reserve(NUM_ENTITIES);

    auto start = std::chrono::high_resolution_clock::now();

    for (std::size_t i = 0; i < NUM_ENTITIES; ++i)
    {
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{static_cast<float>(i), 0.0f, 0.0f});
        mgr.add_component<Velocity>(e, Velocity{1.0f, 0.0f, 0.0f});
    }

    int count = 0;
    mgr.for_each<Position>([&count]([[maybe_unused]] entity e, Position &pos)
    {
        pos.x += 1.0f;
        ++count;
    });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(count, NUM_ENTITIES);
    EXPECT_EQ(mgr.size(), NUM_ENTITIES);
    EXPECT_LT(duration.count(), 10); // Should complete in < 10ms
}

//==============================================================================
//                        Capacity
//==============================================================================

TEST_F(EntitiesTests, Reserve)
{
    mgr.reserve(1000);

    // Reserve doesn't create entities
    EXPECT_EQ(mgr.size(), 0);

    // Normal creation still works after reserve
    entity e = mgr.create();
    EXPECT_TRUE(mgr.valid(e));
    EXPECT_EQ(mgr.size(), 1);
}

//==============================================================================
//                        Mandatory Transform
//==============================================================================

TEST_F(EntitiesTests, MandatoryTransform)
{
    // Test 1: Every entity has a transform immediately after create(), with default values
    {
        entity e = mgr.create();
        EXPECT_TRUE(mgr.has_component<components::transform>(e));

        components::transform *t = mgr.get_component<components::transform>(e);
        ASSERT_NE(t, nullptr);
        expect_vec3_near(t->position, glm::vec3(0.0f));
        expect_vec3_near(t->scale, glm::vec3(1.0f));
    }

    // Test 2: transform is still present after removing other components (unaffected by unrelated removes)
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{1.0f, 2.0f, 3.0f});
        mgr.remove_component<Position>(e);

        EXPECT_TRUE(mgr.has_component<components::transform>(e));
    }

    // Test 3: transform cannot be added or removed via the public API (compile-time guard)
    // Uncommenting either line below must fail to compile:
    // mgr.add_component<components::transform>(e);
    // mgr.remove_component<components::transform>(e);

    // Test 4: transform() accessor returns the same component as get_component<transform>(), no null-check needed
    {
        mgr.clear();
        entity e = mgr.create();
        components::transform &t = mgr.transform(e);
        t.position = glm::vec3(5.0f, 0.0f, 0.0f);

        expect_vec3_near(mgr.get_component<components::transform>(e)->position, mgr.transform(e).position);

        const entities &const_mgr = mgr;
        const components::transform &const_t = const_mgr.transform(e);
        expect_vec3_near(const_t.position, glm::vec3(5.0f, 0.0f, 0.0f));
    }
}

//==============================================================================
//                        Hierarchy (set_parent / remove_parent / remove_children)
//==============================================================================

TEST_F(EntitiesTests, Hierarchy)
{
    // Test 1: set_parent makes both sides consistent
    {
        entity parent = mgr.create();
        entity child = mgr.create();

        mgr.set_parent(child, parent);

        ASSERT_TRUE(mgr.has_component<components::parent>(child));
        EXPECT_EQ(mgr.get_component<components::parent>(child)->handle, parent);

        ASSERT_TRUE(mgr.has_component<components::children>(parent));
        const auto &handles = mgr.get_component<components::children>(parent)->handles;
        ASSERT_EQ(handles.size(), 1);
        EXPECT_EQ(handles[0], child);
    }

    // Test 2: multiple children accumulate under one parent, in order
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child0 = mgr.create();
        entity child1 = mgr.create();

        mgr.set_parent(child0, parent);
        mgr.set_parent(child1, parent);

        const auto &handles = mgr.get_component<components::children>(parent)->handles;
        ASSERT_EQ(handles.size(), 2);
        EXPECT_EQ(handles[0], child0);
        EXPECT_EQ(handles[1], child1);
    }

    // Test 3: re-parenting detaches from the old parent (component removed once empty) and attaches to the new one
    {
        mgr.clear();
        entity old_parent = mgr.create();
        entity new_parent = mgr.create();
        entity child = mgr.create();

        mgr.set_parent(child, old_parent);
        mgr.set_parent(child, new_parent);

        EXPECT_EQ(mgr.get_component<components::parent>(child)->handle, new_parent);
        EXPECT_FALSE(mgr.has_component<components::children>(old_parent));

        const auto &new_handles = mgr.get_component<components::children>(new_parent)->handles;
        ASSERT_EQ(new_handles.size(), 1);
        EXPECT_EQ(new_handles[0], child);
    }

    // Test 4: repeated same-parent call is a no-op (doesn't duplicate)
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child = mgr.create();

        mgr.set_parent(child, parent);
        mgr.set_parent(child, parent);

        const auto &handles = mgr.get_component<components::children>(parent)->handles;
        EXPECT_EQ(handles.size(), 1);
    }

    // Test 5: remove_parent detaches both sides; parent's children component is removed once it has no children left
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child = mgr.create();
        mgr.set_parent(child, parent);

        mgr.remove_parent(child);

        EXPECT_FALSE(mgr.has_component<components::parent>(child));
        EXPECT_FALSE(mgr.has_component<components::children>(parent));
        EXPECT_TRUE(mgr.valid(child)); // child remains a valid, independent entity
    }

    // Test 6: remove_parent with multiple children only detaches the specified one; children component remains for the rest
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child0 = mgr.create();
        entity child1 = mgr.create();
        mgr.set_parent(child0, parent);
        mgr.set_parent(child1, parent);

        mgr.remove_parent(child0);

        EXPECT_FALSE(mgr.has_component<components::parent>(child0));
        ASSERT_TRUE(mgr.has_component<components::children>(parent));
        const auto &handles = mgr.get_component<components::children>(parent)->handles;
        ASSERT_EQ(handles.size(), 1);
        EXPECT_EQ(handles[0], child1);
    }

    // Test 7: remove_parent on an unparented entity is a silent no-op
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.remove_parent(e);
        EXPECT_FALSE(mgr.has_component<components::parent>(e));
    }

    // Test 8: remove_children detaches every child (kept alive) and clears the parent's children component
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child0 = mgr.create();
        entity child1 = mgr.create();
        mgr.set_parent(child0, parent);
        mgr.set_parent(child1, parent);

        mgr.remove_children(parent);

        EXPECT_FALSE(mgr.has_component<components::children>(parent));
        EXPECT_FALSE(mgr.has_component<components::parent>(child0));
        EXPECT_FALSE(mgr.has_component<components::parent>(child1));
        EXPECT_TRUE(mgr.valid(child0));
        EXPECT_TRUE(mgr.valid(child1));
    }

    // Test 9: remove_children on a childless entity is a silent no-op
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.remove_children(e);
        EXPECT_FALSE(mgr.has_component<components::children>(e));
    }

    // Test 10: hierarchy components cannot be added/removed via the public API (compile-time guard)
    // Uncommenting any line below must fail to compile:
    // mgr.add_component<components::parent>(entity{}, components::parent{});
    // mgr.remove_component<components::parent>(entity{});
    // mgr.add_component<components::children>(entity{}, components::children{});
    // mgr.remove_component<components::children>(entity{});
}

//==============================================================================
//                        RemoveChildren With Grandchildren (copy-before-iterate guard)
//==============================================================================

TEST_F(EntitiesTests, RemoveChildrenWithGrandchildren)
{
    // remove_children() copies parent's children handles before iterating, specifically because
    // detaching one child can itself cascade set_active() into that child's own children -
    // mutating storage the loop would otherwise still be reading from. Every other
    // remove_children() test in this file uses flat children (no grandchildren), so none of them
    // exercise that cascade; this is the one guard that would catch the copy being reverted to a
    // reference.
    mgr.clear();
    entity parent = mgr.create();
    entity child_a = mgr.create();
    entity child_b = mgr.create();
    entity grandchild = mgr.create();
    mgr.set_parent(child_a, parent);
    mgr.set_parent(child_b, parent);
    mgr.set_parent(grandchild, child_a);

    mgr.deactivate(parent);
    mgr.deactivate(grandchild); // grandchild's own self_active is now false; child_a's stays true

    // Setup check: parent's inactivity suppresses the whole subtree, including the grandchild
    EXPECT_FALSE(mgr.is_active(child_a));
    EXPECT_FALSE(mgr.is_active(child_b));
    EXPECT_FALSE(mgr.is_active(grandchild));

    mgr.remove_children(parent);

    // Test 1: detaching child_a/child_b from parent leaves grandchild's own link untouched -
    // it's still parented to child_a, not orphaned or reparented to parent
    EXPECT_FALSE(mgr.has_component<components::children>(parent));
    EXPECT_FALSE(mgr.has_component<components::parent>(child_a));
    EXPECT_FALSE(mgr.has_component<components::parent>(child_b));
    EXPECT_TRUE(mgr.valid(child_a));
    EXPECT_TRUE(mgr.valid(child_b));
    EXPECT_TRUE(mgr.valid(grandchild));
    ASSERT_TRUE(mgr.has_component<components::parent>(grandchild));
    EXPECT_EQ(mgr.get_component<components::parent>(grandchild)->handle, child_a);

    // Test 2: each detached child's world_active recomputes from its own self_active as a new
    // parentless root; grandchild's world_active recomputes relative to child_a's new state, not
    // parent's old one, and still respects grandchild's own self_active independently
    EXPECT_TRUE(mgr.is_active(child_a));   // parentless root now - self_active alone decides
    EXPECT_TRUE(mgr.is_active(child_b));   // same
    EXPECT_FALSE(mgr.is_active(grandchild)); // follows child_a (now active), but own self_active is false
}

//==============================================================================
//                        SetParent Cycle Detection (debug-time assert)
//==============================================================================

TEST_F(EntitiesTests, SetParentCycleDetection)
{
#ifdef NDEBUG
    GTEST_SKIP() << "cycle detection assert is compiled out under NDEBUG (Release build)";
#endif

    // Test 1: direct 2-node cycle - A is already parent of B, then set_parent(A, B) would make B a
    // parent of its own ancestor A
    EXPECT_DEATH(
        {
            entity a = mgr.create();
            entity b = mgr.create();
            mgr.set_parent(b, a); // b's parent = a
            mgr.set_parent(a, b); // would create a -> b -> a cycle
        },
        "would create a parent/child cycle");

    // Test 2: indirect 3-node cycle - A -> B -> C existing chain, then set_parent(A, C) closes the loop
    EXPECT_DEATH(
        {
            entity a = mgr.create();
            entity b = mgr.create();
            entity c = mgr.create();
            mgr.set_parent(b, a); // b's parent = a
            mgr.set_parent(c, b); // c's parent = b
            mgr.set_parent(a, c); // would create a -> c -> b -> a cycle
        },
        "would create a parent/child cycle");
}

//==============================================================================
//                        Destroy Cascade (destroy() with hierarchy)
//==============================================================================

TEST_F(EntitiesTests, DestroyCascade)
{
    // Test 1: destroying a childless, unparented entity still works (regression baseline)
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.destroy(e);

        EXPECT_FALSE(mgr.valid(e));
        EXPECT_EQ(mgr.size(), 0);
    }

    // Test 2: destroying a parent with one child destroys the child too
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child = mgr.create();
        mgr.set_parent(child, parent);

        mgr.destroy(parent);

        EXPECT_FALSE(mgr.valid(parent));
        EXPECT_FALSE(mgr.valid(child));
    }

    // Test 3: destroying a parent with multiple children destroys all of them
    {
        mgr.clear();
        entity parent = mgr.create();
        entity child0 = mgr.create();
        entity child1 = mgr.create();
        mgr.set_parent(child0, parent);
        mgr.set_parent(child1, parent);

        mgr.destroy(parent);

        EXPECT_FALSE(mgr.valid(parent));
        EXPECT_FALSE(mgr.valid(child0));
        EXPECT_FALSE(mgr.valid(child1));
    }

    // Test 4: destroying a root cascades through a multi-level hierarchy (grandchildren too)
    {
        mgr.clear();
        entity root = mgr.create();
        entity child = mgr.create();
        entity grandchild = mgr.create();
        mgr.set_parent(child, root);
        mgr.set_parent(grandchild, child);

        mgr.destroy(root);

        EXPECT_FALSE(mgr.valid(root));
        EXPECT_FALSE(mgr.valid(child));
        EXPECT_FALSE(mgr.valid(grandchild));
    }

    // Test 5: destroying a middle node detaches upward (parent stays valid, loses it from children)
    // and cascades downward (its own child is destroyed too)
    {
        mgr.clear();
        entity root = mgr.create();
        entity middle = mgr.create();
        entity leaf = mgr.create();
        mgr.set_parent(middle, root);
        mgr.set_parent(leaf, middle);

        mgr.destroy(middle);

        EXPECT_TRUE(mgr.valid(root));
        EXPECT_FALSE(mgr.valid(middle));
        EXPECT_FALSE(mgr.valid(leaf));

        // root's children component should no longer exist - middle was root's only child
        EXPECT_FALSE(mgr.has_component<components::children>(root));
    }
}

//==============================================================================
//                        Destroy vs. Active/Inactive Partition
//==============================================================================

TEST_F(EntitiesTests, DestroyKeepsActivePartitionIntact)
{
    // Test 1: destroying an ACTIVE entity leaves the active/inactive partition consistent
    // across every pool - survivors' active-state and payloads are untouched by the swap-and-pop
    {
        mgr.clear();
        std::vector<entity> es;
        for (int i = 0; i < 6; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Position>(e, Position{static_cast<float>(i), 0.0f, 0.0f});
            mgr.add_component<Velocity>(e, Velocity{static_cast<float>(i) * 0.1f, 0.0f, 0.0f});
            es.push_back(e);
        }

        mgr.deactivate(es[1]);
        mgr.deactivate(es[3]);

        entity destroyed = es[0]; // still active
        mgr.destroy(destroyed);

        EXPECT_FALSE(mgr.valid(destroyed));

        std::vector<entity> active_found;
        for (auto [e, pos, vel] : mgr.extract<Position, Velocity>())
            active_found.push_back(e);

        EXPECT_EQ(active_found.size(), 3u); // es[2], es[4], es[5]
        EXPECT_TRUE(test_utils::has(active_found, es[2]));
        EXPECT_TRUE(test_utils::has(active_found, es[4]));
        EXPECT_TRUE(test_utils::has(active_found, es[5]));

        EXPECT_FALSE(mgr.is_active(es[1]));
        EXPECT_TRUE(mgr.is_active(es[2]));
        EXPECT_FALSE(mgr.is_active(es[3]));
        EXPECT_TRUE(mgr.is_active(es[4]));
        EXPECT_TRUE(mgr.is_active(es[5]));

        for (int i = 1; i < 6; ++i)
        {
            Position *pos = mgr.get_component<Position>(es[i]);
            Velocity *vel = mgr.get_component<Velocity>(es[i]);
            ASSERT_NE(pos, nullptr);
            ASSERT_NE(vel, nullptr);
            EXPECT_EQ(pos->x, static_cast<float>(i));
            EXPECT_EQ(vel->dx, static_cast<float>(i) * 0.1f);
        }
    }

    // Test 2: destroying an INACTIVE entity doesn't change the active survivor count, and
    // everyone else's active-state/payload stays intact
    {
        mgr.clear();
        std::vector<entity> es;
        for (int i = 0; i < 6; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Position>(e, Position{static_cast<float>(i), 0.0f, 0.0f});
            mgr.add_component<Velocity>(e, Velocity{static_cast<float>(i) * 0.1f, 0.0f, 0.0f});
            es.push_back(e);
        }

        mgr.deactivate(es[1]);
        mgr.deactivate(es[3]);

        entity destroyed = es[1]; // inactive
        mgr.destroy(destroyed);

        EXPECT_FALSE(mgr.valid(destroyed));

        std::vector<entity> active_found;
        for (auto [e, pos, vel] : mgr.extract<Position, Velocity>())
            active_found.push_back(e);

        // Active survivor count unchanged from before the destroy: es[0], es[2], es[4], es[5]
        EXPECT_EQ(active_found.size(), 4u);
        EXPECT_TRUE(test_utils::has(active_found, es[0]));
        EXPECT_TRUE(test_utils::has(active_found, es[2]));
        EXPECT_TRUE(test_utils::has(active_found, es[4]));
        EXPECT_TRUE(test_utils::has(active_found, es[5]));

        EXPECT_TRUE(mgr.is_active(es[0]));
        EXPECT_TRUE(mgr.is_active(es[2]));
        EXPECT_FALSE(mgr.is_active(es[3]));
        EXPECT_TRUE(mgr.is_active(es[4]));
        EXPECT_TRUE(mgr.is_active(es[5]));

        for (int i : { 0, 2, 3, 4, 5 })
        {
            Position *pos = mgr.get_component<Position>(es[i]);
            Velocity *vel = mgr.get_component<Velocity>(es[i]);
            ASSERT_NE(pos, nullptr);
            ASSERT_NE(vel, nullptr);
            EXPECT_EQ(pos->x, static_cast<float>(i));
            EXPECT_EQ(vel->dx, static_cast<float>(i) * 0.1f);
        }
    }

    // Test 3: destroying a parent with mixed active/inactive children exercises the cascade
    // worklist against the partition-aware erase; entities outside the destroyed subtree
    // are not silently deactivated or corrupted
    {
        mgr.clear();

        entity other_active = mgr.create();
        mgr.add_component<Position>(other_active, Position{10.0f, 0.0f, 0.0f});

        entity other_inactive = mgr.create();
        mgr.add_component<Position>(other_inactive, Position{20.0f, 0.0f, 0.0f});
        mgr.deactivate(other_inactive);

        entity parent = mgr.create();
        mgr.add_component<Position>(parent, Position{30.0f, 0.0f, 0.0f});

        entity child_active = mgr.create();
        mgr.add_component<Position>(child_active, Position{31.0f, 0.0f, 0.0f});
        mgr.set_parent(child_active, parent);

        entity child_inactive = mgr.create();
        mgr.add_component<Position>(child_inactive, Position{32.0f, 0.0f, 0.0f});
        mgr.set_parent(child_inactive, parent);
        mgr.deactivate(child_inactive);

        mgr.destroy(parent);

        EXPECT_FALSE(mgr.valid(parent));
        EXPECT_FALSE(mgr.valid(child_active));
        EXPECT_FALSE(mgr.valid(child_inactive));

        // Entities outside the destroyed subtree keep their own active-state and payload
        EXPECT_TRUE(mgr.valid(other_active));
        EXPECT_TRUE(mgr.valid(other_inactive));
        EXPECT_TRUE(mgr.is_active(other_active));
        EXPECT_FALSE(mgr.is_active(other_inactive));
        EXPECT_EQ(mgr.get_component<Position>(other_active)->x, 10.0f);
        EXPECT_EQ(mgr.get_component<Position>(other_inactive)->x, 20.0f);

        std::vector<entity> active_found;
        for (auto [e, pos] : mgr.extract<Position>())
            active_found.push_back(e);

        EXPECT_EQ(active_found.size(), 1u); // only other_active survives in the active partition
        EXPECT_TRUE(test_utils::has(active_found, other_active));
    }
}

//==============================================================================
//                        Activate / Deactivate
//==============================================================================

TEST_F(EntitiesTests, ActivateDeactivate)
{
    entity e = mgr.create();
    mgr.add_component<Position>(e, Position{1.0f, 2.0f, 3.0f});
    mgr.add_component<Velocity>(e, Velocity{0.1f, 0.2f, 0.3f});

    // Test 1: deactivate() removes the entity from extract<>() iteration but leaves its
    // components in place, still reachable via has_component()/get_component()
    {
        mgr.deactivate(e);

        bool found = false;
        for (auto [ent, pos, vel] : mgr.extract<Position, Velocity>())
            if (ent == e) found = true;
        EXPECT_FALSE(found);

        EXPECT_TRUE(mgr.has_component<Position>(e));
        EXPECT_TRUE(mgr.has_component<Velocity>(e));
        ASSERT_NE(mgr.get_component<Position>(e), nullptr);
        ASSERT_NE(mgr.get_component<Velocity>(e), nullptr);
        EXPECT_EQ(mgr.get_component<Position>(e)->x, 1.0f);
        EXPECT_EQ(mgr.get_component<Velocity>(e)->dx, 0.1f);
    }

    // Test 2: activate() on the same entity restores it to extract<>() iteration, payload intact
    {
        mgr.activate(e);

        bool found = false;
        for (auto [ent, pos, vel] : mgr.extract<Position, Velocity>())
            if (ent == e) found = true;
        EXPECT_TRUE(found);

        EXPECT_EQ(mgr.get_component<Position>(e)->x, 1.0f);
        EXPECT_EQ(mgr.get_component<Velocity>(e)->dx, 0.1f);
    }

    // Test 3: a fresh entity is active by default at create(); is_active() tracks state at
    // each transition, not just the final state
    {
        mgr.clear();
        entity fresh = mgr.create();

        EXPECT_TRUE(mgr.is_active(fresh));

        mgr.deactivate(fresh);
        EXPECT_FALSE(mgr.is_active(fresh));

        mgr.activate(fresh);
        EXPECT_TRUE(mgr.is_active(fresh));
    }
}

//==============================================================================
//                        Self-Active vs. World-Active Cascade
//==============================================================================

TEST_F(EntitiesTests, SelfActiveCascade)
{
    // Test 1: reactivating a parent must restore each descendant to its OWN self_active
    // state, not force every descendant active (independently-toggled siblings)
    {
        mgr.clear();
        entity player = mgr.create();
        entity gun = mgr.create();
        entity dagger = mgr.create();
        entity stick = mgr.create();
        mgr.set_parent(gun, player);
        mgr.set_parent(dagger, player);
        mgr.set_parent(stick, player);

        // Only dagger/stick are explicitly deactivated - this touches only their own
        // self_active, gun's self_active stays true
        mgr.deactivate(dagger);
        mgr.deactivate(stick);

        EXPECT_TRUE(mgr.is_active(gun));
        EXPECT_FALSE(mgr.is_active(dagger));
        EXPECT_FALSE(mgr.is_active(stick));

        // Deactivating the shared ancestor forces every descendant's world_active false,
        // regardless of their own self_active
        mgr.deactivate(player);

        EXPECT_FALSE(mgr.is_active(gun));
        EXPECT_FALSE(mgr.is_active(dagger));
        EXPECT_FALSE(mgr.is_active(stick));

        mgr.activate(player);

        // Regression guard: reactivating player must NOT force dagger/stick active too -
        // each descendant is restored to its own self_active state
        EXPECT_TRUE(mgr.is_active(gun));
        EXPECT_FALSE(mgr.is_active(dagger));
        EXPECT_FALSE(mgr.is_active(stick));
    }

    // Test 2: deactivate() cascades to every one of the entity's pools, not just some
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{1.0f, 2.0f, 3.0f});
        mgr.add_component<Velocity>(e, Velocity{0.1f, 0.2f, 0.3f});
        mgr.add_component<components::scene_tag>(e, components::scene_tag{});

        mgr.deactivate(e);

        EXPECT_FALSE(mgr.is_active(e));

        bool found_position = false;
        for (auto [ent, pos] : mgr.extract<Position>())
            if (ent == e) found_position = true;
        EXPECT_FALSE(found_position);

        bool found_velocity = false;
        for (auto [ent, vel] : mgr.extract<Velocity>())
            if (ent == e) found_velocity = true;
        EXPECT_FALSE(found_velocity);

        bool found_scene_tag = false;
        for (auto [ent, tag] : mgr.extract<components::scene_tag>())
            if (ent == e) found_scene_tag = true;
        EXPECT_FALSE(found_scene_tag);
    }

    // Test 3: multi-level chain - a self-inactive intermediate/leaf node stays inactive through
    // an ancestor's deactivate()/activate() cycle, at depth 2 (not just direct children)
    {
        mgr.clear();
        entity player = mgr.create();
        entity backpack = mgr.create();
        entity potion = mgr.create();
        mgr.set_parent(backpack, player);
        mgr.set_parent(potion, backpack);

        // Only potion is explicitly deactivated - backpack's self_active stays true (default)
        mgr.deactivate(potion);

        mgr.deactivate(player);
        EXPECT_FALSE(mgr.is_active(backpack));
        EXPECT_FALSE(mgr.is_active(potion));

        mgr.activate(player);

        // backpack's self_active was never touched, so it comes back active with player
        EXPECT_TRUE(mgr.is_active(backpack));
        // potion's own self_active is still false - stays inactive through the whole cycle
        EXPECT_FALSE(mgr.is_active(potion));
    }

    // Test 4: activate() on an already self-active entity is a no-op - no crash, no corruption
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{4.0f, 5.0f, 6.0f});

        EXPECT_TRUE(mgr.is_active(e));

        mgr.activate(e); // already self-active - should be ignored

        EXPECT_TRUE(mgr.is_active(e));

        bool found = false;
        for (auto [ent, pos] : mgr.extract<Position>())
            if (ent == e) found = true;
        EXPECT_TRUE(found);
        EXPECT_EQ(mgr.get_component<Position>(e)->x, 4.0f);
    }
}

//==============================================================================
//                        Reparent Recomputes World-Active
//==============================================================================

TEST_F(EntitiesTests, ReparentRecomputesWorldActive)
{
    // Test 1: set_parent onto an inactive parent forces the child inactive purely from the
    // ancestor-chain change - child's own self_active is never touched. Re-parenting again onto
    // an active parent restores it, again with no explicit activate()/deactivate() call on the
    // child anywhere in this block
    {
        mgr.clear();
        entity child = mgr.create();
        EXPECT_TRUE(mgr.is_active(child));

        entity inactive_parent = mgr.create();
        mgr.deactivate(inactive_parent);

        mgr.set_parent(child, inactive_parent);
        EXPECT_FALSE(mgr.is_active(child));

        entity active_parent = mgr.create();
        mgr.set_parent(child, active_parent);
        EXPECT_TRUE(mgr.is_active(child));
    }

    // Test 2: remove_parent from an inactive parent restores world_active from the child's own
    // self_active alone, now that it's a parentless root - no explicit activate() call
    {
        mgr.clear();
        entity parent = mgr.create();
        mgr.deactivate(parent);

        entity child = mgr.create();
        mgr.set_parent(child, parent);
        // child's own self_active is still true (never explicitly deactivated), but the
        // inactive parent suppresses it
        EXPECT_FALSE(mgr.is_active(child));

        mgr.remove_parent(child);
        EXPECT_TRUE(mgr.is_active(child));
    }

    // Test 3: remove_children recomputes each detached child independently from its own
    // self_active, not a blanket reactivation of every former child
    {
        mgr.clear();
        entity parent = mgr.create();
        mgr.deactivate(parent);

        entity child_a = mgr.create();
        entity child_b = mgr.create();
        mgr.set_parent(child_a, parent);
        mgr.set_parent(child_b, parent);
        mgr.deactivate(child_b); // child_b's own self_active is now false; child_a's stays true

        // Setup check: parent's inactivity suppresses both, regardless of their own self_active
        EXPECT_FALSE(mgr.is_active(child_a));
        EXPECT_FALSE(mgr.is_active(child_b));

        mgr.remove_children(parent);

        EXPECT_TRUE(mgr.is_active(child_a));  // parentless root now - self_active alone decides
        EXPECT_FALSE(mgr.is_active(child_b)); // still parentless, but its own self_active is false
    }

    // Test 4: reparenting between two inactive parents must never observably pass through an
    // active intermediate state. set_parent() detaches the old link via the private
    // unlink_parent() helper (not the public remove_parent()) specifically so the child is never
    // treated as a parentless root mid-call - which, with its true self_active of true, would
    // otherwise be momentarily active - before being linked to the new parent and recomputed once
    // at the very end via a single set_active() call. This test can only observe before/after
    // state, but that's still a meaningful regression guard: a broken implementation that routed
    // through remove_parent()'s own recompute would flip is_active(child) to true right before
    // set_parent() completes, and a caller inspecting state from a signal/callback triggered by
    // that recompute would see it.
    {
        mgr.clear();
        entity old_parent = mgr.create();
        entity new_parent = mgr.create();
        mgr.deactivate(old_parent);
        mgr.deactivate(new_parent);

        entity child = mgr.create();
        mgr.set_parent(child, old_parent);
        EXPECT_FALSE(mgr.is_active(child));

        mgr.set_parent(child, new_parent);
        EXPECT_FALSE(mgr.is_active(child));
    }
}

//==============================================================================
//                        Add Component To Inactive Entity
//==============================================================================

TEST_F(EntitiesTests, AddComponentToInactiveEntity)
{
    // Test 1: add_component() on an already-deactivated entity inserts the new component
    // straight into the inactive partition - it's hidden from extract<>() until activate(),
    // even though it was never itself explicitly deactivated. The reference returned by
    // add_component() must stay valid immediately after the call (regression guard against the
    // internal deactivate-on-insert swap relocating it before the caller reads from it).
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Position>(e, Position{1.0f, 2.0f, 3.0f});
        mgr.deactivate(e);

        Velocity &vel_ref = mgr.add_component<Velocity>(e, Velocity{4.0f, 5.0f, 6.0f});

        // Reference is valid right away, not just via a fresh get_component() afterward
        EXPECT_EQ(vel_ref.dx, 4.0f);
        EXPECT_EQ(vel_ref.dy, 5.0f);
        EXPECT_EQ(vel_ref.dz, 6.0f);

        bool found_single = false;
        for (auto [ent, vel] : mgr.extract<Velocity>())
            if (ent == e) found_single = true;
        EXPECT_FALSE(found_single);

        bool found_multi = false;
        for (auto [ent, pos, vel] : mgr.extract<Position, Velocity>())
            if (ent == e) found_multi = true;
        EXPECT_FALSE(found_multi);

        // Direct lookup still works regardless of active state
        Velocity *vel = mgr.get_component<Velocity>(e);
        ASSERT_NE(vel, nullptr);
        EXPECT_EQ(vel->dx, 4.0f);
        EXPECT_EQ(vel->dy, 5.0f);
        EXPECT_EQ(vel->dz, 6.0f);

        mgr.activate(e);

        bool found_single_active = false;
        for (auto [ent, v] : mgr.extract<Velocity>())
            if (ent == e) found_single_active = true;
        EXPECT_TRUE(found_single_active);

        bool found_multi_active = false;
        for (auto [ent, pos, v] : mgr.extract<Position, Velocity>())
        {
            if (ent == e)
            {
                found_multi_active = true;
                EXPECT_EQ(pos.x, 1.0f);
                EXPECT_EQ(pos.y, 2.0f);
                EXPECT_EQ(pos.z, 3.0f);
                EXPECT_EQ(v.dx, 4.0f);
                EXPECT_EQ(v.dy, 5.0f);
                EXPECT_EQ(v.dz, 6.0f);
            }
        }
        EXPECT_TRUE(found_multi_active);
    }

    // Test 2: set_parent() on an already-deactivated child propagates that same inactive state
    // into its freshly-inserted parent component, instead of defaulting it to active
    {
        mgr.clear();
        entity child = mgr.create();
        mgr.deactivate(child);

        entity parent = mgr.create();
        mgr.set_parent(child, parent);

        bool found = false;
        for (auto [ent, p] : mgr.extract<components::parent>())
            if (ent == child) found = true;
        EXPECT_FALSE(found);

        // Direct lookup unaffected by active state
        EXPECT_TRUE(mgr.has_component<components::parent>(child));
        ASSERT_NE(mgr.get_component<components::parent>(child), nullptr);
        EXPECT_EQ(mgr.get_component<components::parent>(child)->handle, parent);
    }
}

//==============================================================================
//                        Deep Hierarchy Cascade (iterative worklist, not recursion)
//==============================================================================

TEST_F(EntitiesTests, DeepHierarchyCascade)
{
    // destroy()/set_active() cascade a hierarchy via an explicit std::vector-backed worklist,
    // deliberately not recursion, so a malformed/deep parent chain can't stack-overflow. Every
    // other test in this file tops out at 2-3 levels deep, so a recursive rewrite of either
    // cascade would pass the whole suite - this is the one guard deep enough to actually catch
    // that regression. Chain is built leaf-first (each new entity becomes the parent of the
    // previous one) so set_parent()'s own cycle-detection ancestor walk stays O(1) per call -
    // the walk starts at the brand-new parent, which has no ancestors yet.
    constexpr std::size_t DEPTH = 50000;

    std::vector<entity> chain;
    chain.reserve(DEPTH + 1);

    entity leaf = mgr.create();
    chain.push_back(leaf);
    entity current = leaf;
    for (std::size_t i = 0; i < DEPTH; ++i)
    {
        entity next = mgr.create();
        mgr.set_parent(current, next);
        chain.push_back(next);
        current = next;
    }

    entity deepest = chain.front();
    entity root = chain.back();

    // Test 1: deactivate(root) cascades world_active = false all the way down to the deepest
    // descendant, tens of thousands of levels away
    {
        mgr.deactivate(root);
        EXPECT_FALSE(mgr.is_active(deepest));
    }

    // Test 2: activate(root) cascades world_active = true back down to the deepest descendant
    {
        mgr.activate(root);
        EXPECT_TRUE(mgr.is_active(deepest));
    }

    // Test 3: a self-deactivated midpoint still suppresses everything below it (down to deepest)
    // through a root-level deactivate()/activate() cycle, while everything between root and the
    // midpoint - never touched itself - comes back active. Same self/world-split invariant as
    // SelfActiveCascade, just exercised at depth instead of on a 2-3 node tree.
    {
        entity midpoint = chain[chain.size() / 2];
        entity near_root = chain[chain.size() - 2]; // ancestor of midpoint, child of root

        mgr.deactivate(midpoint); // self_active(midpoint) = false, doesn't touch anyone else

        mgr.deactivate(root);
        mgr.activate(root);

        EXPECT_FALSE(mgr.is_active(midpoint)); // own self_active still false
        EXPECT_FALSE(mgr.is_active(deepest));  // descendant of a world-inactive midpoint
        EXPECT_TRUE(mgr.is_active(near_root)); // ancestor of midpoint, restored with root
    }

    // Test 4: destroy(root) cascades removal all the way down to the deepest descendant
    {
        mgr.destroy(root);

        EXPECT_FALSE(mgr.valid(root));
        EXPECT_FALSE(mgr.valid(deepest));
        EXPECT_EQ(mgr.size(), 0u);
    }
}
