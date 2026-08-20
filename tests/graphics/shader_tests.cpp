#include <gtest/gtest.h>
#include "../gl_context.hpp"
#include <gamecoe/graphics/shader.hpp>
#include <gamecoe_config.hpp>
#include <glad/gl.h>
#include <filesystem>
#include <fstream>

using namespace gamecoe;
using namespace gamecoe::graphics;

namespace
{
    std::filesystem::path write_temp_shader(const std::string &name, const std::string &source)
    {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream file(path);
        file << source;
        return path;
    }

    const std::string valid_vertex_source =
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    const std::string valid_fragment_source =
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    const std::string uniform_fragment_source =
        "uniform bool u_bool;\n"
        "uniform int u_int;\n"
        "uniform float u_float;\n"
        "uniform vec3 u_vec3;\n"
        "uniform mat4 u_mat4;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    vec3 color = u_vec3 * u_float;\n"
        "    if (u_bool)\n"
        "        color += vec3(u_int);\n"
        "    color += vec3(u_mat4[0][0]);\n"
        "    FragColor = vec4(color, 1.0);\n"
        "}\n";
}

//==============================================================================
//              ShaderTests - graphics::shader class tests
//==============================================================================

class ShaderTests : public GLContextTest {};

//==============================================================================
//                        Creation
//==============================================================================

TEST_F(ShaderTests, CreateSuccess)
{
    SKIP_IF_NO_GL();

    auto vertex_path = write_temp_shader("shader_tests_valid.vert", valid_vertex_source);
    auto fragment_path = write_temp_shader("shader_tests_valid.frag", valid_fragment_source);

    auto result = shader::create(vertex_path.string(), fragment_path.string());
    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->id(), 0u);

    std::filesystem::remove(vertex_path);
    std::filesystem::remove(fragment_path);
}

TEST_F(ShaderTests, CreateMissingFileFails)
{
    SKIP_IF_NO_GL();

    auto result = shader::create("/nonexistent/path/shader_tests_missing.vert",
                                  "/nonexistent/path/shader_tests_missing.frag");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error_code::file_read_failure);
}

TEST_F(ShaderTests, CreateWithVersionStatementFails)
{
    SKIP_IF_NO_GL();

    std::string vertex_with_version = "#version 460 core\n" + valid_vertex_source;

    auto vertex_path = write_temp_shader("shader_tests_version.vert", vertex_with_version);
    auto fragment_path = write_temp_shader("shader_tests_version.frag", valid_fragment_source);

    auto result = shader::create(vertex_path.string(), fragment_path.string());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error_code::invalid_argument);

    std::filesystem::remove(vertex_path);
    std::filesystem::remove(fragment_path);
}

TEST_F(ShaderTests, CreateWithSyntaxErrorFails)
{
    SKIP_IF_NO_GL();

    const std::string broken_fragment_source =
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0, 0.0, 0.0, 1.0)\n" // missing semicolon
        "}\n";

    auto vertex_path = write_temp_shader("shader_tests_broken.vert", valid_vertex_source);
    auto fragment_path = write_temp_shader("shader_tests_broken.frag", broken_fragment_source);

    auto result = shader::create(vertex_path.string(), fragment_path.string());
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, error_code::shader_compilation_failure);

    std::filesystem::remove(vertex_path);
    std::filesystem::remove(fragment_path);
}

//==============================================================================
//                        Uniform Setters
//==============================================================================

