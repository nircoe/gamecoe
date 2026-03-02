#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
#include <vector>
#include <algorithm>

using namespace gamecoe;

//==============================================================================
//                    Test Component Types
//==============================================================================

struct Transform
{
    float x, y, z;

    bool operator==(const Transform &other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Velocity
{
    float dx, dy, dz;

    bool operator==(const Velocity &other) const
    {
        return dx == other.dx && dy == other.dy && dz == other.dz;
    }
};

struct Health
{
    int value;

    bool operator==(const Health &other) const
    {
        return value == other.value;
    }
};

//==============================================================================
//                    ExtractionTests - Multi-component query tests
//==============================================================================

class ExtractionTests : public ::testing::Test
{
protected:
    entities mgr;
};

//==============================================================================
//                        Basic Extraction and Filtering
//==============================================================================

TEST_F(ExtractionTests, BasicExtractionAndFiltering)
{
    // Setup: Create entities with different component combinations
    entity e1 = mgr.create();
    mgr.add_component<Transform>(e1, Transform{1.0f, 0.0f, 0.0f});

    entity e2 = mgr.create();
    mgr.add_component<Velocity>(e2, Velocity{0.1f, 0.0f, 0.0f});

    entity e3 = mgr.create();
    mgr.add_component<Transform>(e3, Transform{3.0f, 0.0f, 0.0f});
    mgr.add_component<Velocity>(e3, Velocity{0.3f, 0.0f, 0.0f});

    entity e4 = mgr.create();
    mgr.add_component<Transform>(e4, Transform{4.0f, 0.0f, 0.0f});
    mgr.add_component<Velocity>(e4, Velocity{0.4f, 0.0f, 0.0f});

    entity e5 = mgr.create();
    mgr.add_component<Transform>(e5, Transform{5.0f, 0.0f, 0.0f});

    // Test 1: Extract single component (Transform)
    {
        std::vector<entity> found_entities;
        auto extracted = mgr.extract<Transform>();

        for (auto [e, t] : extracted)
        {
            found_entities.push_back(e);
        }

        EXPECT_EQ(found_entities.size(), 4); // e1, e3, e4, e5
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e1) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e3) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e4) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e5) != found_entities.end());
    }

    // Test 2: Extract single component (Velocity)
    {
        std::vector<entity> found_entities;
        auto extracted = mgr.extract<Velocity>();

        for (auto [e, v] : extracted)
        {
            found_entities.push_back(e);
        }

        EXPECT_EQ(found_entities.size(), 3); // e2, e3, e4
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e2) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e3) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e4) != found_entities.end());
    }

    // Test 3: Extract multiple components (Transform + Velocity)
    {
        std::vector<entity> found_entities;
        auto extracted = mgr.extract<Transform, Velocity>();

        for (auto [e, t, v] : extracted)
        {
            found_entities.push_back(e);
            // Verify component values are correct
            EXPECT_TRUE(mgr.has_component<Transform>(e));
            EXPECT_TRUE(mgr.has_component<Velocity>(e));
        }

        EXPECT_EQ(found_entities.size(), 2); // Only e3 and e4
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e3) != found_entities.end());
        EXPECT_TRUE(std::find(found_entities.begin(), found_entities.end(), e4) != found_entities.end());
    }

    // Test 4: Verify component values can be read correctly
    {
        auto extracted = mgr.extract<Transform, Velocity>();
        for (auto [e, t, v] : extracted)
        {
            if (e == e3)
            {
                EXPECT_EQ(t.x, 3.0f);
                EXPECT_EQ(v.dx, 0.3f);
            }
            else if (e == e4)
            {
                EXPECT_EQ(t.x, 4.0f);
                EXPECT_EQ(v.dx, 0.4f);
            }
        }
    }

    // Test 5: Verify component values can be modified
    {
        auto extracted = mgr.extract<Transform>();
        for (auto [e, t] : extracted)
        {
            t.x += 100.0f;
        }

        // Verify modifications persisted
        EXPECT_EQ(mgr.get_component<Transform>(e1)->x, 101.0f);
        EXPECT_EQ(mgr.get_component<Transform>(e3)->x, 103.0f);
        EXPECT_EQ(mgr.get_component<Transform>(e4)->x, 104.0f);
        EXPECT_EQ(mgr.get_component<Transform>(e5)->x, 105.0f);
    }
}

