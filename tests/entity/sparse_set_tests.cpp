#include <gtest/gtest.h>
#include <gamecoe/entity/sparse_set.hpp>
#include <vector>

using namespace gamecoe;

//==============================================================================
//                    SparseSetTests - Sparse set data structure tests
//==============================================================================

class SparseSetTests : public ::testing::Test
{
protected:
    sparse_set set;
};

//==============================================================================
//                        Insert Operations
//==============================================================================

TEST_F(SparseSetTests, InsertOperations)
{
    // Test 1: Insert single entity
    {
        auto e = entity::create(42, 0);

        set.insert(e);

        EXPECT_TRUE(set.contains(e));
        EXPECT_EQ(set.size(), 1);
        EXPECT_FALSE(set.empty());

        auto index = set.index(e);
        EXPECT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 0);
    }

    // Test 2: Insert multiple entities
    {
        set.clear();
        std::vector<entity> entities;

        // Insert 100 entities
        for (std::uint32_t i = 0; i < 100; ++i)
            entities.push_back(entity::create(i, 0));

        for (auto e : entities)
            set.insert(e);

        EXPECT_EQ(set.size(), 100);

        // Verify all are contained
        for (auto e : entities)
            EXPECT_TRUE(set.contains(e));
    }

    // Test 3: Insert duplicate (no-op)
    {
        set.clear();
        auto e = entity::create(42, 0);

        set.insert(e);
        set.insert(e); // Duplicate insert

        EXPECT_EQ(set.size(), 1); // Size unchanged
        EXPECT_TRUE(set.contains(e));
    }
}

//==============================================================================
//                        Erase Operations
//==============================================================================

TEST_F(SparseSetTests, EraseOperations)
{
    // Test 1: Erase single entity
    {
        auto e = entity::create(42, 0);

        set.insert(e);
        set.erase(e);

        EXPECT_FALSE(set.contains(e));
        EXPECT_EQ(set.size(), 0);
        EXPECT_TRUE(set.empty());

        auto index = set.index(e);
        EXPECT_FALSE(index.has_value());
    }

    // Test 2: Erase with swap-and-pop
    {
        set.clear();
        auto e1 = entity::create(10, 0);
        auto e2 = entity::create(20, 0);
        auto e3 = entity::create(30, 0);

        set.insert(e1);
        set.insert(e2);
        set.insert(e3);

        // Erase middle entity (e2)
        set.erase(e2);

        EXPECT_EQ(set.size(), 2);
        EXPECT_TRUE(set.contains(e1));
        EXPECT_FALSE(set.contains(e2));
        EXPECT_TRUE(set.contains(e3));

        // e3 should have been swapped to e2's old position
        EXPECT_EQ(set.index(e1).value(), 0);
        EXPECT_EQ(set.index(e3).value(), 1); // e3 moved to index 1 (e2's old position)
    }

    // Test 3: Erase non-existent entity (no-op)
    {
        set.clear();
        auto e1 = entity::create(10, 0);
        auto e2 = entity::create(20, 0);

        set.insert(e1);
        EXPECT_EQ(set.size(), 1);

        set.erase(e2); // e2 not in set

        EXPECT_EQ(set.size(), 1); // Size unchanged
        EXPECT_TRUE(set.contains(e1));
    }
}

//==============================================================================
//                        Erase At (by dense index)
//==============================================================================

TEST_F(SparseSetTests, EraseAtOperations)
{
    // Test 1: Erase at with swap-and-pop
    {
        auto e1 = entity::create(10, 0);
        auto e2 = entity::create(20, 0);
        auto e3 = entity::create(30, 0);

        set.insert(e1);
        set.insert(e2);
        set.insert(e3);

        // Erase middle entity by index (e2 is at dense index 1)
        set.erase_at(1);

        EXPECT_EQ(set.size(), 2);
        EXPECT_TRUE(set.contains(e1));
        EXPECT_FALSE(set.contains(e2));
        EXPECT_TRUE(set.contains(e3));

        // e3 should have been swapped to index 1 (e2's old position)
        EXPECT_EQ(set.index(e1).value(), 0);
        EXPECT_EQ(set.index(e3).value(), 1);

        // Erase last element by index (no swap needed)
        set.erase_at(1); // e3 is now at index 1 (last)

        EXPECT_EQ(set.size(), 1);
        EXPECT_TRUE(set.contains(e1));
        EXPECT_FALSE(set.contains(e3));
    }

    // Test 2: Erase at out of bounds (no-op)
    {
        set.clear();
        auto e = entity::create(42, 0);
        set.insert(e);

        set.erase_at(5); // Out of bounds, should be no-op

        EXPECT_EQ(set.size(), 1);
        EXPECT_TRUE(set.contains(e));
    }
}

