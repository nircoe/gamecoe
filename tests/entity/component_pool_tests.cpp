#include <gtest/gtest.h>
#include <gamecoe/entity/component_pool.hpp>
#include <memory>

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
