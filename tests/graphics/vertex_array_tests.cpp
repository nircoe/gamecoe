#include <gtest/gtest.h>
#include "../gl_context.hpp"
#include <gamecoe/graphics/vertex_array.hpp>

using namespace gamecoe;
using namespace gamecoe::graphics;

//==============================================================================
//              VertexArrayTests - graphics::vertex_array class tests
//==============================================================================

class VertexArrayTests : public GLContextTest
{
protected:
    static void TearDownTestSuite()
    {
        vertex_array::destroy_shape_vertex_arrays();
        GLContextTest::TearDownTestSuite();
    }
};

//==============================================================================
//                        Creation
//==============================================================================

TEST_F(VertexArrayTests, CreateWithoutIndices)
{
    SKIP_IF_NO_GL();

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    std::size_t vertex_count = 3;

    auto result = vertex_array::create(vertices, vertex_count, 3);
    ASSERT_TRUE(result.has_value());

    EXPECT_FALSE(result->has_indices());
    EXPECT_EQ(result->vertex_count(), vertex_count);
}

TEST_F(VertexArrayTests, CreateWithIndices)
{
    SKIP_IF_NO_GL();

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    std::uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
    std::size_t vertex_count = 4;
    std::size_t index_count = 6;

    auto result = vertex_array::create(vertices, vertex_count, 3, indices, index_count);
    ASSERT_TRUE(result.has_value());

    EXPECT_TRUE(result->has_indices());
    EXPECT_EQ(result->vertex_count(), vertex_count);
    EXPECT_EQ(result->index_count(), index_count);
}

//==============================================================================
//                        Move Construction
//==============================================================================

TEST_F(VertexArrayTests, MoveConstruction)
{
    SKIP_IF_NO_GL();

    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    std::size_t vertex_count = 3;

    auto result = vertex_array::create(vertices, vertex_count, 3);
    ASSERT_TRUE(result.has_value());

    vertex_array moved(std::move(*result));

    EXPECT_FALSE(moved.has_indices());
    EXPECT_EQ(moved.vertex_count(), vertex_count);
}

//==============================================================================
//                        Move Assignment (double-destroy regression guard)
//==============================================================================

TEST_F(VertexArrayTests, MoveAssignmentNoDoubleDestroy)
{
    SKIP_IF_NO_GL();

    float vertices1[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };
    std::size_t vertex_count1 = 3;

    float vertices2[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    std::uint32_t indices2[] = { 0, 1, 2, 2, 3, 0 };
    std::size_t vertex_count2 = 4;
    std::size_t index_count2 = 6;

    auto result1 = vertex_array::create(vertices1, vertex_count1, 3);
    ASSERT_TRUE(result1.has_value());

    auto result2 = vertex_array::create(vertices2, vertex_count2, 3, indices2, index_count2);
    ASSERT_TRUE(result2.has_value());

    vertex_array va1(std::move(*result1));
    vertex_array va2(std::move(*result2));

    va1 = std::move(va2);

    // Different vertex/index counts so this proves state actually transferred, not just that
    // nothing crashed. The still-implicit part: a clean return here lets both destructors run
    // at scope exit without a double glDeleteVertexArrays() on the same GL vertex array id.
    EXPECT_TRUE(va1.has_indices());
    EXPECT_EQ(va1.vertex_count(), vertex_count2);
    EXPECT_EQ(va1.index_count(), index_count2);
}

//==============================================================================
//                        Shape Cache
//==============================================================================

TEST_F(VertexArrayTests, ShapeCacheReturnsValidInstances)
{
    SKIP_IF_NO_GL();

    const vertex_array *triangle_va = vertex_array::triangle();
    const vertex_array *rectangle_va = vertex_array::rectangle();
    const vertex_array *box_va = vertex_array::box();

    EXPECT_NE(triangle_va, nullptr);
    EXPECT_NE(rectangle_va, nullptr);
    EXPECT_NE(box_va, nullptr);
}

TEST_F(VertexArrayTests, ShapeCacheReturnsSamePointer)
{
    SKIP_IF_NO_GL();

    const vertex_array *first = vertex_array::triangle();
    const vertex_array *second = vertex_array::triangle();

    EXPECT_EQ(first, second);
}

TEST_F(VertexArrayTests, DestroyShapeVertexArraysRebuildsFreshInstance)
{
    SKIP_IF_NO_GL();

    const vertex_array *before = vertex_array::triangle();
    ASSERT_NE(before, nullptr);

    // Regression guard: the shape cache pointers used to be function-local statics, left
    // dangling by destroy_shape_vertex_arrays()'s separate tracking vector. They're now
    // file-scope globals that get nulled on destroy, so triangle() rebuilds instead of
    // returning freed memory.
    vertex_array::destroy_shape_vertex_arrays();

    const vertex_array *after = vertex_array::triangle();
    ASSERT_NE(after, nullptr);
    EXPECT_GT(after->vertex_count(), 0u);
}
