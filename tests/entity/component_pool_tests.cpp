#include <gtest/gtest.h>
#include <gamecoe/entity/component_pool.hpp>
#include <algorithm>
#include <memory>
#include <vector>

using namespace gamecoe;

//==============================================================================
//                    Test Component Types
//==============================================================================

// Simple POD for basic tests
struct Position
{
    float x, y, z;

    bool operator==(const Position &other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

// Move-only type for perfect forwarding test
struct MoveOnlyComponent
{
    std::unique_ptr<int> data;

    MoveOnlyComponent(int val) : data(std::make_unique<int>(val)) {}

    MoveOnlyComponent(const MoveOnlyComponent &) = delete;
    MoveOnlyComponent &operator=(const MoveOnlyComponent &) = delete;
    MoveOnlyComponent(MoveOnlyComponent &&) noexcept = default;
    MoveOnlyComponent &operator=(MoveOnlyComponent &&) noexcept = default;
};

// RAII type for lifecycle test
struct LifetimeTracker
{
    int *counter;

    LifetimeTracker(int *c) : counter(c) { ++(*counter); }
    ~LifetimeTracker()
    {
        if (counter)
            --(*counter);
    }

    LifetimeTracker(const LifetimeTracker &other) : counter(other.counter)
    {
        if (counter)
            ++(*counter);
    }
    LifetimeTracker &operator=(const LifetimeTracker &other)
    {
        if (this != &other)
        {
            if (counter) --(*counter);
            counter = other.counter;
            if (counter) ++(*counter);
        }
        return *this;
    }

    LifetimeTracker(LifetimeTracker &&other) noexcept : counter(other.counter)
    {
        other.counter = nullptr;
    }
    LifetimeTracker &operator=(LifetimeTracker &&other) noexcept
    {
        if (this != &other)
        {
            if (counter) --(*counter);
            counter = other.counter;
            other.counter = nullptr;
        }
        return *this;
    }
};

//==============================================================================
//                    ComponentPoolTests - Component pool wrapper tests
//==============================================================================

class ComponentPoolTests : public ::testing::Test
{
protected:
    component_pool<Position> pool;
};

//==============================================================================
//                        Add and Get Operations
//==============================================================================

TEST_F(ComponentPoolTests, AddAndGetOperations)
{
    // Test 1: Add and get single component
    {
        auto e = entity::create(42, 0);

        Position &pos = pool.add(e, true, Position{1.0f, 2.0f, 3.0f});

        EXPECT_TRUE(pool.contains(e));
        EXPECT_EQ(pool.size(), 1);
        EXPECT_FALSE(pool.empty());
        EXPECT_EQ(pool.get(e), pos);

        // Modify and verify changes persist
        pool.get(e).x = 99.0f;
        EXPECT_EQ(pool.get(e).x, 99.0f);
    }

    // Test 2: Add multiple components
    {
        pool.clear();
        std::vector<entity> entities;

        for (std::uint32_t i = 0; i < 50; ++i)
        {
            auto e = entity::create(i, 0);
            entities.push_back(e);
            pool.add(e, true, Position{static_cast<float>(i), 0.0f, 0.0f});
        }

        EXPECT_EQ(pool.size(), 50);

        for (std::size_t i = 0; i < entities.size(); ++i)
            EXPECT_EQ(pool.get(entities[i]).x, static_cast<float>(i));
    }
}

//==============================================================================
//                        Duplicate Add
//==============================================================================

TEST_F(ComponentPoolTests, DuplicateAddIsNoOp)
{
    auto e = entity::create(1, 0);
    pool.add(e, true, Position{1.0f, 2.0f, 3.0f});

#ifndef NDEBUG
    EXPECT_DEATH(pool.add(e, true, Position{9.0f, 9.0f, 9.0f}), "entity already has this component");
#else
    // Release: guard-return, the original value is untouched by the duplicate add.
    pool.add(e, true, Position{9.0f, 9.0f, 9.0f});
    EXPECT_EQ(pool.get(e), (Position{1.0f, 2.0f, 3.0f}));
    EXPECT_EQ(pool.size(), 1u);
#endif
}

//==============================================================================
//                        Remove Components
//==============================================================================

TEST_F(ComponentPoolTests, RemoveSwapAndPop)
{
    auto e1 = entity::create(10, 0);
    auto e2 = entity::create(20, 0);
    auto e3 = entity::create(30, 0);

    pool.add(e1, true, Position{1.0f, 0.0f, 0.0f});
    pool.add(e2, true, Position{2.0f, 0.0f, 0.0f});
    pool.add(e3, true, Position{3.0f, 0.0f, 0.0f});

    pool.remove(e2);

    EXPECT_EQ(pool.size(), 2);
    EXPECT_TRUE(pool.contains(e1));
    EXPECT_FALSE(pool.contains(e2));
    EXPECT_TRUE(pool.contains(e3));

    // Verify e3's component data is still correct after swap
    EXPECT_EQ(pool.get(e3).x, 3.0f);
}

//==============================================================================
//                        Active/Inactive Partition
//==============================================================================

TEST_F(ComponentPoolTests, ActivePartition)
{
    std::vector<entity> entities;
    std::vector<Position> values;

    // Sweep: every entity still in the pool must still hold its originally-inserted value.
    // This is the real regression guard - active_size()/is_active() can look correct while
    // swap_components() silently desyncs a payload from its entity.
    auto payload_sweep = [&]()
    {
        for (std::size_t i = 0; i < entities.size(); ++i)
            if (pool.contains(entities[i]))
                EXPECT_EQ(pool.get(entities[i]), values[i]);
    };

    // Test 1: fresh inserts are all active
    {
        for (std::uint32_t i = 0; i < 3; ++i)
        {
            auto e = entity::create(i, 0);
            Position p{static_cast<float>(i), 0.0f, 0.0f};
            entities.push_back(e);
            values.push_back(p);
            pool.add(e, true, p);
        }

        EXPECT_EQ(pool.active_size(), pool.size());
        for (auto e : entities)
            EXPECT_TRUE(pool.is_active(e));
        payload_sweep();
    }

    auto e1 = entities[0];
    auto e2 = entities[1];
    auto e3 = entities[2];

    // Test 2: deactivate(e) decrements active_size(), keeps contains() true, payloads stay paired
    {
        pool.deactivate(e2);
        EXPECT_EQ(pool.active_size(), 2);
        EXPECT_TRUE(pool.contains(e2));
        EXPECT_FALSE(pool.is_active(e2));
        payload_sweep();
    }

    // Test 3: deactivate(e) again on an already-inactive entity is a no-op
    {
        auto active_before = pool.active_size();
        pool.deactivate(e2);
        EXPECT_EQ(pool.active_size(), active_before);
        payload_sweep();
    }

    // Test 4: activate(e) restores it to the active partition
    {
        pool.activate(e2);
        EXPECT_TRUE(pool.is_active(e2));
        EXPECT_EQ(pool.active_size(), 3);
        payload_sweep();
    }

    // Test 5: activate(e) again on an already-active entity is a no-op
    {
        auto active_before = pool.active_size();
        pool.activate(e2);
        EXPECT_EQ(pool.active_size(), active_before);
        payload_sweep();
    }

    // Test 6: deactivating every entity drops active_size() to 0, size() unchanged, all still contained
    {
        pool.deactivate(e1);
        pool.deactivate(e2);
        pool.deactivate(e3);

        EXPECT_EQ(pool.active_size(), 0);
        EXPECT_EQ(pool.size(), 3);
        EXPECT_TRUE(pool.contains(e1));
        EXPECT_TRUE(pool.contains(e2));
        EXPECT_TRUE(pool.contains(e3));
        payload_sweep();
    }

    // Test 7: multi-entity scenario - deactivate a subset, then activate a different subset,
    // sweeping payload pairing after every single mutation
    {
        pool.clear();
        entities.clear();
        values.clear();

        for (std::uint32_t i = 0; i < 6; ++i)
        {
            auto e = entity::create(i, 0);
            Position p{static_cast<float>(i) * 10.0f, 1.0f, 2.0f};
            entities.push_back(e);
            values.push_back(p);
            pool.add(e, true, p);
        }
        payload_sweep();

        // Deactivate entities 1, 3, 5
        pool.deactivate(entities[1]);
        payload_sweep();
        pool.deactivate(entities[3]);
        payload_sweep();
        pool.deactivate(entities[5]);
        payload_sweep();

        EXPECT_EQ(pool.active_size(), 3);
        EXPECT_TRUE(pool.is_active(entities[0]));
        EXPECT_FALSE(pool.is_active(entities[1]));
        EXPECT_TRUE(pool.is_active(entities[2]));
        EXPECT_FALSE(pool.is_active(entities[3]));
        EXPECT_TRUE(pool.is_active(entities[4]));
        EXPECT_FALSE(pool.is_active(entities[5]));

        // Activate a different subset (1 and 3), leaving 5 inactive
        pool.activate(entities[1]);
        payload_sweep();
        pool.activate(entities[3]);
        payload_sweep();

        EXPECT_EQ(pool.active_size(), 5);
        EXPECT_TRUE(pool.is_active(entities[0]));
        EXPECT_TRUE(pool.is_active(entities[1]));
        EXPECT_TRUE(pool.is_active(entities[2]));
        EXPECT_TRUE(pool.is_active(entities[3]));
        EXPECT_TRUE(pool.is_active(entities[4]));
        EXPECT_FALSE(pool.is_active(entities[5]));
    }
}

//==============================================================================
//                        Add Into a Pool With Inactive Entities
//==============================================================================

TEST_F(ComponentPoolTests, AddIntoPoolWithInactiveEntities)
{
    // Test 1: add() into a pool with [active ‖ inactive] must land the new component in its
    // own slot without corrupting the existing inactive entity's payload - a naive
    // `return m_components.back()` implementation would return a stale/wrong reference here.
    {
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        Position pos_a{1.0f, 0.0f, 0.0f};
        Position pos_b{2.0f, 0.0f, 0.0f};
        Position pos_c{3.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        pool.add(b, true, pos_b);
        pool.deactivate(b); // active=[a], inactive=[b]
        ASSERT_TRUE(pool.is_active(a));
        ASSERT_FALSE(pool.is_active(b));

        Position &ref_c = pool.add(c, true, pos_c);

        EXPECT_EQ(ref_c, pos_c);
        EXPECT_EQ(pool.get(c), pos_c);
        EXPECT_EQ(pool.get(b), pos_b); // untouched by the insert
        EXPECT_EQ(pool.active_size(), 2);
        EXPECT_TRUE(pool.is_active(c));
        EXPECT_FALSE(pool.is_active(b));
    }

    // Test 2: add() a new entity directly as inactive into a pool that already has active entities
    {
        pool.clear();
        auto a = entity::create(1, 0);
        auto d = entity::create(4, 0);

        Position pos_a{1.0f, 0.0f, 0.0f};
        Position pos_d{4.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        Position &ref_d = pool.add(d, false, pos_d);

        EXPECT_EQ(ref_d, pos_d);
        EXPECT_EQ(pool.get(d), pos_d);
        EXPECT_EQ(pool.active_size(), 1);
        EXPECT_TRUE(pool.is_active(a));
        EXPECT_FALSE(pool.is_active(d));
    }

    // Test 3: add() a new entity directly as inactive into a pool with BOTH active and
    // inactive entries already present
    {
        pool.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto e = entity::create(5, 0);

        Position pos_a{1.0f, 0.0f, 0.0f};
        Position pos_b{2.0f, 0.0f, 0.0f};
        Position pos_e{5.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        pool.add(b, true, pos_b);
        pool.deactivate(b); // active=[a], inactive=[b]
        ASSERT_EQ(pool.active_size(), 1);

        Position &ref_e = pool.add(e, false, pos_e);

        EXPECT_EQ(ref_e, pos_e);
        EXPECT_EQ(pool.get(e), pos_e);
        EXPECT_EQ(pool.get(a), pos_a); // untouched by the insert
        EXPECT_EQ(pool.get(b), pos_b); // untouched by the insert
        EXPECT_EQ(pool.active_size(), 1); // unchanged by the new inactive insert
        EXPECT_TRUE(pool.is_active(a));
        EXPECT_FALSE(pool.is_active(b));
        EXPECT_FALSE(pool.is_active(e));
    }
}

//==============================================================================
//                        Remove Across Active Boundary
//==============================================================================

TEST_F(ComponentPoolTests, RemoveAcrossActiveBoundary)
{
    // Test 1: remove an active entity that is NOT the last active one (mid), from a mixed pool
    {
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        Position pos_a{1.0f, 0.0f, 0.0f}, pos_b{2.0f, 0.0f, 0.0f}, pos_c{3.0f, 0.0f, 0.0f}, pos_d{4.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        pool.add(b, true, pos_b);
        pool.add(c, true, pos_c);
        pool.add(d, true, pos_d);
        pool.deactivate(d); // active=[a,b,c], inactive=[d]

        pool.remove(b); // b is active, not the last active entity (c is)

        EXPECT_EQ(pool.active_size(), 2);
        EXPECT_FALSE(pool.contains(b));
        EXPECT_EQ(pool.get(a), pos_a);
        EXPECT_EQ(pool.get(c), pos_c);
        EXPECT_EQ(pool.get(d), pos_d);
        EXPECT_TRUE(pool.is_active(a));
        EXPECT_TRUE(pool.is_active(c));
        EXPECT_FALSE(pool.is_active(d));
    }

    // Test 2: remove an inactive entity from a mixed pool
    {
        pool.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        Position pos_a{1.0f, 0.0f, 0.0f}, pos_b{2.0f, 0.0f, 0.0f}, pos_c{3.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        pool.add(b, true, pos_b);
        pool.add(c, true, pos_c);
        pool.deactivate(c); // active=[a,b], inactive=[c]

        pool.remove(c);

        EXPECT_EQ(pool.active_size(), 2);
        EXPECT_EQ(pool.size(), 2);
        EXPECT_FALSE(pool.contains(c));
        EXPECT_EQ(pool.get(a), pos_a);
        EXPECT_EQ(pool.get(b), pos_b);
        EXPECT_TRUE(pool.is_active(a));
        EXPECT_TRUE(pool.is_active(b));
    }

    // Test 3: remove the entity that IS the last active one - the self-move-guard case
    // (`if (i != last_active) ...`) must not corrupt anything when the guard skips the move
    {
        pool.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        Position pos_a{1.0f, 0.0f, 0.0f}, pos_b{2.0f, 0.0f, 0.0f}, pos_c{3.0f, 0.0f, 0.0f}, pos_d{4.0f, 0.0f, 0.0f};

        pool.add(a, true, pos_a);
        pool.add(b, true, pos_b);
        pool.add(c, true, pos_c);
        pool.add(d, true, pos_d);
        pool.deactivate(d); // active=[a,b,c], inactive=[d]

        pool.remove(c); // c is the last active entity

        EXPECT_EQ(pool.active_size(), 2);
        EXPECT_FALSE(pool.contains(c));
        EXPECT_EQ(pool.get(a), pos_a);
        EXPECT_EQ(pool.get(b), pos_b);
        EXPECT_EQ(pool.get(d), pos_d);
        EXPECT_TRUE(pool.is_active(a));
        EXPECT_TRUE(pool.is_active(b));
        EXPECT_FALSE(pool.is_active(d));
    }

    // Tests 4-6: repeat the same three cases on a move-only component type, to exercise the
    // move/swap code paths (swap_components()/std::move) on a non-trivial type
    component_pool<MoveOnlyComponent> move_pool;

    // Test 4: remove an active entity (mid) from a mixed pool - move-only
    {
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        move_pool.add(a, true, MoveOnlyComponent(1));
        move_pool.add(b, true, MoveOnlyComponent(2));
        move_pool.add(c, true, MoveOnlyComponent(3));
        move_pool.add(d, true, MoveOnlyComponent(4));
        move_pool.deactivate(d);

        move_pool.remove(b);

        EXPECT_EQ(move_pool.active_size(), 2);
        EXPECT_FALSE(move_pool.contains(b));
        EXPECT_EQ(*move_pool.get(a).data, 1);
        EXPECT_EQ(*move_pool.get(c).data, 3);
        EXPECT_EQ(*move_pool.get(d).data, 4);
        EXPECT_TRUE(move_pool.is_active(a));
        EXPECT_TRUE(move_pool.is_active(c));
        EXPECT_FALSE(move_pool.is_active(d));
    }

    // Test 5: remove an inactive entity from a mixed pool - move-only
    {
        move_pool.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        move_pool.add(a, true, MoveOnlyComponent(1));
        move_pool.add(b, true, MoveOnlyComponent(2));
        move_pool.add(c, true, MoveOnlyComponent(3));
        move_pool.deactivate(c);

        move_pool.remove(c);

        EXPECT_EQ(move_pool.active_size(), 2);
        EXPECT_EQ(move_pool.size(), 2);
        EXPECT_FALSE(move_pool.contains(c));
        EXPECT_EQ(*move_pool.get(a).data, 1);
        EXPECT_EQ(*move_pool.get(b).data, 2);
    }

    // Test 6: remove the last active entity - move-only, exercises the self-move guard
    {
        move_pool.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        move_pool.add(a, true, MoveOnlyComponent(1));
        move_pool.add(b, true, MoveOnlyComponent(2));
        move_pool.add(c, true, MoveOnlyComponent(3));
        move_pool.add(d, true, MoveOnlyComponent(4));
        move_pool.deactivate(d);

        move_pool.remove(c);

        EXPECT_EQ(move_pool.active_size(), 2);
        EXPECT_FALSE(move_pool.contains(c));
        EXPECT_EQ(*move_pool.get(a).data, 1);
        EXPECT_EQ(*move_pool.get(b).data, 2);
        EXPECT_EQ(*move_pool.get(d).data, 4);
        EXPECT_TRUE(move_pool.is_active(a));
        EXPECT_TRUE(move_pool.is_active(b));
        EXPECT_FALSE(move_pool.is_active(d));
    }
}

//==============================================================================
//                        Const Access
//==============================================================================

TEST_F(ComponentPoolTests, GetConst)
{
    auto e = entity::create(42, 0);

    pool.add(e, true, Position{1.0f, 2.0f, 3.0f});

    const auto &const_pool = pool;
    const Position &comp = const_pool.get(e);

    EXPECT_EQ(comp.x, 1.0f);
    static_assert(std::is_same_v<decltype(comp), const Position &>);
}

//==============================================================================
//                        Iteration
//==============================================================================

TEST_F(ComponentPoolTests, ForEachIteration)
{
    std::vector<entity> entities;

    for (std::uint32_t i = 0; i < 10; ++i)
    {
        auto e = entity::create(i, 0);
        entities.push_back(e);
        pool.add(e, true, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    int count = 0;
    pool.for_each([&count, &entities](entity e, Position &pos)
    {
        EXPECT_EQ(e, entities[count]);
        EXPECT_EQ(pos.x, static_cast<float>(count));
        ++count;
    });

    EXPECT_EQ(count, 10);
}

//==============================================================================
//                        For Each All (Full Scan Across Partitions)
//==============================================================================

TEST_F(ComponentPoolTests, ForEachAll)
{
    std::vector<entity> entities;

    for (std::uint32_t i = 0; i < 6; ++i)
    {
        auto e = entity::create(i, 0);
        entities.push_back(e);
        pool.add(e, true, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    // Deactivate a subset
    pool.deactivate(entities[1]);
    pool.deactivate(entities[4]);

    std::vector<entity> expected_active;
    for (auto e : entities)
        if (pool.is_active(e))
            expected_active.push_back(e);
    std::sort(expected_active.begin(), expected_active.end());

    std::vector<entity> expected_all = entities;
    std::sort(expected_all.begin(), expected_all.end());

    // Test 1: mutable for_each() visits exactly the active partition
    {
        std::vector<entity> visited;
        pool.for_each([&visited](entity e, Position &)
        {
            visited.push_back(e);
        });
        std::sort(visited.begin(), visited.end());
        EXPECT_EQ(visited, expected_active);
    }

    // Test 2: mutable for_each_all() visits every entity, active and inactive
    {
        std::vector<entity> visited;
        pool.for_each_all([&visited](entity e, Position &)
        {
            visited.push_back(e);
        });
        std::sort(visited.begin(), visited.end());
        EXPECT_EQ(visited, expected_all);
    }

    const component_pool<Position> &const_pool = pool;

    // Test 3: const for_each() visits exactly the active partition
    {
        std::vector<entity> visited;
        const_pool.for_each([&visited](entity e, const Position &)
        {
            visited.push_back(e);
        });
        std::sort(visited.begin(), visited.end());
        EXPECT_EQ(visited, expected_active);
    }

    // Test 4: const for_each_all() visits every entity, active and inactive
    {
        std::vector<entity> visited;
        const_pool.for_each_all([&visited](entity e, const Position &)
        {
            visited.push_back(e);
        });
        std::sort(visited.begin(), visited.end());
        EXPECT_EQ(visited, expected_all);
    }
}

//==============================================================================
//                        Perfect Forwarding
//==============================================================================

TEST_F(ComponentPoolTests, PerfectForwarding)
{
    component_pool<MoveOnlyComponent> move_pool;
    auto e = entity::create(42, 0);

    MoveOnlyComponent comp(123);
    move_pool.add(e, true, std::move(comp));

    EXPECT_TRUE(move_pool.contains(e));
    EXPECT_EQ(*move_pool.get(e).data, 123);
}

//==============================================================================
//                        Component Lifecycle (RAII)
//==============================================================================

TEST_F(ComponentPoolTests, ComponentLifecycle)
{
    component_pool<LifetimeTracker> trackers;
    int counter = 0;

    auto e1 = entity::create(1, 0);
    auto e2 = entity::create(2, 0);
    auto e3 = entity::create(3, 0);

    trackers.add(e1, true, LifetimeTracker{&counter});
    trackers.add(e2, true, LifetimeTracker{&counter});
    trackers.add(e3, true, LifetimeTracker{&counter});

    EXPECT_EQ(counter, 3);

    trackers.remove(e2);
    EXPECT_EQ(counter, 2);

    trackers.clear();
    EXPECT_EQ(counter, 0);
}

//==============================================================================
//                        Clear Pool
//==============================================================================

TEST_F(ComponentPoolTests, ClearPool)
{
    for (std::uint32_t i = 0; i < 20; ++i)
        pool.add(entity::create(i, 0), true, Position{0.0f, 0.0f, 0.0f});

    EXPECT_EQ(pool.size(), 20);

    pool.clear();

    EXPECT_EQ(pool.size(), 0);
    EXPECT_TRUE(pool.empty());

    for (std::uint32_t i = 0; i < 20; ++i)
        EXPECT_FALSE(pool.contains(entity::create(i, 0)));
}
