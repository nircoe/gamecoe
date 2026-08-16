#pragma once

#include <cstdint>
#include <string>
#include <expected>
#include <glm/glm.hpp>
#include <gamecoe/utils/error.hpp>

namespace gamecoe
{
    class Color
    {
        std::uint8_t m_red;
        std::uint8_t m_green;
        std::uint8_t m_blue;
        std::uint8_t m_alpha;

    public:
        constexpr Color() : m_red(0U), m_green(0U), m_blue(0U), m_alpha(255U) { };
        constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255U) : 
                        m_red(red), m_green(green), m_blue(blue), m_alpha(alpha) { };
        
        Color(const Color &color) = default;
        Color &operator=(const Color &color) = default;
        ~Color() = default;

        bool operator==(const Color &other) const;
        bool operator!=(const Color &other) const;

        std::uint8_t red() const;
        std::uint8_t green() const;
        std::uint8_t blue() const;
        std::uint8_t alpha() const;
        
        glm::vec4 normalized() const;

        std::uint32_t rgba() const;

        // Clamps values to [0.0f, 1.0f]
        static Color fromNormalized(float red, float green, float blue, float alpha = 1.0f);
        // Color::fromHex("#RRGGBBAA") or Color::fromHex("#RRGGBB")
        [[nodiscard]] static std::expected<Color, error> fromHex(const std::string &hex);
        // Lerps between Color a and Color b by t
        static Color lerp(const Color &a, const Color &b, float t);
    };

} // namespace gamecoe

namespace colorcoe
{
    using namespace gamecoe;

    // Grayscale
    constexpr Color black()                 { return Color(); }                 // gamecoe::Color(0, 0, 0)
    constexpr Color white()                 { return Color(255, 255, 255); }    // gamecoe::Color(255, 255, 255)
    constexpr Color gray()                  { return Color(128, 128, 128); }    // gamecoe::Color(128, 128, 128)
    constexpr Color dimGray()               { return Color(105, 105, 105); }    // gamecoe::Color(105, 105, 105)
    constexpr Color darkGray()              { return Color(169, 169, 169); }    // gamecoe::Color(169, 169, 169)
    constexpr Color silver()                { return Color(192, 192, 192); }    // gamecoe::Color(192, 192, 192)
    constexpr Color lightGray()             { return Color(211, 211, 211); }    // gamecoe::Color(211, 211, 211)
    constexpr Color gainsboro()             { return Color(220, 220, 220); }    // gamecoe::Color(220, 220, 220)
    constexpr Color whiteSmoke()            { return Color(245, 245, 245); }    // gamecoe::Color(245, 245, 245)
    constexpr Color slateGray()             { return Color(112, 128, 144); }    // gamecoe::Color(112, 128, 144)
    constexpr Color lightSlateGray()        { return Color(119, 136, 153); }    // gamecoe::Color(119, 136, 153)
    constexpr Color darkSlateGray()         { return Color(47, 79, 79); }       // gamecoe::Color(47, 79, 79)

    // Primary colors
    constexpr Color red()                   { return Color(255, 0, 0); }        // gamecoe::Color(255, 0, 0)
    constexpr Color green()                 { return Color(0, 128, 0); }        // gamecoe::Color(0, 128, 0)
    constexpr Color lime()                  { return Color(0, 255, 0); }        // gamecoe::Color(0, 255, 0)
    constexpr Color blue()                  { return Color(0, 0, 255); }        // gamecoe::Color(0, 0, 255)

    // Secondary colors
    constexpr Color yellow()                { return Color(255, 255, 0); }      // gamecoe::Color(255, 255, 0)
    constexpr Color cyan()                  { return Color(0, 255, 255); }      // gamecoe::Color(0, 255, 255)
    constexpr Color aqua()                  { return Color(0, 255, 255); }      // gamecoe::Color(0, 255, 255)
    constexpr Color magenta()               { return Color(255, 0, 255); }      // gamecoe::Color(255, 0, 255)
    constexpr Color fuchsia()               { return Color(255, 0, 255); }      // gamecoe::Color(255, 0, 255)

    // Tertiary colors
    constexpr Color orange()                { return Color(255, 165, 0); }      // gamecoe::Color(255, 165, 0)
    constexpr Color purple()                { return Color(128, 0, 128); }      // gamecoe::Color(128, 0, 128)
    constexpr Color brown()                 { return Color(165, 42, 42); }      // gamecoe::Color(165, 42, 42)
    constexpr Color maroon()                { return Color(128, 0, 0); }        // gamecoe::Color(128, 0, 0)
    constexpr Color olive()                 { return Color(128, 128, 0); }      // gamecoe::Color(128, 128, 0)
    constexpr Color teal()                  { return Color(0, 128, 128); }      // gamecoe::Color(0, 128, 128)
    constexpr Color navy()                  { return Color(0, 0, 128); }        // gamecoe::Color(0, 0, 128)

