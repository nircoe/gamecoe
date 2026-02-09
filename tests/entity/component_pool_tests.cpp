#include <gtest/gtest.h>
#include <gamecoe/entity/component_pool.hpp>
#include <memory>
#include <string>

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

    // Move-only (no copy)
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
    ~LifetimeTracker() { --(*counter); }

    // Copyable for swap-and-pop operations
    LifetimeTracker(const LifetimeTracker &other) : counter(other.counter) { ++(*counter); }
    LifetimeTracker &operator=(const LifetimeTracker &other)
    {
        if (this != &other)
        {
            if (counter)
                --(*counter);
            counter = other.counter;
            ++(*counter);
        }
        return *this;
    }

    // Moveable
    LifetimeTracker(LifetimeTracker &&other) noexcept : counter(other.counter)
    {
        other.counter = nullptr;
    }
    LifetimeTracker &operator=(LifetimeTracker &&other) noexcept
    {
        if (this != &other)
        {
            if (counter)
                --(*counter);
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
//                        Add and Get Components
//==============================================================================

TEST_F(ComponentPoolTests, AddComponent)
{
    auto e = entity::create(42, 0);

    Position &pos = pool.add(e, Position{1.0f, 2.0f, 3.0f});

    EXPECT_TRUE(pool.contains(e));
    EXPECT_EQ(pool.size(), 1);
    EXPECT_FALSE(pool.empty());

    auto retrieved = pool.get(e);
    EXPECT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->get(), pos);
    EXPECT_EQ(retrieved->get().x, 1.0f);
    EXPECT_EQ(retrieved->get().y, 2.0f);
    EXPECT_EQ(retrieved->get().z, 3.0f);
}

TEST_F(ComponentPoolTests, AddMultiple)
{
    std::vector<entity> entities;

    // Add 50 components
    for (std::uint32_t i = 0; i < 50; ++i)
    {
        auto e = entity::create(i, 0);
        entities.push_back(e);
        pool.add(e, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    EXPECT_EQ(pool.size(), 50);

    // Verify all are accessible
    for (std::size_t i = 0; i < entities.size(); ++i)
    {
        auto retrieved = pool.get(entities[i]);
        EXPECT_TRUE(retrieved.has_value());
        EXPECT_EQ(retrieved->get().x, static_cast<float>(i));
    }
}

TEST_F(ComponentPoolTests, ContainsCheck)
{
    auto e = entity::create(42, 0);

    EXPECT_FALSE(pool.contains(e));

    pool.add(e, Position{1.0f, 2.0f, 3.0f});

    EXPECT_TRUE(pool.contains(e));
    EXPECT_EQ(pool.size(), 1);

    // Note: Adding duplicate component triggers assertion in debug builds
    // This is a programmer error, not a runtime behavior to test
}

//==============================================================================
//                        Remove Components
//==============================================================================

TEST_F(ComponentPoolTests, RemoveComponent)
{
    auto e = entity::create(42, 0);

    pool.add(e, Position{1.0f, 2.0f, 3.0f});
    pool.remove(e);

    EXPECT_FALSE(pool.contains(e));
    EXPECT_EQ(pool.size(), 0);
    EXPECT_TRUE(pool.empty());

    auto retrieved = pool.get(e);
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(ComponentPoolTests, RemoveSwapAndPop)
{
    auto e1 = entity::create(10, 0);
    auto e2 = entity::create(20, 0);
    auto e3 = entity::create(30, 0);

    pool.add(e1, Position{1.0f, 0.0f, 0.0f});
    pool.add(e2, Position{2.0f, 0.0f, 0.0f});
    pool.add(e3, Position{3.0f, 0.0f, 0.0f});

    // Remove middle entity (e2)
    pool.remove(e2);

    EXPECT_EQ(pool.size(), 2);
    EXPECT_TRUE(pool.contains(e1));
    EXPECT_FALSE(pool.contains(e2));
    EXPECT_TRUE(pool.contains(e3));

    // Verify e3's component data is still correct (swapped to e2's position)
    auto e3_comp = pool.get(e3);
    EXPECT_TRUE(e3_comp.has_value());
    EXPECT_EQ(e3_comp->get().x, 3.0f);
}

//==============================================================================
//                        Mutable and Const Access
//==============================================================================

TEST_F(ComponentPoolTests, GetMutable)
{
    auto e = entity::create(42, 0);

    pool.add(e, Position{1.0f, 2.0f, 3.0f});

    auto comp = pool.get(e);
    EXPECT_TRUE(comp.has_value());

    // Modify component
    comp->get().x = 99.0f;

    // Verify changes persist
    auto retrieved = pool.get(e);
    EXPECT_EQ(retrieved->get().x, 99.0f);
}

TEST_F(ComponentPoolTests, GetConst)
{
    auto e = entity::create(42, 0);

    pool.add(e, Position{1.0f, 2.0f, 3.0f});

    const auto &const_pool = pool;
    auto comp = const_pool.get(e);

    EXPECT_TRUE(comp.has_value());
    EXPECT_EQ(comp->get().x, 1.0f);

    // Verify const reference wrapper
    static_assert(std::is_same_v<decltype(comp), std::optional<std::reference_wrapper<const Position>>>);
}

//==============================================================================
//                        Iteration
//==============================================================================

TEST_F(ComponentPoolTests, ForEachIteration)
{
    std::vector<entity> entities;

    // Add 10 components
    for (std::uint32_t i = 0; i < 10; ++i)
    {
        auto e = entity::create(i, 0);
        entities.push_back(e);
        pool.add(e, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    // Iterate using for_each
    int count = 0;
    pool.for_each([&count, &entities](entity e, Position &pos)
                  {
        EXPECT_EQ(e, entities[count]);
        EXPECT_EQ(pos.x, static_cast<float>(count));
        ++count; });

    EXPECT_EQ(count, 10);
}

//==============================================================================
//                        Perfect Forwarding
//==============================================================================

TEST_F(ComponentPoolTests, PerfectForwarding)
{
    component_pool<MoveOnlyComponent> move_pool;
    auto e = entity::create(42, 0);

    // Add move-only component using perfect forwarding
    MoveOnlyComponent comp(123);
    move_pool.add(e, std::move(comp));

    EXPECT_TRUE(move_pool.contains(e));

    auto retrieved = move_pool.get(e);
    EXPECT_TRUE(retrieved.has_value());
    EXPECT_EQ(*retrieved->get().data, 123);
}

//==============================================================================
//                        Clear Pool
//==============================================================================

TEST_F(ComponentPoolTests, ClearPool)
{
    // Add components
    for (std::uint32_t i = 0; i < 20; ++i)
        pool.add(entity::create(i, 0), Position{0.0f, 0.0f, 0.0f});

    EXPECT_EQ(pool.size(), 20);

    pool.clear();

    EXPECT_EQ(pool.size(), 0);
    EXPECT_TRUE(pool.empty());

    // Verify entities no longer have components
    for (std::uint32_t i = 0; i < 20; ++i)
        EXPECT_FALSE(pool.contains(entity::create(i, 0)));
}
