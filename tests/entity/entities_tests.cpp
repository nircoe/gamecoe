#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <chrono>
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
//                        SetParent Cycle Detection (debug-time assert)
//==============================================================================

TEST_F(EntitiesTests, SetParentCycleDetection)
{
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