TEST_F(ShaderTests, SetUniformsDoesNotCrash)
{
    SKIP_IF_NO_GL();

    auto vertex_path = write_temp_shader("shader_tests_uniforms.vert", valid_vertex_source);
    auto fragment_path = write_temp_shader("shader_tests_uniforms.frag", uniform_fragment_source);

    auto result = shader::create(vertex_path.string(), fragment_path.string());
    ASSERT_TRUE(result.has_value());

    // Test 1: representative spread of set() overloads on declared uniforms
    EXPECT_NO_FATAL_FAILURE({
        result->set("u_bool", true);
        result->set("u_int", 1);
        result->set("u_float", 1.0f);
        result->set("u_vec3", glm::vec3(1.0f, 0.0f, 0.0f));
        result->set("u_mat4", glm::mat4(1.0f));
    });

    // Test 2: undefined uniform - first call takes the cache-miss path, second hits the cache
    EXPECT_NO_FATAL_FAILURE({
        result->set("u_does_not_exist", 1.0f);
        result->set("u_does_not_exist", 1.0f);
    });

    std::filesystem::remove(vertex_path);
    std::filesystem::remove(fragment_path);
}

//==============================================================================
//                        Move Construction
//==============================================================================

TEST_F(ShaderTests, MoveConstruction)
{
    SKIP_IF_NO_GL();

    auto vertex_path = write_temp_shader("shader_tests_move_ctor.vert", valid_vertex_source);
    auto fragment_path = write_temp_shader("shader_tests_move_ctor.frag", valid_fragment_source);

    auto result = shader::create(vertex_path.string(), fragment_path.string());
    ASSERT_TRUE(result.has_value());

    std::uint32_t original_id = result->id();

    shader moved(std::move(*result));

    EXPECT_EQ(moved.id(), original_id);

    std::filesystem::remove(vertex_path);
    std::filesystem::remove(fragment_path);
}

//==============================================================================
//                        Move Assignment (double-destroy regression guard)
//==============================================================================

TEST_F(ShaderTests, MoveAssignmentNoDoubleDestroy)
{
    SKIP_IF_NO_GL();

    auto vertex_path1 = write_temp_shader("shader_tests_move_assign1.vert", valid_vertex_source);
    auto fragment_path1 = write_temp_shader("shader_tests_move_assign1.frag", uniform_fragment_source);

    auto vertex_path2 = write_temp_shader("shader_tests_move_assign2.vert", valid_vertex_source);
    auto fragment_path2 = write_temp_shader("shader_tests_move_assign2.frag", uniform_fragment_source);

    auto result1 = shader::create(vertex_path1.string(), fragment_path1.string());
    ASSERT_TRUE(result1.has_value());

    auto result2 = shader::create(vertex_path2.string(), fragment_path2.string());
    ASSERT_TRUE(result2.has_value());

    shader s1(std::move(*result1));
    shader s2(std::move(*result2));

    std::uint32_t id2 = s2.id();

    // s1 and s2 are separate GL programs that each independently declare "u_float" - GL is
    // free to assign it a different location number in each, so populating s1's own cache
    // here (with s1's own program's location) before the move is what makes the post-move
    // check below a real regression guard rather than a coincidence.
    s1.set("u_float", 111.0f);

    s1 = std::move(s2);

    // Proves state actually transferred, not just that nothing crashed. The still-implicit
    // part: a clean return here lets both destructors run at scope exit without a double
    // glDeleteProgram() on the same GL program id.
    EXPECT_EQ(s1.id(), id2);

    // Proves the uniform-location cache moved along with m_id, not just m_id alone: if
    // operator=(shader&&) left s1's stale cached location for "u_float" (from its own,
    // just-deleted program) instead of adopting s2's, this set() would write to whatever
    // location number happened to be cached - which may not even be "u_float" on the new
    // program - and the value read back below would not be 222.0f.
    s1.set("u_float", 222.0f);

    std::int32_t location = glGetUniformLocation(s1.id(), "u_float");
    ASSERT_GE(location, 0);

    float value = 0.0f;
    glGetUniformfv(s1.id(), location, &value);
    EXPECT_FLOAT_EQ(value, 222.0f);

    std::filesystem::remove(vertex_path1);
    std::filesystem::remove(fragment_path1);
    std::filesystem::remove(vertex_path2);
    std::filesystem::remove(fragment_path2);
}
