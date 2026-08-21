#include <gtest/gtest.h>
#include "../gl_context.hpp"
#include <gamecoe/graphics/texture.hpp>
#include <filesystem>
#include <fstream>
#include <glad/gl.h>

using namespace gamecoe;
using namespace gamecoe::graphics;

namespace
{
    std::filesystem::path write_temp_ppm(const std::string &name, int width, int height)
    {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream file(path, std::ios::binary);
        file << "P6\n" << width << " " << height << "\n255\n";
        for (int i = 0; i < width * height; ++i)
        {
            char pixel[3] = { static_cast<char>(255), static_cast<char>(0), static_cast<char>(0) };
            file.write(pixel, 3);
        }
        return path;
    }
}

//==============================================================================
//              TextureTests - graphics::texture class tests
//==============================================================================

class TextureTests : public GLContextTest {};

//==============================================================================
//                        Creation
//==============================================================================

TEST_F(TextureTests, CreateSuccess)
{
    SKIP_IF_NO_GL();

    auto path = write_temp_ppm("texture_tests_valid.ppm", 2, 2);

    auto result = texture::create_2d(path.string());
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->id(), 0u);
    EXPECT_EQ(result->dimension(), static_cast<std::int32_t>(GL_TEXTURE_2D));

    std::filesystem::remove(path);
}

TEST_F(TextureTests, CreateMissingFileFails)
{
    SKIP_IF_NO_GL();

    auto result = texture::create_2d("/nonexistent/path/texture_tests_missing.ppm");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error_code::image_load_failure);
}

TEST_F(TextureTests, CreateMipmapOption)
{
    SKIP_IF_NO_GL();

    auto path = write_temp_ppm("texture_tests_mipmap.ppm", 4, 4);

    // Test 1: generate_mipmap = true succeeds
    auto with_mipmap = texture::create_2d(path.string(), true, true);
    EXPECT_TRUE(with_mipmap.has_value());

    // Test 2: generate_mipmap = false succeeds
    auto without_mipmap = texture::create_2d(path.string(), true, false);
    EXPECT_TRUE(without_mipmap.has_value());

    std::filesystem::remove(path);
}

//==============================================================================
//                        Parameters
//==============================================================================

TEST_F(TextureTests, SetParametersSuccess)
{
    SKIP_IF_NO_GL();

    auto path = write_temp_ppm("texture_tests_parameters.ppm", 2, 2);

    auto result = texture::create_2d(path.string());
    ASSERT_TRUE(result.has_value());

    auto params = result->set_parameters(texture_wrap::repeat, texture_wrap::clamp_to_edge,
                                          texture_filter::linear, texture_filter::linear);
    EXPECT_TRUE(params.has_value());

    std::filesystem::remove(path);
}

//==============================================================================
//                        Move Construction
//==============================================================================

TEST_F(TextureTests, MoveConstruction)
{
    SKIP_IF_NO_GL();

    auto path = write_temp_ppm("texture_tests_move_ctor.ppm", 2, 2);

    auto result = texture::create_2d(path.string());
    ASSERT_TRUE(result.has_value());

    std::uint32_t original_id = result->id();

    texture moved(std::move(*result));

    EXPECT_EQ(moved.id(), original_id);
    EXPECT_EQ(moved.dimension(), static_cast<std::int32_t>(GL_TEXTURE_2D));

    std::filesystem::remove(path);
}

//==============================================================================
//                        Move Assignment (double-destroy regression guard)
//==============================================================================

TEST_F(TextureTests, MoveAssignmentNoDoubleDestroy)
{
    SKIP_IF_NO_GL();

    auto path1 = write_temp_ppm("texture_tests_move_assign1.ppm", 2, 2);
    auto path2 = write_temp_ppm("texture_tests_move_assign2.ppm", 4, 4);

    auto result1 = texture::create_2d(path1.string());
    ASSERT_TRUE(result1.has_value());

    auto result2 = texture::create_2d(path2.string());
    ASSERT_TRUE(result2.has_value());

    texture t1(std::move(*result1));
    texture t2(std::move(*result2));

    std::uint32_t id2 = t2.id();

    t1 = std::move(t2);

    // Proves state actually transferred, not just that nothing crashed. The still-implicit
    // part: a clean return here lets both destructors run at scope exit without a double
    // glDeleteTextures() on the same GL texture id.
    EXPECT_EQ(t1.id(), id2);

    std::filesystem::remove(path1);
    std::filesystem::remove(path2);
}