//==============================================================================
//                        Generation Handling
//==============================================================================

TEST_F(SparseSetTests, GenerationMismatch)
{
    // Create two entities with same ID but different generations
    auto e_gen0 = entity::create(42, 0);
    auto e_gen1 = entity::create(42, 1);

    set.insert(e_gen0);

    // e_gen0 is contained, but e_gen1 (same id, different gen) is NOT
    EXPECT_TRUE(set.contains(e_gen0));
    EXPECT_FALSE(set.contains(e_gen1)); // Generation mismatch!

    // Index should return nullopt for wrong generation
    EXPECT_TRUE(set.index(e_gen0).has_value());
    EXPECT_FALSE(set.index(e_gen1).has_value());
}

//==============================================================================
//                        Paging Behavior
//==============================================================================

TEST_F(SparseSetTests, PagingBehavior)
{
    // Insert entities spanning multiple pages (page size = 1024)
    auto e0 = entity::create(0, 0);       // Page 0
    auto e1024 = entity::create(1024, 0); // Page 1
    auto e2048 = entity::create(2048, 0); // Page 2
    auto e5000 = entity::create(5000, 0); // Page 4

    set.insert(e0);
    set.insert(e1024);
    set.insert(e2048);
    set.insert(e5000);

    EXPECT_EQ(set.size(), 4);
    EXPECT_TRUE(set.contains(e0));
    EXPECT_TRUE(set.contains(e1024));
    EXPECT_TRUE(set.contains(e2048));
    EXPECT_TRUE(set.contains(e5000));
}

//==============================================================================
//                        Clear and Reset
//==============================================================================

TEST_F(SparseSetTests, ClearSet)
{
    for (std::uint32_t i = 0; i < 50; ++i)
        set.insert(entity::create(i, 0));

    EXPECT_EQ(set.size(), 50);

    set.clear();

    EXPECT_EQ(set.size(), 0);
    EXPECT_TRUE(set.empty());

    // Verify previously inserted entities are no longer contained
    for (std::uint32_t i = 0; i < 50; ++i)
        EXPECT_FALSE(set.contains(entity::create(i, 0)));
}

//==============================================================================
//                        Iteration
//==============================================================================

TEST_F(SparseSetTests, IterateDense)
{
    std::vector<entity> entities;

    // Insert 10 entities
    for (std::uint32_t i = 0; i < 10; ++i)
    {
        auto e = entity::create(i * 10, 0); // IDs: 0, 10, 20, ..., 90
        entities.push_back(e);
        set.insert(e);
    }

    // Iterate using begin()/end()
    std::vector<entity> iterated_entities;
    for (auto it = set.begin(); it != set.end(); ++it)
        iterated_entities.push_back(*it);

    EXPECT_EQ(iterated_entities.size(), 10);

    // Verify all entities were iterated (order matches insertion order)
    for (std::size_t i = 0; i < entities.size(); ++i)
        EXPECT_EQ(iterated_entities[i], entities[i]);
}

//==============================================================================
//                        Move Semantics
//==============================================================================

TEST_F(SparseSetTests, MoveSemantics)
{
    sparse_set set1;
    auto e1 = entity::create(10, 0);
    auto e2 = entity::create(20, 0);

    set1.insert(e1);
    set1.insert(e2);
    EXPECT_EQ(set1.size(), 2);

    // Move constructor
    sparse_set set2(std::move(set1));
    EXPECT_EQ(set2.size(), 2);
    EXPECT_EQ(set1.size(), 0);
    EXPECT_TRUE(set2.contains(e1));
    EXPECT_TRUE(set2.contains(e2));

    // Move assignment
    sparse_set set3;
    set3 = std::move(set2);
    EXPECT_EQ(set3.size(), 2);
    EXPECT_TRUE(set3.contains(e1));
    EXPECT_TRUE(set3.contains(e2));
}

//==============================================================================
//                        Capacity Management
//==============================================================================

TEST_F(SparseSetTests, ReserveCapacity)
{
    // Reserve capacity for 1000 entities
    set.reserve(1000);

    // Insert 500 entities (should not trigger reallocation)
    for (std::uint32_t i = 0; i < 500; ++i)
        set.insert(entity::create(i, 0));

    EXPECT_EQ(set.size(), 500);

    // Verify all entities are accessible
    for (std::uint32_t i = 0; i < 500; ++i)
        EXPECT_TRUE(set.contains(entity::create(i, 0)));
}
