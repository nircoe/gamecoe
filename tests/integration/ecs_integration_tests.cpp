#include <gtest/gtest.h>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <chrono>

using namespace gamecoe;

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

struct Health
{
    int value;
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

    // Copyable for swap-and-pop
    LifetimeTracker(const LifetimeTracker &other) : counter(other.counter)
    {
        if (counter)
            ++(*counter);
    }
    LifetimeTracker &operator=(const LifetimeTracker &other)
    {
        if (this != &other)
        {
            if (counter)
                --(*counter);
            counter = other.counter;
            if (counter)
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
//                    ECSIntegrationTests - ECS component interaction tests
//==============================================================================

class ECSIntegrationTests : public ::testing::Test
{
protected:
    component_pool<Position> positions;
    component_pool<Velocity> velocities;
    component_pool<Health> healths;
};

//==============================================================================
//                        Multi-Component Entity
//==============================================================================

TEST_F(ECSIntegrationTests, MultiComponentEntity)
{
    auto e = entity::create(42, 0);

    // Add 3 different component types to same entity
    positions.add(e, Position{1.0f, 2.0f, 3.0f});
    velocities.add(e, Velocity{0.1f, 0.2f, 0.3f});
    healths.add(e, Health{100});

    // Verify all components are accessible
    EXPECT_TRUE(positions.contains(e));
    EXPECT_TRUE(velocities.contains(e));
    EXPECT_TRUE(healths.contains(e));

    auto pos = positions.get(e);
    auto vel = velocities.get(e);
    auto hp = healths.get(e);

    EXPECT_TRUE(pos.has_value());
    EXPECT_TRUE(vel.has_value());
    EXPECT_TRUE(hp.has_value());

    EXPECT_EQ(pos->get().x, 1.0f);
    EXPECT_EQ(vel->get().dx, 0.1f);
    EXPECT_EQ(hp->get().value, 100);
}

//==============================================================================
//                        Component Lifecycle (RAII)
//==============================================================================

TEST_F(ECSIntegrationTests, ComponentLifecycle)
{
    component_pool<LifetimeTracker> trackers;
    int counter = 0;

    auto e1 = entity::create(1, 0);
    auto e2 = entity::create(2, 0);
    auto e3 = entity::create(3, 0);

    // Add components (counter should increment)
    trackers.add(e1, LifetimeTracker{&counter});
    trackers.add(e2, LifetimeTracker{&counter});
    trackers.add(e3, LifetimeTracker{&counter});

    EXPECT_EQ(counter, 3);

    // Remove e2 (counter should decrement)
    trackers.remove(e2);
    EXPECT_EQ(counter, 2);

    // Clear all (counter should go to 0)
    trackers.clear();
    EXPECT_EQ(counter, 0);
}

//==============================================================================
//                        Bulk Operations Performance
//==============================================================================

TEST_F(ECSIntegrationTests, BulkOperations)
{
    const std::size_t NUM_ENTITIES = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    // Add 1000 entities with components
    for (std::uint32_t i = 0; i < NUM_ENTITIES; ++i)
    {
        auto e = entity::create(i, 0);
        positions.add(e, Position{static_cast<float>(i), 0.0f, 0.0f});
    }

    // Iterate all entities
    int count = 0;
    positions.for_each([&count](entity e, Position &pos)
                       {
        pos.x += 1.0f; // Simple transform
        ++count; });

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(count, NUM_ENTITIES);
    EXPECT_LT(duration.count(), 10); // Should complete in < 10ms

    // Verify all entities were updated
    for (std::uint32_t i = 0; i < NUM_ENTITIES; ++i)
    {
        auto e = entity::create(i, 0);
        auto pos = positions.get(e);
        EXPECT_TRUE(pos.has_value());
        EXPECT_EQ(pos->get().x, static_cast<float>(i) + 1.0f);
    }
}

//==============================================================================
//                        Invalid Entity Handling
//==============================================================================

TEST_F(ECSIntegrationTests, InvalidEntityHandling)
{
    auto invalid = entity::invalid();

    // Operations on invalid entity should be safe
    EXPECT_FALSE(positions.contains(invalid));
    EXPECT_FALSE(positions.get(invalid).has_value());

    // Remove should be no-op
    positions.remove(invalid);
    EXPECT_EQ(positions.size(), 0);
}
