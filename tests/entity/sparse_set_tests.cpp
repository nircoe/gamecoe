#include <gtest/gtest.h>
#include <gamecoe/entity/sparse_set.hpp>
#include <algorithm>
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

        set.insert(e, true);

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
            set.insert(e, true);

        EXPECT_EQ(set.size(), 100);

        // Verify all are contained
        for (auto e : entities)
            EXPECT_TRUE(set.contains(e));
    }

    // Test 3: Insert duplicate (no-op)
    {
        set.clear();
        auto e = entity::create(42, 0);

        set.insert(e, true);
        set.insert(e, true); // Duplicate insert

        EXPECT_EQ(set.size(), 1); // Size unchanged
        EXPECT_TRUE(set.contains(e));
    }

    // Test 4: Insert inactive entity into an empty set
    {
        set.clear();
        auto e = entity::create(42, 0);

        set.insert(e, false);

        EXPECT_EQ(set.active_size(), 0);
        EXPECT_FALSE(set.is_active(set.index(e).value()));
        EXPECT_TRUE(set.contains(e));

        auto index = set.index(e);
        EXPECT_TRUE(index.has_value());
    }

    // Test 5: Insert inactive entity into a set with existing active AND inactive entries
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.deactivate(b); // active=[a,c], inactive=[b]
        ASSERT_EQ(set.active_size(), 2);

        auto d = entity::create(4, 0);
        set.insert(d, false);

        EXPECT_EQ(set.active_size(), 2); // unchanged by the new inactive insert
        EXPECT_EQ(set.size(), 4);
        EXPECT_TRUE(set.contains(d));
        EXPECT_FALSE(set.is_active(set.index(d).value()));

        // Existing partition undisturbed
        EXPECT_TRUE(set.is_active(set.index(a).value()));
        EXPECT_TRUE(set.is_active(set.index(c).value()));
        EXPECT_FALSE(set.is_active(set.index(b).value()));
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

        set.insert(e, true);
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

        set.insert(e1, true);
        set.insert(e2, true);
        set.insert(e3, true);

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

        set.insert(e1, true);
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

        set.insert(e1, true);
        set.insert(e2, true);
        set.insert(e3, true);

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
        set.insert(e, true);

        set.erase_at(5); // Out of bounds, should be no-op

        EXPECT_EQ(set.size(), 1);
        EXPECT_TRUE(set.contains(e));
    }
}

//==============================================================================
//                        Active/Inactive Partition
//==============================================================================

TEST_F(SparseSetTests, ActivePartition)
{
    // Test 1: fresh inserts are all active
    {
        std::vector<entity> entities;
        for (std::uint32_t i = 0; i < 5; ++i)
        {
            auto e = entity::create(i, 0);
            entities.push_back(e);
            set.insert(e, true);
        }

        EXPECT_EQ(set.active_size(), set.size());
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(set.size()); ++i)
            EXPECT_TRUE(set.is_active(i));
    }

    // Tests 2-5: deactivate/activate toggling on a single entity, including no-op repeats
    set.clear();
    auto e1 = entity::create(1, 0);
    auto e2 = entity::create(2, 0);
    auto e3 = entity::create(3, 0);
    set.insert(e1, true);
    set.insert(e2, true);
    set.insert(e3, true);
    auto original_active = set.active_size();

    // Test 2: deactivate(e) decrements active_size(), keeps contains() true, moves index past the boundary
    set.deactivate(e2);
    EXPECT_EQ(set.active_size(), original_active - 1);
    EXPECT_TRUE(set.contains(e2));
    EXPECT_GE(set.index(e2).value(), set.active_size());

    // Test 3: deactivate(e) again on an already-inactive entity is a no-op
    {
        auto active_before = set.active_size();
        auto index_before = set.index(e2).value();
        set.deactivate(e2);
        EXPECT_EQ(set.active_size(), active_before);
        EXPECT_EQ(set.index(e2).value(), index_before);
    }

    // Test 4: activate(e) restores it to the active partition
    set.activate(e2);
    EXPECT_LT(set.index(e2).value(), set.active_size());
    EXPECT_EQ(set.active_size(), original_active);

    // Test 5: activate(e) again on an already-active entity is a no-op
    {
        auto active_before = set.active_size();
        auto index_before = set.index(e2).value();
        set.activate(e2);
        EXPECT_EQ(set.active_size(), active_before);
        EXPECT_EQ(set.index(e2).value(), index_before);
    }

    // Test 6: deactivating every entity drops active_size() to 0, size() unchanged, all still contained
    {
        set.deactivate(e1);
        set.deactivate(e2);
        set.deactivate(e3);

        EXPECT_EQ(set.active_size(), 0);
        EXPECT_EQ(set.size(), 3);
        EXPECT_TRUE(set.contains(e1));
        EXPECT_TRUE(set.contains(e2));
        EXPECT_TRUE(set.contains(e3));
    }

    // Test 7: partition sweep - deactivating a subset never loses or duplicates entities across [0, size())
    {
        set.clear();
        std::vector<entity> inserted;
        for (std::uint32_t i = 0; i < 5; ++i)
        {
            auto e = entity::create(i, 0);
            inserted.push_back(e);
            set.insert(e, true);
        }

        // Deactivate every other entity
        std::vector<entity> expected_active;
        std::vector<entity> expected_inactive;
        for (std::size_t i = 0; i < inserted.size(); ++i)
        {
            if (i % 2 == 0)
                expected_active.push_back(inserted[i]);
            else
            {
                set.deactivate(inserted[i]);
                expected_inactive.push_back(inserted[i]);
            }
        }

        for (auto e : expected_active)
            EXPECT_LT(set.index(e).value(), set.active_size());
        for (auto e : expected_inactive)
            EXPECT_GE(set.index(e).value(), set.active_size());

        // No entity lost or duplicated across the full dense range
        std::vector<entity> all_by_dense_index;
        for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(set.size()); ++i)
            all_by_dense_index.push_back(set.get_entity_at_index(i));

        std::sort(all_by_dense_index.begin(), all_by_dense_index.end());
        std::vector<entity> sorted_inserted = inserted;
        std::sort(sorted_inserted.begin(), sorted_inserted.end());
        EXPECT_EQ(all_by_dense_index, sorted_inserted);
    }
}

