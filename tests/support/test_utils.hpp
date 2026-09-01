#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>
#include <gtest/gtest.h>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <vector>
#include <algorithm>
#define SKIP_IF_NOT(condition, message) \
    do { if (!(condition)) \
    { \
        GTEST_SKIP() << message; \
    } } while (0)

namespace test_utils
{
    // Fatally asserts mgr has exactly one entity, then writes its handle to out (for tests that flush one spawn and need its real handle).
    // Callers MUST wrap the call in ASSERT_NO_FATAL_FAILURE(...) - a fatal assert inside this helper only unwinds this function, not the caller's TEST_F.
    inline void sole_entity(gamecoe::entities &mgr, gamecoe::entity &out)
    {
        ASSERT_EQ(mgr.size(), 1u) << "test_utils::sole_entity(): expected exactly one entity in mgr";
        out = gamecoe::entity::invalid();
        mgr.for_each<gamecoe::components::transform>([&out](gamecoe::entity ent, [[maybe_unused]] const gamecoe::components::transform &tr)
        {
            out = ent;
        });
    }

    template <typename T>
    bool has(const std::vector<T> &values, const T &value)
    {
        return std::find(values.begin(), values.end(), value) != values.end();
    }

    inline void expect_vec3_near(const glm::vec3 &actual, const glm::vec3 &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }

    inline void expect_quat_near(const glm::quat &actual, const glm::quat &expected, float epsilon = 1e-5f)
    {
        EXPECT_NEAR(actual.w, expected.w, epsilon);
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }

    inline void expect_mat4_near(const glm::mat4 &actual, const glm::mat4 &expected, float epsilon = 1e-5f)
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                EXPECT_NEAR(actual[col][row], expected[col][row], epsilon);
    }

    inline bool quat_near(const glm::quat &a, const glm::quat &b, float epsilon = 1e-4f)
    {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }
} // namespace test_utils
