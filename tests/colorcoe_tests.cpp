#include <gtest/gtest.h>
#include <colorcoe.hpp>
#include <gamecoe/utils/error.hpp>

using namespace gamecoe;

//==============================================================================
//                 ColorcoeTests - Color::fromHex() tests
//==============================================================================

class ColorcoeTests : public ::testing::Test { };

//==============================================================================
//                         Successful Decoding
//==============================================================================

TEST_F(ColorcoeTests, FromHexSuccess)
{
    // Test 1: "#RRGGBB" decodes red/green/blue with the default alpha
    {
        auto result = Color::fromHex("#6C5CE8");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->red(), 0x6C);
        EXPECT_EQ(result->green(), 0x5C);
        EXPECT_EQ(result->blue(), 0xE8);
        EXPECT_EQ(result->alpha(), 255);
        EXPECT_EQ(result.value(), Color(0x6C, 0x5C, 0xE8));
    }

    // Test 2: "#RRGGBBAA" decodes red/green/blue/alpha exactly
    {
        auto result = Color::fromHex("#1A2B3C4D");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->red(), 0x1A);
        EXPECT_EQ(result->green(), 0x2B);
        EXPECT_EQ(result->blue(), 0x3C);
        EXPECT_EQ(result->alpha(), 0x4D);
        EXPECT_EQ(result.value(), Color(0x1A, 0x2B, 0x3C, 0x4D));
    }
}

//==============================================================================
//                          Malformed Input Failures
//==============================================================================

TEST_F(ColorcoeTests, FromHexFailure)
{
    // Test 1: missing the leading '#'
    {
        auto result = Color::fromHex("6C5CE8");
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, error_code::invalid_argument);
    }

    // Test 2: wrong length
    {
        auto result = Color::fromHex("#6C5");
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, error_code::invalid_argument);
    }

    // Test 3: non-hex digit among otherwise valid-length input
    {
        auto result = Color::fromHex("#GG5CE8");
        ASSERT_FALSE(result.has_value());
        EXPECT_EQ(result.error().code, error_code::invalid_argument);
    }
}