    // Red/Pink family
    constexpr Color darkRed()               { return Color(139, 0, 0); }        // gamecoe::Color(139, 0, 0)
    constexpr Color firebrick()             { return Color(178, 34, 34); }      // gamecoe::Color(178, 34, 34)
    constexpr Color crimson()               { return Color(220, 20, 60); }      // gamecoe::Color(220, 20, 60)
    constexpr Color indianRed()             { return Color(205, 92, 92); }      // gamecoe::Color(205, 92, 92)
    constexpr Color lightCoral()            { return Color(240, 128, 128); }    // gamecoe::Color(240, 128, 128)
    constexpr Color salmon()                { return Color(250, 128, 114); }    // gamecoe::Color(250, 128, 114)
    constexpr Color darkSalmon()            { return Color(233, 150, 122); }    // gamecoe::Color(233, 150, 122)
    constexpr Color lightSalmon()           { return Color(255, 160, 122); }    // gamecoe::Color(255, 160, 122)
    constexpr Color tomato()                { return Color(255, 99, 71); }      // gamecoe::Color(255, 99, 71)
    constexpr Color coral()                 { return Color(255, 127, 80); }     // gamecoe::Color(255, 127, 80)
    constexpr Color deepPink()              { return Color(255, 20, 147); }     // gamecoe::Color(255, 20, 147)
    constexpr Color hotPink()               { return Color(255, 105, 180); }    // gamecoe::Color(255, 105, 180)
    constexpr Color pink()                  { return Color(255, 192, 203); }    // gamecoe::Color(255, 192, 203)
    constexpr Color lightPink()             { return Color(255, 182, 193); }    // gamecoe::Color(255, 182, 193)
    constexpr Color paleVioletRed()         { return Color(219, 112, 147); }    // gamecoe::Color(219, 112, 147)
    constexpr Color mediumVioletRed()       { return Color(199, 21, 133); }     // gamecoe::Color(199, 21, 133)

    // Orange family
    constexpr Color orangeRed()             { return Color(255, 69, 0); }       // gamecoe::Color(255, 69, 0)
    constexpr Color darkOrange()            { return Color(255, 140, 0); }      // gamecoe::Color(255, 140, 0)

    // Yellow family
    constexpr Color gold()                  { return Color(255, 215, 0); }      // gamecoe::Color(255, 215, 0)
    constexpr Color lightYellow()           { return Color(255, 255, 224); }    // gamecoe::Color(255, 255, 224)
    constexpr Color lemonChiffon()          { return Color(255, 250, 205); }    // gamecoe::Color(255, 250, 205)
    constexpr Color lightGoldenRodYellow()  { return Color(250, 250, 210); }    // gamecoe::Color(250, 250, 210)
    constexpr Color papayaWhip()            { return Color(255, 239, 213); }    // gamecoe::Color(255, 239, 213)
    constexpr Color moccasin()              { return Color(255, 228, 181); }    // gamecoe::Color(255, 228, 181)
    constexpr Color peachPuff()             { return Color(255, 218, 185); }    // gamecoe::Color(255, 218, 185)
    constexpr Color paleGoldenRod()         { return Color(238, 232, 170); }    // gamecoe::Color(238, 232, 170)
    constexpr Color khaki()                 { return Color(240, 230, 140); }    // gamecoe::Color(240, 230, 140)
    constexpr Color darkKhaki()             { return Color(189, 183, 107); }    // gamecoe::Color(189, 183, 107)
    constexpr Color goldenRod()             { return Color(218, 165, 32); }     // gamecoe::Color(218, 165, 32)
    constexpr Color darkGoldenRod()         { return Color(184, 134, 11); }     // gamecoe::Color(184, 134, 11)

