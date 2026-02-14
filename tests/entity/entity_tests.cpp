#include <gtest/gtest.h>
#include <gamecoe/entity/entity.hpp>
#include <set>
#include <map>
#include <unordered_set>

using namespace gamecoe;

//==============================================================================
//                    EntityTests - Entity handle tests
//==============================================================================

class EntityTests : public ::testing::Test
{
protected:
    // Test helper: custom hash for unordered containers
    struct entity_hash
    {
        std::size_t operator()(entity e) const noexcept
        {
            return std::hash<std::uint32_t>{}(e.id()) ^
                   (std::hash<std::uint16_t>{}(e.generation()) << 1);
        }
    };
};

//==============================================================================
//                        Entity Creation and Validity
//==============================================================================

TEST_F(EntityTests, CreateValidEntity)
{
    auto e = entity::create(42, 5);

    EXPECT_EQ(e.id(), 42);
    EXPECT_EQ(e.generation(), 5);
    EXPECT_TRUE(e != entity::invalid());
}

TEST_F(EntityTests, CreateBoundaryValues)
{
    // Test maximum ID (1,048,574)
    auto max_id_entity = entity::create(entity::MAX_ENTITIES, 0);
    EXPECT_EQ(max_id_entity.id(), entity::MAX_ENTITIES);
    EXPECT_EQ(max_id_entity.generation(), 0);
    EXPECT_TRUE(max_id_entity != entity::invalid());

    // Test maximum generation (4,094)
    auto max_gen_entity = entity::create(0, entity::MAX_GENERATIONS);
    EXPECT_EQ(max_gen_entity.id(), 0);
    EXPECT_EQ(max_gen_entity.generation(), entity::MAX_GENERATIONS);
    EXPECT_TRUE(max_gen_entity != entity::invalid());

    // Test both at max
    auto max_both = entity::create(entity::MAX_ENTITIES, entity::MAX_GENERATIONS);
    EXPECT_EQ(max_both.id(), entity::MAX_ENTITIES);
    EXPECT_EQ(max_both.generation(), entity::MAX_GENERATIONS);
    EXPECT_TRUE(max_both != entity::invalid());
}

TEST_F(EntityTests, InvalidEntity)
{
    auto invalid = entity::invalid();

    EXPECT_TRUE(invalid == entity::invalid());
    EXPECT_EQ(invalid, entity::invalid()); // Two invalid entities are equal
}

//==============================================================================
//                        Bit Packing and Layout
//==============================================================================

TEST_F(EntityTests, BitPacking)
{
    // Verify ID-major bit layout: ID in high bits, generation in low bits
    // Bit layout: [31..........12][11.........0]
    //             [   20-bit ID  ][12-bit gen ]

    auto e = entity::create(1, 0);
    EXPECT_EQ(e.id(), 1);
    EXPECT_EQ(e.generation(), 0);

    // Create entity with generation set
    auto e2 = entity::create(0, 1);
    EXPECT_EQ(e2.id(), 0);
    EXPECT_EQ(e2.generation(), 1);

    // Verify different IDs with same generation don't collide
    auto e3 = entity::create(100, 5);
    auto e4 = entity::create(200, 5);
    EXPECT_NE(e3, e4);
    EXPECT_EQ(e3.generation(), e4.generation());
}

//==============================================================================
//                        Comparison Operators
//==============================================================================

TEST_F(EntityTests, Comparison)
{
    // Test equality
    auto e1 = entity::create(42, 5);
    auto e2 = entity::create(42, 5);
    auto e3 = entity::create(42, 6);
    auto e4 = entity::create(43, 5);

    EXPECT_EQ(e1, e2);
    EXPECT_NE(e1, e3);
    EXPECT_NE(e1, e4);

    // Test ordering: ID-major (ordered by ID first, then generation)
    auto low_id = entity::create(10, 100);
    auto high_id = entity::create(20, 0);
    EXPECT_LT(low_id, high_id); // Lower ID comes first regardless of generation

    auto same_id_low_gen = entity::create(50, 1);
    auto same_id_high_gen = entity::create(50, 2);
    EXPECT_LT(same_id_low_gen, same_id_high_gen); // Same ID, lower gen comes first
}

//==============================================================================
//                        STL Container Compatibility
//==============================================================================

TEST_F(EntityTests, StdContainers)
{
    // Test std::set (requires operator<=>)
    std::set<entity> entity_set;
    auto e1 = entity::create(1, 0);
    auto e2 = entity::create(2, 0);
    auto e3 = entity::create(1, 0); // Duplicate of e1

    entity_set.insert(e1);
    entity_set.insert(e2);
    entity_set.insert(e3);

    EXPECT_EQ(entity_set.size(), 2); // e3 is duplicate of e1

    // Test std::map (requires operator<=>)
    std::map<entity, int> entity_map;
    entity_map[e1] = 100;
    entity_map[e2] = 200;

    EXPECT_EQ(entity_map[e1], 100);
    EXPECT_EQ(entity_map[e2], 200);

    // Test std::unordered_set (requires std::hash specialization)
    std::unordered_set<entity, entity_hash> entity_uset;
    entity_uset.insert(e1);
    entity_uset.insert(e2);
    entity_uset.insert(e3);

    EXPECT_EQ(entity_uset.size(), 2); // e3 is duplicate of e1
    EXPECT_TRUE(entity_uset.contains(e1));
    EXPECT_TRUE(entity_uset.contains(e2));
}
