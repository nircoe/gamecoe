#pragma once

#include <cstdint>
#include <string>
#include <expected>
#include <glm/glm.hpp>
#include <gamecoe/utils/error.hpp>

namespace gamecoe
{
    class color
    {
        std::uint8_t m_red;
        std::uint8_t m_green;
        std::uint8_t m_blue;
        std::uint8_t m_alpha;

    public:
        constexpr color() : m_red(0U), m_green(0U), m_blue(0U), m_alpha(255U) { };
        constexpr color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255U) :
                        m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) { };

        color(const color &color_) = default;
        color &operator=(const color &color_) = default;
        ~color() = default;

        [[nodiscard]] bool operator==(const color &other) const noexcept;
        [[nodiscard]] bool operator!=(const color &other) const noexcept;

        [[nodiscard]] std::uint8_t red() const noexcept;
        [[nodiscard]] std::uint8_t green() const noexcept;
        [[nodiscard]] std::uint8_t blue() const noexcept;
        [[nodiscard]] std::uint8_t alpha() const noexcept;

        [[nodiscard]] glm::vec4 normalized() const noexcept;

        [[nodiscard]] std::uint32_t rgba() const noexcept;

        // Clamps values to [0.0f, 1.0f]
        [[nodiscard]] static color from_normalized(float red, float green, float blue, float alpha = 1.0f);
        // color::from_hex("#RRGGBBAA") or color::from_hex("#RRGGBB")
        [[nodiscard]] static std::expected<color, error> from_hex(const std::string &hex);
        // Lerps between color a and color b by t
        [[nodiscard]] static color lerp(const color &a, const color &b, float t);
    };

} // namespace gamecoe

namespace colorcoe
{
    using namespace gamecoe;

    // Grayscale
    constexpr color black()                 { return color(); }                 // gamecoe::color(0, 0, 0)
    constexpr color white()                 { return color(255, 255, 255); }    // gamecoe::color(255, 255, 255)
    constexpr color gray()                  { return color(128, 128, 128); }    // gamecoe::color(128, 128, 128)
    constexpr color dim_gray()               { return color(105, 105, 105); }    // gamecoe::color(105, 105, 105)
    constexpr color dark_gray()              { return color(169, 169, 169); }    // gamecoe::color(169, 169, 169)
    constexpr color silver()                { return color(192, 192, 192); }    // gamecoe::color(192, 192, 192)
    constexpr color light_gray()             { return color(211, 211, 211); }    // gamecoe::color(211, 211, 211)
    constexpr color gainsboro()             { return color(220, 220, 220); }    // gamecoe::color(220, 220, 220)
    constexpr color white_smoke()            { return color(245, 245, 245); }    // gamecoe::color(245, 245, 245)
    constexpr color slate_gray()             { return color(112, 128, 144); }    // gamecoe::color(112, 128, 144)
    constexpr color light_slate_gray()        { return color(119, 136, 153); }    // gamecoe::color(119, 136, 153)
    constexpr color dark_slate_gray()         { return color(47, 79, 79); }       // gamecoe::color(47, 79, 79)

    // Primary colors
    constexpr color red()                   { return color(255, 0, 0); }        // gamecoe::color(255, 0, 0)
    constexpr color green()                 { return color(0, 128, 0); }        // gamecoe::color(0, 128, 0)
    constexpr color lime()                  { return color(0, 255, 0); }        // gamecoe::color(0, 255, 0)
    constexpr color blue()                  { return color(0, 0, 255); }        // gamecoe::color(0, 0, 255)

    // Secondary colors
    constexpr color yellow()                { return color(255, 255, 0); }      // gamecoe::color(255, 255, 0)
    constexpr color cyan()                  { return color(0, 255, 255); }      // gamecoe::color(0, 255, 255)
    constexpr color aqua()                  { return color(0, 255, 255); }      // gamecoe::color(0, 255, 255)
    constexpr color magenta()               { return color(255, 0, 255); }      // gamecoe::color(255, 0, 255)
    constexpr color fuchsia()               { return color(255, 0, 255); }      // gamecoe::color(255, 0, 255)