    // Green family
    constexpr Color darkGreen()             { return Color(0, 100, 0); }        // gamecoe::Color(0, 100, 0)
    constexpr Color darkOliveGreen()        { return Color(85, 107, 47); }      // gamecoe::Color(85, 107, 47)
    constexpr Color forestGreen()           { return Color(34, 139, 34); }      // gamecoe::Color(34, 139, 34)
    constexpr Color seaGreen()              { return Color(46, 139, 87); }      // gamecoe::Color(46, 139, 87)
    constexpr Color mediumSeaGreen()        { return Color(60, 179, 113); }     // gamecoe::Color(60, 179, 113)
    constexpr Color darkSeaGreen()          { return Color(143, 188, 143); }    // gamecoe::Color(143, 188, 143)
    constexpr Color oliveDrab()             { return Color(107, 142, 35); }     // gamecoe::Color(107, 142, 35)
    constexpr Color yellowGreen()           { return Color(154, 205, 50); }     // gamecoe::Color(154, 205, 50)
    constexpr Color limeGreen()             { return Color(50, 205, 50); }      // gamecoe::Color(50, 205, 50)
    constexpr Color lawnGreen()             { return Color(124, 252, 0); }      // gamecoe::Color(124, 252, 0)
    constexpr Color chartreuse()            { return Color(127, 255, 0); }      // gamecoe::Color(127, 255, 0)
    constexpr Color greenYellow()           { return Color(173, 255, 47); }     // gamecoe::Color(173, 255, 47)
    constexpr Color springGreen()           { return Color(0, 255, 127); }      // gamecoe::Color(0, 255, 127)
    constexpr Color mediumSpringGreen()     { return Color(0, 250, 154); }      // gamecoe::Color(0, 250, 154)
    constexpr Color lightGreen()            { return Color(144, 238, 144); }    // gamecoe::Color(144, 238, 144)
    constexpr Color paleGreen()             { return Color(152, 251, 152); }    // gamecoe::Color(152, 251, 152)

    // Cyan/Turquoise family
    constexpr Color lightSeaGreen()         { return Color(32, 178, 170); }     // gamecoe::Color(32, 178, 170)
    constexpr Color mediumAquaMarine()      { return Color(102, 205, 170); }    // gamecoe::Color(102, 205, 170)
    constexpr Color aquaMarine()            { return Color(127, 255, 212); }    // gamecoe::Color(127, 255, 212)
    constexpr Color darkCyan()              { return Color(0, 139, 139); }      // gamecoe::Color(0, 139, 139)
    constexpr Color darkTurquoise()         { return Color(0, 206, 209); }      // gamecoe::Color(0, 206, 209)
    constexpr Color mediumTurquoise()       { return Color(72, 209, 204); }     // gamecoe::Color(72, 209, 204)
    constexpr Color turquoise()             { return Color(64, 224, 208); }     // gamecoe::Color(64, 224, 208)
    constexpr Color lightCyan()             { return Color(224, 255, 255); }    // gamecoe::Color(224, 255, 255)
    constexpr Color paleTurquoise()         { return Color(175, 238, 238); }    // gamecoe::Color(175, 238, 238)

    // Blue family
    constexpr Color midnightBlue()          { return Color(25, 25, 112); }      // gamecoe::Color(25, 25, 112)
    constexpr Color darkBlue()              { return Color(0, 0, 139); }        // gamecoe::Color(0, 0, 139)
    constexpr Color mediumBlue()            { return Color(0, 0, 205); }        // gamecoe::Color(0, 0, 205)
    constexpr Color royalBlue()             { return Color(65, 105, 225); }     // gamecoe::Color(65, 105, 225)
    constexpr Color steelBlue()             { return Color(70, 130, 180); }     // gamecoe::Color(70, 130, 180)
    constexpr Color dodgerBlue()            { return Color(30, 144, 255); }     // gamecoe::Color(30, 144, 255)
    constexpr Color deepSkyBlue()           { return Color(0, 191, 255); }      // gamecoe::Color(0, 191, 255)
    constexpr Color cornFlowerBlue()        { return Color(100, 149, 237); }    // gamecoe::Color(100, 149, 237)
    constexpr Color skyBlue()               { return Color(135, 206, 235); }    // gamecoe::Color(135, 206, 235)
    constexpr Color lightSkyBlue()          { return Color(135, 206, 250); }    // gamecoe::Color(135, 206, 250)
    constexpr Color lightSteelBlue()        { return Color(176, 196, 222); }    // gamecoe::Color(176, 196, 222)
    constexpr Color lightBlue()             { return Color(173, 216, 230); }    // gamecoe::Color(173, 216, 230)
    constexpr Color powderBlue()            { return Color(176, 224, 230); }    // gamecoe::Color(176, 224, 230)
    constexpr Color cadetBlue()             { return Color(95, 158, 160); }     // gamecoe::Color(95, 158, 160)

