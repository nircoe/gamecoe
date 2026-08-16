#include <gtest/gtest.h>
#include <gamecoe/utils/paths.hpp>
#include <filesystem>

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
        std::string execDir = result.value();
        EXPECT_TRUE(std::filesystem::exists(execDir));
        EXPECT_TRUE(std::filesystem::is_directory(execDir));
    }

    // Test 2: resolve_path() returns the executable directory joined with the relative path
    {
        std::string relativePath = "some_relative_file";
        auto result = resolve_path(relativePath);
        ASSERT_TRUE(result.has_value());

        auto execDirResult = executable_directory();
        ASSERT_TRUE(execDirResult.has_value());

        std::string expected = (std::filesystem::path(execDirResult.value()) / relativePath).string();
        EXPECT_EQ(result.value(), expected);
    }
}