//==============================================================================
//                        Empty and Edge Cases
//==============================================================================

TEST_F(ExtractionTests, EmptyAndEdgeCases)
{
    // Test 1: Empty entities manager
    {
        auto extracted = mgr.extract<Transform>();
        EXPECT_EQ(extracted.begin(), extracted.end());

        int count = 0;
        for ([[maybe_unused]] auto [e, t] : extracted)
        {
            ++count;
        }
        EXPECT_EQ(count, 0);
    }

    // Test 2: Entities exist but none have the requested component
    {
        entity e1 = mgr.create();
        entity e2 = mgr.create();
        mgr.add_component<Transform>(e1, Transform{1.0f, 0.0f, 0.0f});
        mgr.add_component<Transform>(e2, Transform{2.0f, 0.0f, 0.0f});

        auto extracted = mgr.extract<Velocity>();
        EXPECT_EQ(extracted.begin(), extracted.end());

        int count = 0;
        for ([[maybe_unused]] auto [e, v] : extracted)
        {
            ++count;
        }
        EXPECT_EQ(count, 0);
    }

    // Test 3: Request multiple components where no entity has all of them
    {
        mgr.clear();

        entity e1 = mgr.create();
        mgr.add_component<Transform>(e1, Transform{1.0f, 0.0f, 0.0f});

        entity e2 = mgr.create();
        mgr.add_component<Velocity>(e2, Velocity{0.1f, 0.0f, 0.0f});

        entity e3 = mgr.create();
        mgr.add_component<Health>(e3, Health{100});

        // No entity has all three components
        auto extracted = mgr.extract<Transform, Velocity, Health>();
        EXPECT_EQ(extracted.begin(), extracted.end());

        int count = 0;
        for ([[maybe_unused]] auto [e, t, v, h] : extracted)
        {
            ++count;
        }
        EXPECT_EQ(count, 0);
    }

    // Test 4: Component type that was never added to any entity
    {
        mgr.clear();
        entity e = mgr.create();
        mgr.add_component<Transform>(e, Transform{1.0f, 0.0f, 0.0f});

        auto extracted = mgr.extract<Health>();
        EXPECT_EQ(extracted.begin(), extracted.end());
    }
}

//==============================================================================
//                        Const Correctness
//==============================================================================

TEST_F(ExtractionTests, ConstCorrectness)
{
    entity e1 = mgr.create();
    mgr.add_component<Transform>(e1, Transform{1.0f, 2.0f, 3.0f});
    mgr.add_component<Velocity>(e1, Velocity{0.1f, 0.2f, 0.3f});

    entity e2 = mgr.create();
    mgr.add_component<Transform>(e2, Transform{4.0f, 5.0f, 6.0f});
    mgr.add_component<Velocity>(e2, Velocity{0.4f, 0.5f, 0.6f});

    // Test 1: Mixed const/mutable access
    {
        auto extracted = mgr.extract<Transform, const Velocity>();

        for (auto [e, t, v] : extracted)
        {
            // Can modify Transform
            t.x += 10.0f;

            // Can read Velocity (const reference)
            float speed = v.dx;
            EXPECT_TRUE(speed >= 0.0f);

            // Note: Uncommenting the line below should cause a compile error
            // v.dx = 1.0f;  // Error: v is const Velocity&
        }

        // Verify Transform was modified
        EXPECT_EQ(mgr.get_component<Transform>(e1)->x, 11.0f);
        EXPECT_EQ(mgr.get_component<Transform>(e2)->x, 14.0f);

        // Verify Velocity was not modified
        EXPECT_EQ(mgr.get_component<Velocity>(e1)->dx, 0.1f);
        EXPECT_EQ(mgr.get_component<Velocity>(e2)->dx, 0.4f);
    }

    // Test 2: Const entities manager forces const components
    {
        const entities &const_mgr = mgr;
        auto extracted = const_mgr.extract<Transform>();

        for (auto [e, t] : extracted)
        {
            // Can read Transform
            float x = t.x;
            EXPECT_TRUE(x >= 0.0f);

            // Note: Uncommenting the line below should cause a compile error
            // t.x = 99.0f;  // Error: t is const Transform&
        }

        // Verify type is const Transform&
        static_assert(std::is_same_v<
            decltype(const_mgr.extract<Transform>()),
            extraction<const Transform>
        >);
    }

    // Test 3: Const entities with multiple components
    {
        const entities &const_mgr = mgr;
        auto extracted = const_mgr.extract<Transform, Velocity>();

        int count = 0;
        for (auto [e, t, v] : extracted)
        {
            // Can only read
            EXPECT_TRUE(t.x >= 0.0f);
            EXPECT_TRUE(v.dx >= 0.0f);
            ++count;

            // Note: These should cause compile errors
            // t.x = 1.0f;  // Error: const Transform&
            // v.dx = 1.0f; // Error: const Velocity&
        }
        EXPECT_EQ(count, 2);
    }
}