    // Tertiary colors
    constexpr color orange()                { return color(255, 165, 0); }      // gamecoe::color(255, 165, 0)
    constexpr color purple()                { return color(128, 0, 128); }      // gamecoe::color(128, 0, 128)
    constexpr color brown()                 { return color(165, 42, 42); }      // gamecoe::color(165, 42, 42)
    constexpr color maroon()                { return color(128, 0, 0); }        // gamecoe::color(128, 0, 0)
    constexpr color olive()                 { return color(128, 128, 0); }      // gamecoe::color(128, 128, 0)
    constexpr color teal()                  { return color(0, 128, 128); }      // gamecoe::color(0, 128, 128)
    constexpr color navy()                  { return color(0, 0, 128); }        // gamecoe::color(0, 0, 128)

    // Red/Pink family
    constexpr color dark_red()               { return color(139, 0, 0); }        // gamecoe::color(139, 0, 0)
    constexpr color firebrick()             { return color(178, 34, 34); }      // gamecoe::color(178, 34, 34)
    constexpr color crimson()               { return color(220, 20, 60); }      // gamecoe::color(220, 20, 60)
    constexpr color indian_red()             { return color(205, 92, 92); }      // gamecoe::color(205, 92, 92)
    constexpr color light_coral()            { return color(240, 128, 128); }    // gamecoe::color(240, 128, 128)
    constexpr color salmon()                { return color(250, 128, 114); }    // gamecoe::color(250, 128, 114)
    constexpr color dark_salmon()            { return color(233, 150, 122); }    // gamecoe::color(233, 150, 122)
    constexpr color light_salmon()           { return color(255, 160, 122); }    // gamecoe::color(255, 160, 122)
    constexpr color tomato()                { return color(255, 99, 71); }      // gamecoe::color(255, 99, 71)
    constexpr color coral()                 { return color(255, 127, 80); }     // gamecoe::color(255, 127, 80)
    constexpr color deep_pink()              { return color(255, 20, 147); }     // gamecoe::color(255, 20, 147)
    constexpr color hot_pink()               { return color(255, 105, 180); }    // gamecoe::color(255, 105, 180)
    constexpr color pink()                  { return color(255, 192, 203); }    // gamecoe::color(255, 192, 203)
    constexpr color light_pink()             { return color(255, 182, 193); }    // gamecoe::color(255, 182, 193)
    constexpr color pale_violet_red()         { return color(219, 112, 147); }    // gamecoe::color(219, 112, 147)
    constexpr color medium_violet_red()       { return color(199, 21, 133); }     // gamecoe::color(199, 21, 133)

    // Orange family
    constexpr color orange_red()             { return color(255, 69, 0); }       // gamecoe::color(255, 69, 0)
    constexpr color dark_orange()            { return color(255, 140, 0); }      // gamecoe::color(255, 140, 0)

    // Yellow family
    constexpr color gold()                  { return color(255, 215, 0); }      // gamecoe::color(255, 215, 0)
    constexpr color light_yellow()           { return color(255, 255, 224); }    // gamecoe::color(255, 255, 224)
    constexpr color lemon_chiffon()          { return color(255, 250, 205); }    // gamecoe::color(255, 250, 205)
    constexpr color light_golden_rod_yellow()  { return color(250, 250, 210); }    // gamecoe::color(250, 250, 210)
    constexpr color papaya_whip()            { return color(255, 239, 213); }    // gamecoe::color(255, 239, 213)
    constexpr color moccasin()              { return color(255, 228, 181); }    // gamecoe::color(255, 228, 181)
    constexpr color peach_puff()             { return color(255, 218, 185); }    // gamecoe::color(255, 218, 185)
    constexpr color pale_golden_rod()         { return color(238, 232, 170); }    // gamecoe::color(238, 232, 170)
    constexpr color khaki()                 { return color(240, 230, 140); }    // gamecoe::color(240, 230, 140)
    constexpr color dark_khaki()             { return color(189, 183, 107); }    // gamecoe::color(189, 183, 107)
    constexpr color golden_rod()             { return color(218, 165, 32); }     // gamecoe::color(218, 165, 32)
    constexpr color dark_golden_rod()         { return color(184, 134, 11); }     // gamecoe::color(184, 134, 11)

