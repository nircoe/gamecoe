#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
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

TEST_F(EntitiesTests, CreateAndValid)
{
    entity e = mgr.create();

    EXPECT_TRUE(mgr.valid(e));
    EXPECT_TRUE(e != entity::invalid());
    EXPECT_EQ(mgr.size(), 1);
}

TEST_F(EntitiesTests, CreateMultiple)
{
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

TEST_F(EntitiesTests, DestroyEntity)
{
    entity e = mgr.create();
    EXPECT_EQ(mgr.size(), 1);

    mgr.destroy(e);

    EXPECT_FALSE(mgr.valid(e));
    EXPECT_EQ(mgr.size(), 0);
}

TEST_F(EntitiesTests, RecycleId)
{
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

TEST_F(EntitiesTests, ClearEntities)
{
    entity e0 = mgr.create();
    entity e1 = mgr.create();
    entity e2 = mgr.create();

    mgr.clear();

    EXPECT_EQ(mgr.size(), 0);
    EXPECT_FALSE(mgr.valid(e0));
    EXPECT_FALSE(mgr.valid(e1));
    EXPECT_FALSE(mgr.valid(e2));
}

//==============================================================================
//                        Component Operations
//==============================================================================

TEST_F(EntitiesTests, AddHasGetRemoveComponent)
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

TEST_F(EntitiesTests, GetComponentConst)
{
    entity e = mgr.create();
    mgr.add_component<Position>(e, Position{5.0f, 6.0f, 7.0f});

    const entities &const_mgr = mgr;
    const Position *pos = const_mgr.get_component<Position>(e);

    EXPECT_NE(pos, nullptr);
    EXPECT_EQ(pos->x, 5.0f);
    static_assert(std::is_same_v<decltype(pos), const Position *>);
}

TEST_F(EntitiesTests, GetComponentNullptr)
{
    entity e = mgr.create();
    entity invalid = entity::invalid();

    // Entity without the component
    EXPECT_EQ(mgr.get_component<Position>(e), nullptr);

    // Invalid entity handle
    EXPECT_EQ(mgr.get_component<Position>(invalid), nullptr);
}

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