//==============================================================================
//                        Iterator Protocol
//==============================================================================

TEST_F(ExtractionTests, IteratorProtocol)
{
    // Setup
    entity e1 = mgr.create();
    mgr.add_component<Transform>(e1, Transform{1.0f, 0.0f, 0.0f});

    entity e2 = mgr.create();
    mgr.add_component<Transform>(e2, Transform{2.0f, 0.0f, 0.0f});

    entity e3 = mgr.create();
    mgr.add_component<Transform>(e3, Transform{3.0f, 0.0f, 0.0f});

    auto extracted = mgr.extract<Transform>();

    // Test 1: begin() != end() when entities exist
    EXPECT_NE(extracted.begin(), extracted.end());

    // Test 2: Prefix operator++ advances and returns reference
    {
        auto it = extracted.begin();
        auto &ref = ++it;
        EXPECT_EQ(&ref, &it); // Returns reference to self
    }

    // Test 3: Postfix operator++ advances and returns copy
    {
        auto it = extracted.begin();
        auto copy = it++;
        EXPECT_NE(it, copy); // Copy is different from advanced iterator
    }

    // Test 4: operator* returns correct tuple
    {
        auto it = extracted.begin();
        auto [e, t] = *it;

        EXPECT_TRUE(mgr.valid(e));
        EXPECT_TRUE(mgr.has_component<Transform>(e));

        // Verify we can use the types correctly
        static_assert(std::is_same_v<decltype(e), entity>);
        static_assert(std::is_same_v<decltype(t), Transform&>);
    }

    // Test 5: Iterator equality/inequality
    {
        auto it1 = extracted.begin();
        auto it2 = extracted.begin();
        auto it_end = extracted.end();

        EXPECT_EQ(it1, it2);
        EXPECT_NE(it1, it_end);
    }

    // Test 6: Range-for loop syntax works
    {
        std::vector<float> x_values;
        for (auto [e, t] : extracted)
        {
            x_values.push_back(t.x);
        }

        EXPECT_EQ(x_values.size(), 3);
        EXPECT_TRUE(std::find(x_values.begin(), x_values.end(), 1.0f) != x_values.end());
        EXPECT_TRUE(std::find(x_values.begin(), x_values.end(), 2.0f) != x_values.end());
        EXPECT_TRUE(std::find(x_values.begin(), x_values.end(), 3.0f) != x_values.end());
    }

    // Test 7: Manual iteration with begin/end
    {
        int count = 0;
        for (auto it = extracted.begin(); it != extracted.end(); ++it)
        {
            auto [e, t] = *it;
            EXPECT_TRUE(mgr.valid(e));
            ++count;
        }
        EXPECT_EQ(count, 3);
    }

    // Test 8: Empty extraction has begin() == end()
    {
        mgr.clear();
        auto empty_extracted = mgr.extract<Transform>();
        EXPECT_EQ(empty_extracted.begin(), empty_extracted.end());
    }
}