    // Green family
    constexpr color dark_green()             { return color(0, 100, 0); }        // gamecoe::color(0, 100, 0)
    constexpr color dark_olive_green()        { return color(85, 107, 47); }      // gamecoe::color(85, 107, 47)
    constexpr color forest_green()           { return color(34, 139, 34); }      // gamecoe::color(34, 139, 34)
    constexpr color sea_green()              { return color(46, 139, 87); }      // gamecoe::color(46, 139, 87)
    constexpr color medium_sea_green()        { return color(60, 179, 113); }     // gamecoe::color(60, 179, 113)
    constexpr color dark_sea_green()          { return color(143, 188, 143); }    // gamecoe::color(143, 188, 143)
    constexpr color olive_drab()             { return color(107, 142, 35); }     // gamecoe::color(107, 142, 35)
    constexpr color yellow_green()           { return color(154, 205, 50); }     // gamecoe::color(154, 205, 50)
    constexpr color lime_green()             { return color(50, 205, 50); }      // gamecoe::color(50, 205, 50)
    constexpr color lawn_green()             { return color(124, 252, 0); }      // gamecoe::color(124, 252, 0)
    constexpr color chartreuse()            { return color(127, 255, 0); }      // gamecoe::color(127, 255, 0)
    constexpr color green_yellow()           { return color(173, 255, 47); }     // gamecoe::color(173, 255, 47)
    constexpr color spring_green()           { return color(0, 255, 127); }      // gamecoe::color(0, 255, 127)
    constexpr color medium_spring_green()     { return color(0, 250, 154); }      // gamecoe::color(0, 250, 154)
    constexpr color light_green()            { return color(144, 238, 144); }    // gamecoe::color(144, 238, 144)
    constexpr color pale_green()             { return color(152, 251, 152); }    // gamecoe::color(152, 251, 152)

    // Cyan/Turquoise family
    constexpr color light_sea_green()         { return color(32, 178, 170); }     // gamecoe::color(32, 178, 170)
    constexpr color medium_aqua_marine()      { return color(102, 205, 170); }    // gamecoe::color(102, 205, 170)
    constexpr color aqua_marine()            { return color(127, 255, 212); }    // gamecoe::color(127, 255, 212)
    constexpr color dark_cyan()              { return color(0, 139, 139); }      // gamecoe::color(0, 139, 139)
    constexpr color dark_turquoise()         { return color(0, 206, 209); }      // gamecoe::color(0, 206, 209)
    constexpr color medium_turquoise()       { return color(72, 209, 204); }     // gamecoe::color(72, 209, 204)
    constexpr color turquoise()             { return color(64, 224, 208); }     // gamecoe::color(64, 224, 208)
    constexpr color light_cyan()             { return color(224, 255, 255); }    // gamecoe::color(224, 255, 255)
    constexpr color pale_turquoise()         { return color(175, 238, 238); }    // gamecoe::color(175, 238, 238)

    // Blue family
    constexpr color midnight_blue()          { return color(25, 25, 112); }      // gamecoe::color(25, 25, 112)
    constexpr color dark_blue()              { return color(0, 0, 139); }        // gamecoe::color(0, 0, 139)
    constexpr color medium_blue()            { return color(0, 0, 205); }        // gamecoe::color(0, 0, 205)
    constexpr color royal_blue()             { return color(65, 105, 225); }     // gamecoe::color(65, 105, 225)
    constexpr color steel_blue()             { return color(70, 130, 180); }     // gamecoe::color(70, 130, 180)
    constexpr color dodger_blue()            { return color(30, 144, 255); }     // gamecoe::color(30, 144, 255)
    constexpr color deep_sky_blue()           { return color(0, 191, 255); }      // gamecoe::color(0, 191, 255)
    constexpr color corn_flower_blue()        { return color(100, 149, 237); }    // gamecoe::color(100, 149, 237)
    constexpr color sky_blue()               { return color(135, 206, 235); }    // gamecoe::color(135, 206, 235)
    constexpr color light_sky_blue()          { return color(135, 206, 250); }    // gamecoe::color(135, 206, 250)
    constexpr color light_steel_blue()        { return color(176, 196, 222); }    // gamecoe::color(176, 196, 222)
    constexpr color light_blue()             { return color(173, 216, 230); }    // gamecoe::color(173, 216, 230)
    constexpr color powder_blue()            { return color(176, 224, 230); }    // gamecoe::color(176, 224, 230)
    constexpr color cadet_blue()             { return color(95, 158, 160); }     // gamecoe::color(95, 158, 160)

