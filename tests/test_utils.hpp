#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>
#include <gtest/gtest.h>

namespace test_utils
{
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
