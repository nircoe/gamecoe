#include <gtest/gtest.h>
#include <gamecoe/system/system.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <cstdint>
#include <vector>

using namespace gamecoe;

namespace
{
    struct marker
    {
        int value;
    };

    void noop_system(game&) {}
} // namespace

//==============================================================================
//                SystemTests - system_entry and make_system tests
//==============================================================================

TEST(SystemTests, MakeSystemSplitsReadsAndWrites)
{
    const std::uint32_t transform_id = entities::component_id<components::transform>();
    const std::uint32_t marker_id = entities::component_id<marker>();

    // Test 1: const goes to reads, non-const goes to writes
    {
        system_entry entry = make_system<const components::transform, marker>(noop_system);
        EXPECT_EQ(entry.reads, std::vector<std::uint32_t>{transform_id});
        EXPECT_EQ(entry.writes, std::vector<std::uint32_t>{marker_id});
    }

    // Test 2: all-const goes to reads only, all-non-const goes to writes only
    {
        system_entry reads_only = make_system<const components::transform, const marker>(noop_system);
        EXPECT_EQ(reads_only.reads, (std::vector<std::uint32_t>{transform_id, marker_id}));
        EXPECT_TRUE(reads_only.writes.empty());

        system_entry writes_only = make_system<components::transform, marker>(noop_system);
        EXPECT_TRUE(writes_only.reads.empty());
        EXPECT_EQ(writes_only.writes, (std::vector<std::uint32_t>{transform_id, marker_id}));
    }

    // Test 3: empty pack gives empty access sets and a callable function
    {
        system_entry entry = make_system([](game&) {});
        EXPECT_TRUE(entry.reads.empty());
        EXPECT_TRUE(entry.writes.empty());
        EXPECT_TRUE(static_cast<bool>(entry.func));
    }

    // Test 4: declaration order is kept within each vector
    {
        system_entry entry = make_system<marker, components::transform>(noop_system);
        EXPECT_EQ(entry.writes, (std::vector<std::uint32_t>{marker_id, transform_id}));
    }

    // Test 5: reference forms behave like the plain forms
    {
        system_entry entry = make_system<const components::transform&, marker&>(noop_system);
        EXPECT_EQ(entry.reads, std::vector<std::uint32_t>{transform_id});
        EXPECT_EQ(entry.writes, std::vector<std::uint32_t>{marker_id});
    }
}

TEST(SystemTests, ConstAndNonConstShareOneId)
{
    const std::uint32_t marker_id = entities::component_id<marker>();

    EXPECT_EQ(make_system<const marker>(noop_system).reads.front(), marker_id);
    EXPECT_EQ(make_system<marker>(noop_system).writes.front(), marker_id);
    EXPECT_NE(marker_id, entities::component_id<components::transform>());
}