    // Purple/Violet family
    constexpr color indigo()                { return color(75, 0, 130); }       // gamecoe::color(75, 0, 130)
    constexpr color dark_magenta()           { return color(139, 0, 139); }      // gamecoe::color(139, 0, 139)
    constexpr color dark_violet()            { return color(148, 0, 211); }      // gamecoe::color(148, 0, 211)
    constexpr color dark_slate_blue()         { return color(72, 61, 139); }      // gamecoe::color(72, 61, 139)
    constexpr color blue_violet()            { return color(138, 43, 226); }     // gamecoe::color(138, 43, 226)
    constexpr color dark_orchid()            { return color(153, 50, 204); }     // gamecoe::color(153, 50, 204)
    constexpr color slate_blue()             { return color(106, 90, 205); }     // gamecoe::color(106, 90, 205)
    constexpr color medium_slate_blue()       { return color(123, 104, 238); }    // gamecoe::color(123, 104, 238)
    constexpr color medium_purple()          { return color(147, 112, 219); }    // gamecoe::color(147, 112, 219)
    constexpr color medium_orchid()          { return color(186, 85, 211); }     // gamecoe::color(186, 85, 211)
    constexpr color violet()                { return color(238, 130, 238); }    // gamecoe::color(238, 130, 238)
    constexpr color plum()                  { return color(221, 160, 221); }    // gamecoe::color(221, 160, 221)
    constexpr color thistle()               { return color(216, 191, 216); }    // gamecoe::color(216, 191, 216)
    constexpr color orchid()                { return color(218, 112, 214); }    // gamecoe::color(218, 112, 214)
    constexpr color lavender()              { return color(230, 230, 250); }    // gamecoe::color(230, 230, 250)

    // Brown/Tan/Beige family
    constexpr color saddle_brown()           { return color(139, 69, 19); }      // gamecoe::color(139, 69, 19)
    constexpr color sienna()                { return color(160, 82, 45); }      // gamecoe::color(160, 82, 45)
    constexpr color chocolate()             { return color(210, 105, 30); }     // gamecoe::color(210, 105, 30)
    constexpr color peru()                  { return color(205, 133, 63); }     // gamecoe::color(205, 133, 63)
    constexpr color sandy_brown()            { return color(244, 164, 96); }     // gamecoe::color(244, 164, 96)
    constexpr color burly_wood()             { return color(222, 184, 135); }    // gamecoe::color(222, 184, 135)
    constexpr color tan()                   { return color(210, 180, 140); }    // gamecoe::color(210, 180, 140)
    constexpr color rosy_brown()             { return color(188, 143, 143); }    // gamecoe::color(188, 143, 143)
    constexpr color wheat()                 { return color(245, 222, 179); }    // gamecoe::color(245, 222, 179)
    constexpr color navajo_white()           { return color(255, 222, 173); }    // gamecoe::color(255, 222, 173)
    constexpr color bisque()                { return color(255, 228, 196); }    // gamecoe::color(255, 228, 196)
    constexpr color blanched_almond()        { return color(255, 235, 205); }    // gamecoe::color(255, 235, 205)
    constexpr color corn_silk()              { return color(255, 248, 220); }    // gamecoe::color(255, 248, 220)
    constexpr color antique_white()          { return color(250, 235, 215); }    // gamecoe::color(250, 235, 215)
    constexpr color beige()                 { return color(245, 245, 220); }    // gamecoe::color(245, 245, 220)

    // White tints
    constexpr color snow()                  { return color(255, 250, 250); }    // gamecoe::color(255, 250, 250)
    constexpr color misty_rose()             { return color(255, 228, 225); }    // gamecoe::color(255, 228, 225)
    constexpr color sea_shell()              { return color(255, 245, 238); }    // gamecoe::color(255, 245, 238)
    constexpr color lavender_blush()         { return color(255, 240, 245); }    // gamecoe::color(255, 240, 245)
    constexpr color linen()                 { return color(250, 240, 230); }    // gamecoe::color(250, 240, 230)
    constexpr color old_lace()               { return color(253, 245, 230); }    // gamecoe::color(253, 245, 230)
    constexpr color floral_white()           { return color(255, 250, 240); }    // gamecoe::color(255, 250, 240)
    constexpr color ivory()                 { return color(255, 255, 240); }    // gamecoe::color(255, 255, 240)
    constexpr color honeydew()              { return color(240, 255, 240); }    // gamecoe::color(240, 255, 240)
    constexpr color mint_cream()             { return color(245, 255, 250); }    // gamecoe::color(245, 255, 250)
    constexpr color azure()                 { return color(240, 255, 255); }    // gamecoe::color(240, 255, 255)
    constexpr color alice_blue()             { return color(240, 248, 255); }    // gamecoe::color(240, 248, 255)
    constexpr color ghost_white()            { return color(248, 248, 255); }    // gamecoe::color(248, 248, 255)

} // namespace colorcoe