//==============================================================================
//                        Erase Across Active Boundary
//==============================================================================

TEST_F(SparseSetTests, EraseAcrossActiveBoundary)
{
    // Test 1: erasing an active entity while another is inactive must route through the
    // boundary first - regression guard for the swap-and-pop dropping an inactive entity
    // into the active partition (or vice versa)
    {
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.deactivate(c); // active=[a,b], inactive=[c]
        ASSERT_EQ(set.active_size(), 2);

        set.erase(a);

        // Pre-fix bug candidate: active_size() left at 2 with c incorrectly landing active
        constexpr std::size_t buggy_active_size = 2;
        EXPECT_NE(set.active_size(), buggy_active_size);
        EXPECT_EQ(set.active_size(), 1);

        EXPECT_TRUE(set.is_active(set.index(b).value()));
        EXPECT_FALSE(set.is_active(set.index(c).value()));
        EXPECT_TRUE(set.contains(b));
        EXPECT_TRUE(set.contains(c));
        EXPECT_FALSE(set.contains(a));
    }

    // Test 2: erase an active entity that is NOT the last active one (mid)
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.insert(d, true);
        set.deactivate(d); // active=[a,b,c], inactive=[d]

        set.erase(b); // b is active, not the last active entity (c is)

        EXPECT_EQ(set.active_size(), 2);
        EXPECT_FALSE(set.contains(b));
        EXPECT_TRUE(set.contains(a));
        EXPECT_TRUE(set.contains(c));
        EXPECT_TRUE(set.contains(d));
        EXPECT_TRUE(set.is_active(set.index(a).value()));
        EXPECT_TRUE(set.is_active(set.index(c).value()));
        EXPECT_FALSE(set.is_active(set.index(d).value()));
    }

    // Test 3: erase the entity that IS the last active one
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.insert(d, true);
        set.deactivate(d); // active=[a,b,c], inactive=[d]

        set.erase(c); // c is the last active entity

        EXPECT_EQ(set.active_size(), 2);
        EXPECT_FALSE(set.contains(c));
        EXPECT_TRUE(set.contains(a));
        EXPECT_TRUE(set.contains(b));
        EXPECT_TRUE(set.contains(d));
        EXPECT_TRUE(set.is_active(set.index(a).value()));
        EXPECT_TRUE(set.is_active(set.index(b).value()));
        EXPECT_FALSE(set.is_active(set.index(d).value()));
    }

    // Test 4: erase an inactive entity that is NOT the dense-back element
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);
        auto e = entity::create(5, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.insert(d, true);
        set.insert(e, true);
        set.deactivate(c);
        set.deactivate(d); // active=[a,b,e], inactive=[d,c] with d NOT at the dense-back position

        ASSERT_FALSE(set.is_active(set.index(d).value()));
        ASSERT_NE(set.index(d).value(), static_cast<std::uint32_t>(set.size() - 1)); // d is not the dense-back element

        set.erase(d);

        EXPECT_EQ(set.active_size(), 3);
        EXPECT_FALSE(set.contains(d));
        EXPECT_TRUE(set.contains(a));
        EXPECT_TRUE(set.contains(b));
        EXPECT_TRUE(set.contains(c));
        EXPECT_TRUE(set.contains(e));
        EXPECT_FALSE(set.is_active(set.index(c).value()));
    }

    // Test 5: erase an inactive entity that IS the dense-back element
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.deactivate(c); // active=[a,b], inactive=[c], c is the dense-back element

        ASSERT_EQ(set.index(c).value(), static_cast<std::uint32_t>(set.size() - 1));

        set.erase(c);

        EXPECT_EQ(set.active_size(), 2);
        EXPECT_EQ(set.size(), 2);
        EXPECT_TRUE(set.contains(a));
        EXPECT_TRUE(set.contains(b));
        EXPECT_FALSE(set.contains(c));
    }

    // Test 6: erase the only entity in the set (when it's active)
    {
        set.clear();
        auto a = entity::create(1, 0);
        set.insert(a, true);

        set.erase(a);

        EXPECT_EQ(set.size(), 0);
        EXPECT_EQ(set.active_size(), 0);
        EXPECT_TRUE(set.empty());
        EXPECT_FALSE(set.contains(a));
    }

    // Test 7: erase-all in mixed active/inactive order ends in size() == 0 && active_size() == 0
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);
        auto d = entity::create(4, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);
        set.insert(d, true);
        set.deactivate(b);
        set.deactivate(d); // active=[a,c], inactive contains b and d

        set.erase(d); // inactive
        set.erase(a); // active
        set.erase(c); // active
        set.erase(b); // inactive

        EXPECT_EQ(set.size(), 0);
        EXPECT_EQ(set.active_size(), 0);
        EXPECT_TRUE(set.empty());
        EXPECT_FALSE(set.contains(a));
        EXPECT_FALSE(set.contains(b));
        EXPECT_FALSE(set.contains(c));
        EXPECT_FALSE(set.contains(d));
    }

    // Test 8: entirely-active set (no inactive entities at all) - confirm erase still
    // behaves exactly as before this ticket (regression check for the plain case)
    {
        set.clear();
        auto a = entity::create(1, 0);
        auto b = entity::create(2, 0);
        auto c = entity::create(3, 0);

        set.insert(a, true);
        set.insert(b, true);
        set.insert(c, true);

        set.erase(b); // mid element, everything active

        EXPECT_EQ(set.size(), 2);
        EXPECT_EQ(set.active_size(), 2);
        EXPECT_TRUE(set.contains(a));
        EXPECT_FALSE(set.contains(b));
        EXPECT_TRUE(set.contains(c));
        EXPECT_EQ(set.index(a).value(), 0);
        EXPECT_EQ(set.index(c).value(), 1);
        EXPECT_TRUE(set.is_active(set.index(a).value()));
        EXPECT_TRUE(set.is_active(set.index(c).value()));
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

    set.insert(e_gen0, true);

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

    set.insert(e0, true);
    set.insert(e1024, true);
    set.insert(e2048, true);
    set.insert(e5000, true);

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
        set.insert(entity::create(i, 0), true);

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
        set.insert(e, true);
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

    set1.insert(e1, true);
    set1.insert(e2, true);
    set1.deactivate(e2);
    EXPECT_EQ(set1.size(), 2);
    EXPECT_EQ(set1.active_size(), 1);

    // Move constructor
    sparse_set set2(std::move(set1));
    EXPECT_EQ(set2.size(), 2);
    EXPECT_EQ(set2.active_size(), 1);
    EXPECT_EQ(set1.size(), 0);
    EXPECT_EQ(set1.active_size(), 0);
    EXPECT_TRUE(set2.contains(e1));
    EXPECT_TRUE(set2.contains(e2));

    // Move assignment
    sparse_set set3;
    set3 = std::move(set2);
    EXPECT_EQ(set3.size(), 2);
    EXPECT_EQ(set3.active_size(), 1);
    EXPECT_EQ(set2.size(), 0);
    EXPECT_EQ(set2.active_size(), 0);
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
        set.insert(entity::create(i, 0), true);

    EXPECT_EQ(set.size(), 500);

    // Verify all entities are accessible
    for (std::uint32_t i = 0; i < 500; ++i)
        EXPECT_TRUE(set.contains(entity::create(i, 0)));
}
