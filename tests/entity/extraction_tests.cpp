#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
#include <vector>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <support/test_utils.hpp>
#include <support/scene_id.hpp>

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

namespace
{
    template <typename View>
    std::vector<entity> entities_of(View &&view)
    {
        std::vector<entity> out;
        for (auto item : view)
            out.push_back(std::get<0>(item));
        return out;
    }

    std::vector<entity> sorted(std::vector<entity> list)
    {
        std::sort(list.begin(), list.end());
        return list;
    }
} // namespace

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
        EXPECT_TRUE(test_utils::has(found_entities, e1));
        EXPECT_TRUE(test_utils::has(found_entities, e3));
        EXPECT_TRUE(test_utils::has(found_entities, e4));
        EXPECT_TRUE(test_utils::has(found_entities, e5));
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
        EXPECT_TRUE(test_utils::has(found_entities, e2));
        EXPECT_TRUE(test_utils::has(found_entities, e3));
        EXPECT_TRUE(test_utils::has(found_entities, e4));
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
        EXPECT_TRUE(test_utils::has(found_entities, e3));
        EXPECT_TRUE(test_utils::has(found_entities, e4));
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
//                        Active Partition Bound
//==============================================================================

TEST_F(ExtractionTests, ActivePartitionBound)
{
    // Test 1: pool selection is driven by active_size(), not size() - construct two pools
    // where the two metrics disagree on which pool is "smallest" and confirm extract<>()
    // still returns the correct (small) result set.
    //
    // Transform pool: 100 total entities, only 1 stays active (99 deactivated).
    // Health pool: 5 total entities, all 5 active.
    // Old selection (by size()): Health (5) < Transform (100) -> Health chosen as primary.
    // New selection (by active_size()): Transform (1) < Health (5) -> Transform chosen as primary.
    // This makes old vs. new pool selection disagree; the matching entity below is the only
    // one with both components, so a correct result here means the smallest ACTIVE partition
    // was iterated correctly regardless of which pool ended up driving the loop.
    {
        entity match = mgr.create();
        mgr.add_component<Transform>(match, Transform{1.0f, 2.0f, 3.0f});
        mgr.add_component<Health>(match, Health{50});

        for (int i = 0; i < 99; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Transform>(e, Transform{static_cast<float>(i), 0.0f, 0.0f});
            mgr.deactivate(e);
        }

        for (int i = 0; i < 4; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Health>(e, Health{i});
        }

        auto extracted = mgr.extract<Transform, Health>();

        std::vector<entity> found_entities;
        for (auto [e, t, h] : extracted)
        {
            found_entities.push_back(e);
            EXPECT_EQ(t.x, 1.0f);
            EXPECT_EQ(t.y, 2.0f);
            EXPECT_EQ(t.z, 3.0f);
            EXPECT_EQ(h.value, 50);
        }

        EXPECT_EQ(found_entities.size(), 1);
        EXPECT_EQ(found_entities[0], match);
    }

    // Test 2: a pool with active_size() == 0 (but size() > 0) makes the extraction empty,
    // regardless of which pool is picked as primary. This is the case that actually breaks
    // if active_size() weren't wired into the selection: the smallest-active-partition bound
    // must be 0, so the driving loop must never execute.
    {
        mgr.clear();

        for (int i = 0; i < 50; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Transform>(e, Transform{static_cast<float>(i), 0.0f, 0.0f});
            mgr.deactivate(e);
        }

        for (int i = 0; i < 3; ++i)
        {
            entity e = mgr.create();
            mgr.add_component<Health>(e, Health{i});
        }

        auto extracted = mgr.extract<Transform, Health>();
        EXPECT_EQ(extracted.begin(), extracted.end());

        int count = 0;
        for ([[maybe_unused]] auto [e, t, h] : extracted)
        {
            ++count;
        }
        EXPECT_EQ(count, 0);
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
//                        Frozen Scenes
//==============================================================================

TEST_F(ExtractionTests, FrozenScenes)
{
    constexpr scene_id s1 = scene_id::TestScene1;
    constexpr scene_id s2 = scene_id::TestScene2;
    constexpr scene_id s3 = scene_id::TestScene3;

    // Test 1: extract_with_frozen() returns the same extraction type as extract()
    {
        const entities &const_mgr = mgr;

        static_assert(std::is_same_v<
            decltype(mgr.extract_with_frozen<Transform>()),
            decltype(mgr.extract<Transform>())
        >);
        static_assert(std::is_same_v<
            decltype(const_mgr.extract_with_frozen<Transform>()),
            extraction<const Transform>
        >);
        static_assert(std::is_same_v<
            decltype(const_mgr.extract_with_frozen<Transform>()),
            decltype(const_mgr.extract<Transform>())
        >);
    }

    // Test 2: the filter applies whichever pool drives the iteration
    {
        mgr.clear();

        entity f1 = mgr.create({}, s1);
        entity f2 = mgr.create({}, s1);
        entity v1 = mgr.create({}, s2);
        for (entity e : { f1, f2, v1 })
        {
            mgr.add_component<Transform>(e, Transform{1.0f, 0.0f, 0.0f});
            mgr.add_component<Health>(e, Health{10});
        }
        mgr.set_scene_frozen(s1, true);

        // Transform is the smaller pool
        for (int i = 0; i < 5; ++i)
            mgr.add_component<Health>(mgr.create(), Health{i});

        EXPECT_EQ((entities_of(mgr.extract<Transform, Health>())), std::vector<entity>{ v1 });
        EXPECT_EQ((sorted(entities_of(mgr.extract_with_frozen<Transform, Health>()))), sorted({ f1, f2, v1 }));

        // Velocity is the smallest pool and drives the iteration
        mgr.clear();
        f1 = mgr.create({}, s1);
        f2 = mgr.create({}, s1);
        v1 = mgr.create({}, s2);
        for (entity e : { f1, f2, v1 })
        {
            mgr.add_component<Transform>(e, Transform{1.0f, 0.0f, 0.0f});
            mgr.add_component<Health>(e, Health{10});
        }
        mgr.set_scene_frozen(s1, true);
        for (int i = 0; i < 5; ++i)
            mgr.add_component<Transform>(mgr.create(), Transform{0.0f, 0.0f, 0.0f});
        for (entity e : { f1, f2, v1 })
            mgr.add_component<Velocity>(e, Velocity{0.0f, 0.0f, 0.0f});

        EXPECT_EQ((entities_of(mgr.extract<Velocity, Transform, Health>())), std::vector<entity>{ v1 });
        EXPECT_EQ((sorted(entities_of(mgr.extract_with_frozen<Velocity, Transform, Health>()))),
                  sorted({ f1, f2, v1 }));
    }

    // Test 3: frozen entities at the start, middle and end of the dense array are skipped
    {
        mgr.clear();

        entity f1 = mgr.create({}, s1);
        entity v1 = mgr.create({}, s2);
        entity f2 = mgr.create({}, s1);
        entity f3 = mgr.create({}, s1);
        entity v2 = mgr.create({}, s2);
        entity f4 = mgr.create({}, s1);
        for (entity e : { f1, v1, f2, f3, v2, f4 })
            mgr.add_component<Transform>(e, Transform{0.0f, 0.0f, 0.0f});

        mgr.set_scene_frozen(s1, true);
        EXPECT_EQ(sorted(entities_of(mgr.extract<Transform>())), sorted({ v1, v2 }));

        mgr.set_scene_frozen(s2, true);
        auto view = mgr.extract<Transform>();
        EXPECT_EQ(view.begin(), view.end());
        EXPECT_TRUE(entities_of(view).empty());
        EXPECT_EQ(entities_of(mgr.extract_with_frozen<Transform>()).size(), 6u);
    }

    // Test 4: the iterator lands on the next visible entity
    {
        mgr.clear();

        entity f1 = mgr.create({}, s1);
        entity v1 = mgr.create({}, s2);
        entity f2 = mgr.create({}, s1);
        entity f3 = mgr.create({}, s1);
        entity v2 = mgr.create({}, s2);
        entity f4 = mgr.create({}, s1);
        float x = 0.0f;
        for (entity e : { f1, v1, f2, f3, v2, f4 })
            mgr.add_component<Transform>(e, Transform{x++, 0.0f, 0.0f});

        mgr.set_scene_frozen(s1, true);
        auto view = mgr.extract<Transform>();

        auto it = view.begin();
        {
            auto [e, t] = *it;
            EXPECT_EQ(e, v1);
            EXPECT_EQ(t.x, 1.0f);
        }

        auto &ref = ++it;
        EXPECT_EQ(&ref, &it);
        {
            auto [e, t] = *it;
            EXPECT_EQ(e, v2);
            EXPECT_EQ(t.x, 4.0f);
        }
        ++it;
        EXPECT_EQ(it, view.end());

        it = view.begin();
        auto copy = it++;
        EXPECT_EQ(std::get<0>(*copy), v1);
        EXPECT_EQ(std::get<0>(*it), v2);

        int count = 0;
        for ([[maybe_unused]] auto [e, t] : view)
            ++count;
        EXPECT_EQ(count, 2);
    }

    // Test 5: with no scene_tag pool there is nothing to filter
    {
        mgr.clear();

        entity a = mgr.create();
        entity b = mgr.create();
        for (entity e : { a, b })
            mgr.add_component<Transform>(e, Transform{0.0f, 0.0f, 0.0f});
        mgr.set_scene_frozen(s1, true);

        EXPECT_EQ(sorted(entities_of(mgr.extract<Transform>())), sorted({ a, b }));
        EXPECT_EQ(sorted(entities_of(mgr.extract_with_frozen<Transform>())), sorted({ a, b }));
    }

    // Test 6: scenes frozen during the loop apply to the entities that come after
    {
        mgr.clear();

        entity g0 = mgr.create();
        entity a1 = mgr.create({}, s1);
        entity a2 = mgr.create({}, s1);
        entity b1 = mgr.create({}, s2);
        entity b2 = mgr.create({}, s2);
        entity c1 = mgr.create({}, s3);
        entity c2 = mgr.create({}, s3);
        entity g1 = mgr.create();
        for (entity e : { g0, a1, a2, b1, b2, c1, c2, g1 })
            mgr.add_component<Transform>(e, Transform{0.0f, 0.0f, 0.0f});

        // The extraction holds the frozen vector itself, not its buffer, so the list can grow
        // while the loop runs.
        auto view = mgr.extract<Transform>();

        std::vector<entity> delivered;
        for ([[maybe_unused]] auto [e, t] : view)
        {
            delivered.push_back(e);
            if (e == g0) mgr.set_scene_frozen(s1, true);
            else if (e == b1) mgr.set_scene_frozen(s2, true);
            else if (e == c1) mgr.set_scene_frozen(s3, true);
        }
        EXPECT_EQ(delivered, (std::vector<entity>{ g0, b1, c1, g1 }));

        EXPECT_EQ(sorted(entities_of(view)), sorted({ g0, g1 }));

        // Unfreezing from the loop works the same way
        mgr.clear();
        g0 = mgr.create();
        a1 = mgr.create({}, s1);
        a2 = mgr.create({}, s1);
        for (entity e : { g0, a1, a2 })
            mgr.add_component<Transform>(e, Transform{0.0f, 0.0f, 0.0f});
        mgr.set_scene_frozen(s1, true);

        delivered.clear();
        for ([[maybe_unused]] auto [e, t] : mgr.extract<Transform>())
        {
            delivered.push_back(e);
            if (e == g0) mgr.set_scene_frozen(s1, false);
        }
        EXPECT_EQ(delivered, (std::vector<entity>{ g0, a1, a2 }));
    }

    // Test 7: an extraction kept across freeze changes follows them
    {
        mgr.clear();

        entity g = mgr.create();
        entity a = mgr.create({}, s1);
        for (entity e : { g, a })
            mgr.add_component<Transform>(e, Transform{0.0f, 0.0f, 0.0f});

        auto view = mgr.extract<Transform>();
        EXPECT_EQ(sorted(entities_of(view)), sorted({ g, a }));

        mgr.set_scene_frozen(s1, true);
        EXPECT_EQ(entities_of(view), std::vector<entity>{ g });

        mgr.set_scene_frozen(s1, false);
        EXPECT_EQ(sorted(entities_of(view)), sorted({ g, a }));
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
        EXPECT_TRUE(test_utils::has(x_values, 1.0f));
        EXPECT_TRUE(test_utils::has(x_values, 2.0f));
        EXPECT_TRUE(test_utils::has(x_values, 3.0f));
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
