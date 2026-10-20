#include <gtest/gtest.h>
#include <gamecoe/utils/paths.hpp>
#include <filesystem>
#include <string>

using namespace gamecoe;

//==============================================================================
//                 PathsTests - Executable path resolution tests
//==============================================================================

class PathsTests : public ::testing::Test { };

//==============================================================================
//                           Successful Path Resolution
//==============================================================================

TEST_F(PathsTests, SuccessPaths)
{
    // Note: Failure-path tests (OS-call failures, unsupported platforms) are not
    // deterministically triggerable from unit tests without mock seams; these are
    // syscall-level or compile-time platform arms, not testable in a portable manner.

    // Test 1: executable_directory() returns an existing directory
    {
        auto result = executable_directory();
        ASSERT_TRUE(result.has_value());
        std::string exec_dir = result.value();
        EXPECT_TRUE(std::filesystem::exists(exec_dir));
        EXPECT_TRUE(std::filesystem::is_directory(exec_dir));
    }

    // Test 2: resolve_path() returns the executable directory joined with the relative path
    {
        std::string relative_path = "some_relative_file";
        auto result = resolve_path(relative_path);
        ASSERT_TRUE(result.has_value());

        auto exec_dir_result = executable_directory();
        ASSERT_TRUE(exec_dir_result.has_value());

        std::string expected = (std::filesystem::path(exec_dir_result.value()) / relative_path).string();
        EXPECT_EQ(result.value(), expected);
    }
}
