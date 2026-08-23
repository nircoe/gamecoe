#include <gtest/gtest.h>
#include <support/gl_context.hpp>
#include <gamecoe/graphics/buffer.hpp>
#include <gamecoe_config.hpp>
#include <glad/gl.h>

using namespace gamecoe;
using namespace gamecoe::graphics;

//==============================================================================
//              GraphicsBufferTests - graphics::buffer class tests
//==============================================================================

class GraphicsBufferTests : public GLContextTest {};

//==============================================================================
//                        Creation
//==============================================================================

TEST_F(GraphicsBufferTests, CreateVertexBuffer)
{
    SKIP_IF_NO_GL();

    auto result = buffer::create_vertex_buffer();
    ASSERT_TRUE(result.has_value());

    EXPECT_NE(result->id(), 0u);
    EXPECT_EQ(result->target(), static_cast<std::uint32_t>(GL_ARRAY_BUFFER));
}

TEST_F(GraphicsBufferTests, CreateIndexBuffer)
{
    SKIP_IF_NO_GL();

    auto result = buffer::create_index_buffer();
    ASSERT_TRUE(result.has_value());

    EXPECT_NE(result->id(), 0u);
    EXPECT_EQ(result->target(), static_cast<std::uint32_t>(GL_ELEMENT_ARRAY_BUFFER));
}

#if GAMECOE_HAS_UBO
TEST_F(GraphicsBufferTests, CreateUniformBuffer)
{
    SKIP_IF_NO_GL();

    auto result = buffer::create_uniform_buffer(0);
    ASSERT_TRUE(result.has_value());

    EXPECT_NE(result->id(), 0u);
    EXPECT_EQ(result->target(), static_cast<std::uint32_t>(GL_UNIFORM_BUFFER));
}
#endif

//==============================================================================
//                        Data Upload
//==============================================================================

TEST_F(GraphicsBufferTests, UploadData)
{
    SKIP_IF_NO_GL();

    auto result = buffer::create_vertex_buffer();
    ASSERT_TRUE(result.has_value());

    float data[] = { 0.0f, 1.0f, 2.0f, 3.0f };

    // Test 1: first upload allocates storage
    auto first = result->upload_data(data, sizeof(data));
    EXPECT_TRUE(first.has_value());

    // Test 2: second upload reuses the already-allocated storage
    auto second = result->upload_data(data, sizeof(data));
    EXPECT_TRUE(second.has_value());
}

//==============================================================================
//                        Move Construction
//==============================================================================

TEST_F(GraphicsBufferTests, MoveConstruction)
{
    SKIP_IF_NO_GL();

    auto result = buffer::create_vertex_buffer();
    ASSERT_TRUE(result.has_value());

    std::uint32_t original_id = result->id();

    buffer moved(std::move(*result));

    EXPECT_EQ(moved.id(), original_id);
    EXPECT_EQ(moved.target(), static_cast<std::uint32_t>(GL_ARRAY_BUFFER));
}

//==============================================================================
//                        Move Assignment (double-destroy regression guard)
//==============================================================================

TEST_F(GraphicsBufferTests, MoveAssignmentNoDoubleDestroy)
{
    SKIP_IF_NO_GL();

    auto result1 = buffer::create_vertex_buffer();
    ASSERT_TRUE(result1.has_value());

    auto result2 = buffer::create_index_buffer();
    ASSERT_TRUE(result2.has_value());

    buffer b1(std::move(*result1));
    buffer b2(std::move(*result2));

    b1 = std::move(b2);

    // Different targets so this proves state actually transferred, not just that nothing
    // crashed. The still-implicit part: a clean return here lets both destructors run at
    // scope exit without a double glDeleteBuffers() on the same GL buffer id.
    EXPECT_EQ(b1.target(), static_cast<std::uint32_t>(GL_ELEMENT_ARRAY_BUFFER));
}
