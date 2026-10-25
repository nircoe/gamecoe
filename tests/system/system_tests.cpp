#include <gamecoe/component/transform.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/system/system.hpp>
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

using namespace gamecoe;

namespace
{
    struct marker
    {
        int value;
    };

    void noop_system(game &) { }
} // namespace

//==============================================================================
//                SystemTests - systems::entry and systems::create tests
//==============================================================================

class SystemTests : public ::testing::Test
{
};

TEST_F(SystemTests, CreateSplitsReadsAndWrites)
{
    const std::uint32_t transform_id = entities::component_id<components::transform>();
    const std::uint32_t marker_id = entities::component_id<marker>();

    // Test 1: const goes to reads, non-const goes to writes
    {
        systems::entry entry = systems::create<const components::transform, marker>("a", noop_system);
        EXPECT_EQ(entry.reads, std::vector<std::uint32_t>{ transform_id });
        EXPECT_EQ(entry.writes, std::vector<std::uint32_t>{ marker_id });
    }

    // Test 2: all-const goes to reads only, all-non-const goes to writes only
    {
        systems::entry reads_only = systems::create<const components::transform, const marker>("a", noop_system);
        EXPECT_EQ(reads_only.reads, (std::vector<std::uint32_t>{ transform_id, marker_id }));
        EXPECT_TRUE(reads_only.writes.empty());

        systems::entry writes_only = systems::create<components::transform, marker>("a", noop_system);
        EXPECT_TRUE(writes_only.reads.empty());
        EXPECT_EQ(writes_only.writes, (std::vector<std::uint32_t>{ transform_id, marker_id }));
    }

    // Test 3: empty pack gives empty access sets and a callable function
    {
        systems::entry entry = systems::create("a", [](game &) { });
        EXPECT_TRUE(entry.reads.empty());
        EXPECT_TRUE(entry.writes.empty());
        EXPECT_TRUE(static_cast<bool>(entry.func));
    }

    // Test 4: declaration order is kept within each vector
    {
        systems::entry entry = systems::create<marker, components::transform>("a", noop_system);
        EXPECT_EQ(entry.writes, (std::vector<std::uint32_t>{ marker_id, transform_id }));
    }

    // Test 5: reference forms behave like the plain forms
    {
        systems::entry entry = systems::create<const components::transform &, marker &>("a", noop_system);
        EXPECT_EQ(entry.reads, std::vector<std::uint32_t>{ transform_id });
        EXPECT_EQ(entry.writes, std::vector<std::uint32_t>{ marker_id });
    }

    // Test 6: const and non-const forms share one id
    {
        EXPECT_EQ(systems::create<const marker>("a", noop_system).reads.front(), marker_id);
        EXPECT_EQ(systems::create<marker>("a", noop_system).writes.front(), marker_id);
        EXPECT_NE(marker_id, transform_id);
    }
}

TEST_F(SystemTests, CreateStoresName)
{
    // Test 1: the name is stored as given
    {
        EXPECT_EQ(systems::create("physics", noop_system).name, "physics");
    }

    // Test 2: the name is kept for a lambda and with an explicit component pack
    {
        EXPECT_EQ(systems::create("lambda", [](game &) { }).name, "lambda");
        EXPECT_EQ(systems::create<marker>("m", noop_system).name, "m");
    }
}
