#include <gtest/gtest.h>
#include <gamecoe/component/transform.hpp>
#include <support/test_utils.hpp>

using namespace gamecoe;
using namespace test_utils;

//==============================================================================
//                 TransformTests - transform component tests
//==============================================================================

class TransformTests : public ::testing::Test
{
protected:
    components::transform t;
};

//==============================================================================
//                        Default Construction
//==============================================================================

TEST_F(TransformTests, DefaultConstruction)
{
    // Test 1: Default position, rotation, scale
    {
        expect_vec3_near(t.position, glm::vec3(0.0f, 0.0f, 0.0f));
        expect_quat_near(t.rotation, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
        expect_vec3_near(t.scale, glm::vec3(1.0f, 1.0f, 1.0f));
    }
}

//==============================================================================
//                        Matrix Composition
//==============================================================================

TEST_F(TransformTests, MatrixComposition)
{
    float rotation_angle = glm::radians(90.0f);
    glm::vec3 rotation_axis(0.0f, 1.0f, 0.0f);

    // Known, non-trivial position/rotation/scale
    t.position = glm::vec3(1.0f, 2.0f, 3.0f);
    t.rotation = glm::angleAxis(rotation_angle, rotation_axis);
    t.scale = glm::vec3(2.0f, 1.0f, 0.5f);

    // Test 1: translation_matrix() - identity with translation column set
    {
        glm::mat4 expected = glm::translate(glm::mat4(1.0f), t.position);
        expect_mat4_near(t.translation_matrix(), expected);
    }

    // Test 2: rotation_matrix() - matches independently-derived axis-angle rotation matrix
    {
        glm::mat4 expected = glm::rotate(glm::mat4(1.0f), rotation_angle, rotation_axis);
        expect_mat4_near(t.rotation_matrix(), expected);
    }

    // Test 3: scale_matrix() - identity with diagonal set to scale components
    {
        glm::mat4 expected = glm::scale(glm::mat4(1.0f), t.scale);
        expect_mat4_near(t.scale_matrix(), expected);
    }

    // Test 4: local_matrix() equals the naive translation * rotation * scale composition
    {
        glm::mat4 expected = t.translation_matrix() * t.rotation_matrix() * t.scale_matrix();
        expect_mat4_near(t.local_matrix(), expected);
    }
}

//==============================================================================
//                        Direction Vectors
//==============================================================================

TEST_F(TransformTests, DirectionVectors)
{
    // Test 1: Default (identity) rotation
    {
        expect_vec3_near(t.forward(), glm::vec3(0.0f, 0.0f, -1.0f));
        expect_vec3_near(t.right(), glm::vec3(1.0f, 0.0f, 0.0f));
        expect_vec3_near(t.up(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // Test 2: Known 90-degree rotation around Y
    {
        t.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Analytically-derived expected value (not just re-deriving forward()'s own
        // formula, which would make this check tautological)
        expect_vec3_near(t.forward(), glm::vec3(-1.0f, 0.0f, 0.0f)); // right-handed: -Z rotated +90 about Y -> -X
    }
}

//==============================================================================
//                        Euler Rotation
//==============================================================================

TEST_F(TransformTests, EulerRotation)
{
    // Test 1: Pure single-axis (X) rotation round-trips through euler_rotation()
    {
        glm::vec3 euler(glm::radians(30.0f), 0.0f, 0.0f);
        t.rotation = glm::quat(euler);

        expect_vec3_near(t.euler_rotation(), euler);
    }
}

//==============================================================================
//                        Translate
//==============================================================================

TEST_F(TransformTests, Translate)
{
    // Test 1: Single translate() call offsets position
    {
        t.position = glm::vec3(1.0f, 2.0f, 3.0f);
        t.translate(glm::vec3(1.0f, 1.0f, 1.0f));

        expect_vec3_near(t.position, glm::vec3(2.0f, 3.0f, 4.0f));
    }

    // Test 2: Successive translate() calls accumulate, not overwrite
    {
        t.translate(glm::vec3(0.5f, -1.0f, 2.0f));

        expect_vec3_near(t.position, glm::vec3(2.5f, 2.0f, 6.0f));
    }
}

//==============================================================================
//              rotate() vs rotate_around() Composition Order
//==============================================================================

TEST_F(TransformTests, RotateCompositionOrder)
{
    glm::quat initial_rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Test 1: rotate_around() post-multiplies (intrinsic rotation)
    {
        t.rotation = initial_rotation;

        glm::vec3 axis(1.0f, 0.0f, 0.0f);
        float angle = glm::radians(30.0f);

        t.rotate_around(axis, angle);

        glm::quat delta = glm::angleAxis(angle, glm::normalize(axis));
        glm::quat post_multiply_expected = initial_rotation * delta;
        glm::quat pre_multiply_wrong = delta * initial_rotation;

        // The two candidate results must actually differ, otherwise this test
        // wouldn't catch a regression to the old (buggy) pre-multiply order.
        EXPECT_FALSE(quat_near(post_multiply_expected, pre_multiply_wrong));

        expect_quat_near(t.rotation, post_multiply_expected);
    }

    // Test 2: rotate() pre-multiplies (extrinsic rotation) - mirror check
    {
        t.rotation = initial_rotation;

        glm::vec3 euler_offset(glm::radians(20.0f), 0.0f, 0.0f);

        t.rotate(euler_offset);

        glm::quat delta = glm::quat(euler_offset);
        glm::quat pre_multiply_expected = delta * initial_rotation;
        glm::quat post_multiply_wrong = initial_rotation * delta;

        EXPECT_FALSE(quat_near(pre_multiply_expected, post_multiply_wrong));

        expect_quat_near(t.rotation, pre_multiply_expected);
    }
}