    // Purple/Violet family
    constexpr Color indigo()                { return Color(75, 0, 130); }       // gamecoe::Color(75, 0, 130)
    constexpr Color darkMagenta()           { return Color(139, 0, 139); }      // gamecoe::Color(139, 0, 139)
    constexpr Color darkViolet()            { return Color(148, 0, 211); }      // gamecoe::Color(148, 0, 211)
    constexpr Color darkSlateBlue()         { return Color(72, 61, 139); }      // gamecoe::Color(72, 61, 139)
    constexpr Color blueViolet()            { return Color(138, 43, 226); }     // gamecoe::Color(138, 43, 226)
    constexpr Color darkOrchid()            { return Color(153, 50, 204); }     // gamecoe::Color(153, 50, 204)
    constexpr Color slateBlue()             { return Color(106, 90, 205); }     // gamecoe::Color(106, 90, 205)
    constexpr Color mediumSlateBlue()       { return Color(123, 104, 238); }    // gamecoe::Color(123, 104, 238)
    constexpr Color mediumPurple()          { return Color(147, 112, 219); }    // gamecoe::Color(147, 112, 219)
    constexpr Color mediumOrchid()          { return Color(186, 85, 211); }     // gamecoe::Color(186, 85, 211)
    constexpr Color violet()                { return Color(238, 130, 238); }    // gamecoe::Color(238, 130, 238)
    constexpr Color plum()                  { return Color(221, 160, 221); }    // gamecoe::Color(221, 160, 221)
    constexpr Color thistle()               { return Color(216, 191, 216); }    // gamecoe::Color(216, 191, 216)
    constexpr Color orchid()                { return Color(218, 112, 214); }    // gamecoe::Color(218, 112, 214)
    constexpr Color lavender()              { return Color(230, 230, 250); }    // gamecoe::Color(230, 230, 250)

    // Brown/Tan/Beige family
    constexpr Color saddleBrown()           { return Color(139, 69, 19); }      // gamecoe::Color(139, 69, 19)
    constexpr Color sienna()                { return Color(160, 82, 45); }      // gamecoe::Color(160, 82, 45)
    constexpr Color chocolate()             { return Color(210, 105, 30); }     // gamecoe::Color(210, 105, 30)
    constexpr Color peru()                  { return Color(205, 133, 63); }     // gamecoe::Color(205, 133, 63)
    constexpr Color sandyBrown()            { return Color(244, 164, 96); }     // gamecoe::Color(244, 164, 96)
    constexpr Color burlyWood()             { return Color(222, 184, 135); }    // gamecoe::Color(222, 184, 135)
    constexpr Color tan()                   { return Color(210, 180, 140); }    // gamecoe::Color(210, 180, 140)
    constexpr Color rosyBrown()             { return Color(188, 143, 143); }    // gamecoe::Color(188, 143, 143)
    constexpr Color wheat()                 { return Color(245, 222, 179); }    // gamecoe::Color(245, 222, 179)
    constexpr Color navajoWhite()           { return Color(255, 222, 173); }    // gamecoe::Color(255, 222, 173)
    constexpr Color bisque()                { return Color(255, 228, 196); }    // gamecoe::Color(255, 228, 196)
    constexpr Color blanchedAlmond()        { return Color(255, 235, 205); }    // gamecoe::Color(255, 235, 205)
    constexpr Color cornSilk()              { return Color(255, 248, 220); }    // gamecoe::Color(255, 248, 220)
    constexpr Color antiqueWhite()          { return Color(250, 235, 215); }    // gamecoe::Color(250, 235, 215)
    constexpr Color beige()                 { return Color(245, 245, 220); }    // gamecoe::Color(245, 245, 220)

    // White tints
    constexpr Color snow()                  { return Color(255, 250, 250); }    // gamecoe::Color(255, 250, 250)
    constexpr Color mistyRose()             { return Color(255, 228, 225); }    // gamecoe::Color(255, 228, 225)
    constexpr Color seaShell()              { return Color(255, 245, 238); }    // gamecoe::Color(255, 245, 238)
    constexpr Color lavenderBlush()         { return Color(255, 240, 245); }    // gamecoe::Color(255, 240, 245)
    constexpr Color linen()                 { return Color(250, 240, 230); }    // gamecoe::Color(250, 240, 230)
    constexpr Color oldLace()               { return Color(253, 245, 230); }    // gamecoe::Color(253, 245, 230)
    constexpr Color floralWhite()           { return Color(255, 250, 240); }    // gamecoe::Color(255, 250, 240)
    constexpr Color ivory()                 { return Color(255, 255, 240); }    // gamecoe::Color(255, 255, 240)
    constexpr Color honeydew()              { return Color(240, 255, 240); }    // gamecoe::Color(240, 255, 240)
    constexpr Color mintCream()             { return Color(245, 255, 250); }    // gamecoe::Color(245, 255, 250)
    constexpr Color azure()                 { return Color(240, 255, 255); }    // gamecoe::Color(240, 255, 255)
    constexpr Color aliceBlue()             { return Color(240, 248, 255); }    // gamecoe::Color(240, 248, 255)
    constexpr Color ghostWhite()            { return Color(248, 248, 255); }    // gamecoe::Color(248, 248, 255)

} // namespace colorcoe
