#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace gamecoe
{
    class Color
    {
        std::uint8_t m_red;
        std::uint8_t m_green;
        std::uint8_t m_blue;
        std::uint8_t m_alpha;

    public:
        constexpr Color();
        constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255U);
        
        Color(const Color &color) = default;
        Color &operator=(const Color &color) = default;
        ~Color() = default;

        bool operator==(const Color &other) const;
        bool operator!=(const Color &other) const;

        std::uint8_t red() const;
        std::uint8_t green() const;
        std::uint8_t blue() const;
        std::uint8_t alpha() const;
        
        std::array<float, 4> normalized() const;

        std::uint32_t rgba() const;

        static Color fromNormalized(float red, float green, float blue, float alpha = 1.0f); // Clamps values to [0.0f, 1.0f]
        static Color fromHex(const std::string &hex); // Color::fromHex("#RRGGBBAA") or Color::fromHex("#RRGGBB")
        static Color lerp(const Color &a, const Color &b, float t); // Lerps between Color a and Color b by t
        // anymore methods?
    };

} // namespace gamecoe

namespace colorcoe
{
    using namespace gamecoe;

    // Basic colors
    constexpr Color black()     { return Color(); }                 // gamecoe::Color(0, 0, 0)
    Color white()     { return Color(255, 255, 255); }    // gamecoe::Color(255, 255, 255)
    Color red()       { return Color(255, 0, 0); }        // gamecoe::Color(255, 0, 0)
    Color lime()      { return Color(0, 255, 0); }        // gamecoe::Color(0, 255, 0)
    Color blue()      { return Color(0, 0, 255); }        // gamecoe::Color(0, 0, 255)
    Color yellow()    { return Color(255, 255, 0); }      // gamecoe::Color(255, 255, 0)
    Color aqua()      { return Color(0, 255, 255); }      // gamecoe::Color(0, 255, 255)
    Color cyan()      { return Color(0, 255, 255); }      // gamecoe::Color(0, 255, 255)
    Color magenta()   { return Color(255, 0, 255); }      // gamecoe::Color(255, 0, 255)
    Color fuchsia()   { return Color(255, 0, 255); }      // gamecoe::Color(255, 0, 255)
    Color silver()    { return Color(192, 192, 192); }    // gamecoe::Color(192, 192, 192)
    Color gray()      { return Color(128, 128, 128); }    // gamecoe::Color(128, 128, 128)
    Color maroon()    { return Color(128, 0, 0); }        // gamecoe::Color(128, 0, 0)
    Color olive()     { return Color(128, 128, 0); }      // gamecoe::Color(128, 128, 0)
    Color green()     { return Color(0, 128, 0); }        // gamecoe::Color(0, 128, 0)
    Color purple()    { return Color(128, 0, 128); }      // gamecoe::Color(128, 0, 128)
    Color teal()      { return Color(0, 128, 128); }      // gamecoe::Color(0, 128, 128)
    Color navy()      { return Color(0, 0, 128); }        // gamecoe::Color(0, 0, 128)

    // Red colors
    Color darkRed()        { return Color(139, 0, 0); }        // gamecoe::Color(139, 0, 0)
    Color brown()          { return Color(165, 42, 42); }      // gamecoe::Color(165, 42, 42)
    Color firebrick()      { return Color(178, 34, 34); }      // gamecoe::Color(178, 34, 34)
    Color crimson()        { return Color(220, 20, 60); }      // gamecoe::Color(220, 20, 60)
    Color tomato()         { return Color(255, 99, 71); }      // gamecoe::Color(255, 99, 71)
    Color coral()          { return Color(255, 127, 80); }     // gamecoe::Color(255, 127, 80)
    Color indianRed()      { return Color(205, 92, 92); }      // gamecoe::Color(205, 92, 92)
    Color lightCoral()     { return Color(240, 128, 128); }    // gamecoe::Color(240, 128, 128)
    Color darkSalmon()     { return Color(233, 150, 122); }    // gamecoe::Color(233, 150, 122)
    Color salmon()         { return Color(250, 128, 114); }    // gamecoe::Color(250, 128, 114)
    Color lightSalmon()    { return Color(255, 160, 122); }    // gamecoe::Color(255, 160, 122)

    // Orange colors
    Color orangeRed()      { return Color(255, 69, 0); }       // gamecoe::Color(255, 69, 0)
    Color darkOrange()     { return Color(255, 140, 0); }      // gamecoe::Color(255, 140, 0)
    Color orange()         { return Color(255, 165, 0); }      // gamecoe::Color(255, 165, 0)

    // Gold/Yellow colors
    Color gold()                 { return Color(255, 215, 0); }       // gamecoe::Color(255, 215, 0)
    Color darkGoldenRod()        { return Color(184, 134, 11); }      // gamecoe::Color(184, 134, 11)
    Color goldenRod()            { return Color(218, 165, 32); }      // gamecoe::Color(218, 165, 32)
    Color paleGoldenRod()        { return Color(238, 232, 170); }     // gamecoe::Color(238, 232, 170)
    Color darkKhaki()            { return Color(189, 183, 107); }     // gamecoe::Color(189, 183, 107)
    Color khaki()                { return Color(240, 230, 140); }     // gamecoe::Color(240, 230, 140)
    Color yellowGreen()          { return Color(154, 205, 50); }      // gamecoe::Color(154, 205, 50)
    Color darkOliveGreen()       { return Color(85, 107, 47); }       // gamecoe::Color(85, 107, 47)
    Color oliveDrab()            { return Color(107, 142, 35); }      // gamecoe::Color(107, 142, 35)
    Color lawnGreen()            { return Color(124, 252, 0); }       // gamecoe::Color(124, 252, 0)
    Color chartreuse()           { return Color(127, 255, 0); }       // gamecoe::Color(127, 255, 0)
    Color greenYellow()          { return Color(173, 255, 47); }      // gamecoe::Color(173, 255, 47)
    Color lightGoldenRodYellow() { return Color(250, 250, 210); }     // gamecoe::Color(250, 250, 210)
    Color lightYellow()          { return Color(255, 255, 224); }     // gamecoe::Color(255, 255, 224)

    // Green colors
    Color darkGreen()          { return Color(0, 100, 0); }       // gamecoe::Color(0, 100, 0)
    Color forestGreen()        { return Color(34, 139, 34); }     // gamecoe::Color(34, 139, 34)
    Color limeGreen()          { return Color(50, 205, 50); }     // gamecoe::Color(50, 205, 50)
    Color lightGreen()         { return Color(144, 238, 144); }   // gamecoe::Color(144, 238, 144)
    Color paleGreen()          { return Color(152, 251, 152); }   // gamecoe::Color(152, 251, 152)
    Color darkSeaGreen()       { return Color(143, 188, 143); }   // gamecoe::Color(143, 188, 143)
    Color mediumSpringGreen()  { return Color(0, 250, 154); }     // gamecoe::Color(0, 250, 154)
    Color springGreen()        { return Color(0, 255, 127); }     // gamecoe::Color(0, 255, 127)
    Color seaGreen()           { return Color(46, 139, 87); }     // gamecoe::Color(46, 139, 87)
    Color mediumAquaMarine()   { return Color(102, 205, 170); }   // gamecoe::Color(102, 205, 170)
    Color mediumSeaGreen()     { return Color(60, 179, 113); }    // gamecoe::Color(60, 179, 113)
    Color lightSeaGreen()      { return Color(32, 178, 170); }    // gamecoe::Color(32, 178, 170)

    // Cyan/Turquoise colors
    Color darkSlateGray()   { return Color(47, 79, 79); }       // gamecoe::Color(47, 79, 79)
    Color darkCyan()        { return Color(0, 139, 139); }      // gamecoe::Color(0, 139, 139)
    Color lightCyan()       { return Color(224, 255, 255); }    // gamecoe::Color(224, 255, 255)
    Color darkTurquoise()   { return Color(0, 206, 209); }      // gamecoe::Color(0, 206, 209)
    Color turquoise()       { return Color(64, 224, 208); }     // gamecoe::Color(64, 224, 208)
    Color mediumTurquoise() { return Color(72, 209, 204); }     // gamecoe::Color(72, 209, 204)
    Color paleTurquoise()   { return Color(175, 238, 238); }    // gamecoe::Color(175, 238, 238)
    Color aquaMarine()      { return Color(127, 255, 212); }    // gamecoe::Color(127, 255, 212)

    // Blue colors
    Color powderBlue()      { return Color(176, 224, 230); }    // gamecoe::Color(176, 224, 230)
    Color cadetBlue()       { return Color(95, 158, 160); }     // gamecoe::Color(95, 158, 160)
    Color steelBlue()       { return Color(70, 130, 180); }     // gamecoe::Color(70, 130, 180)
    Color cornFlowerBlue()  { return Color(100, 149, 237); }    // gamecoe::Color(100, 149, 237)
    Color deepSkyBlue()     { return Color(0, 191, 255); }      // gamecoe::Color(0, 191, 255)
    Color dodgerBlue()      { return Color(30, 144, 255); }     // gamecoe::Color(30, 144, 255)
    Color lightBlue()       { return Color(173, 216, 230); }    // gamecoe::Color(173, 216, 230)
    Color skyBlue()         { return Color(135, 206, 235); }    // gamecoe::Color(135, 206, 235)
    Color lightSkyBlue()    { return Color(135, 206, 250); }    // gamecoe::Color(135, 206, 250)
    Color midnightBlue()    { return Color(25, 25, 112); }      // gamecoe::Color(25, 25, 112)
    Color darkBlue()        { return Color(0, 0, 139); }        // gamecoe::Color(0, 0, 139)
    Color mediumBlue()      { return Color(0, 0, 205); }        // gamecoe::Color(0, 0, 205)
    Color royalBlue()       { return Color(65, 105, 225); }     // gamecoe::Color(65, 105, 225)

    // Purple/Violet colors
    Color blueViolet()       { return Color(138, 43, 226); }    // gamecoe::Color(138, 43, 226)
    Color indigo()           { return Color(75, 0, 130); }      // gamecoe::Color(75, 0, 130)
    Color darkSlateBlue()    { return Color(72, 61, 139); }     // gamecoe::Color(72, 61, 139)
    Color slateBlue()        { return Color(106, 90, 205); }    // gamecoe::Color(106, 90, 205)
    Color mediumSlateBlue()  { return Color(123, 104, 238); }   // gamecoe::Color(123, 104, 238)
    Color mediumPurple()     { return Color(147, 112, 219); }   // gamecoe::Color(147, 112, 219)
    Color darkMagenta()      { return Color(139, 0, 139); }     // gamecoe::Color(139, 0, 139)
    Color darkViolet()       { return Color(148, 0, 211); }     // gamecoe::Color(148, 0, 211)
    Color darkOrchid()       { return Color(153, 50, 204); }    // gamecoe::Color(153, 50, 204)
    Color mediumOrchid()     { return Color(186, 85, 211); }    // gamecoe::Color(186, 85, 211)
    Color thistle()          { return Color(216, 191, 216); }   // gamecoe::Color(216, 191, 216)
    Color plum()             { return Color(221, 160, 221); }   // gamecoe::Color(221, 160, 221)
    Color violet()           { return Color(238, 130, 238); }   // gamecoe::Color(238, 130, 238)
    Color orchid()           { return Color(218, 112, 214); }   // gamecoe::Color(218, 112, 214)
    Color mediumVioletRed()  { return Color(199, 21, 133); }    // gamecoe::Color(199, 21, 133)
    Color paleVioletRed()    { return Color(219, 112, 147); }   // gamecoe::Color(219, 112, 147)

    // Pink colors
    Color deepPink()    { return Color(255, 20, 147); }     // gamecoe::Color(255, 20, 147)
    Color hotPink()     { return Color(255, 105, 180); }    // gamecoe::Color(255, 105, 180)
    Color lightPink()   { return Color(255, 182, 193); }    // gamecoe::Color(255, 182, 193)
    Color pink()        { return Color(255, 192, 203); }    // gamecoe::Color(255, 192, 203)

    // Beige/Tan colors
    Color antiqueWhite()   { return Color(250, 235, 215); }    // gamecoe::Color(250, 235, 215)
    Color beige()          { return Color(245, 245, 220); }    // gamecoe::Color(245, 245, 220)
    Color bisque()         { return Color(255, 228, 196); }    // gamecoe::Color(255, 228, 196)
    Color blanchedAlmond() { return Color(255, 235, 205); }    // gamecoe::Color(255, 235, 205)
    Color wheat()          { return Color(245, 222, 179); }    // gamecoe::Color(245, 222, 179)
    Color cornSilk()       { return Color(255, 248, 220); }    // gamecoe::Color(255, 248, 220)
    Color lemonChiffon()   { return Color(255, 250, 205); }    // gamecoe::Color(255, 250, 205)
    Color saddleBrown()    { return Color(139, 69, 19); }      // gamecoe::Color(139, 69, 19)
    Color sienna()         { return Color(160, 82, 45); }      // gamecoe::Color(160, 82, 45)
    Color chocolate()      { return Color(210, 105, 30); }     // gamecoe::Color(210, 105, 30)
    Color peru()           { return Color(205, 133, 63); }     // gamecoe::Color(205, 133, 63)
    Color sandyBrown()     { return Color(244, 164, 96); }     // gamecoe::Color(244, 164, 96)
    Color burlyWood()      { return Color(222, 184, 135); }    // gamecoe::Color(222, 184, 135)
    Color tan()            { return Color(210, 180, 140); }    // gamecoe::Color(210, 180, 140)
    Color rosyBrown()      { return Color(188, 143, 143); }    // gamecoe::Color(188, 143, 143)
    Color moccasin()       { return Color(255, 228, 181); }    // gamecoe::Color(255, 228, 181)
    Color navajoWhite()    { return Color(255, 222, 173); }    // gamecoe::Color(255, 222, 173)
    Color peachPuff()      { return Color(255, 218, 185); }    // gamecoe::Color(255, 218, 185)
    Color mistyRose()      { return Color(255, 228, 225); }    // gamecoe::Color(255, 228, 225)
    Color lavenderBlush()  { return Color(255, 240, 245); }    // gamecoe::Color(255, 240, 245)
    Color linen()          { return Color(250, 240, 230); }    // gamecoe::Color(250, 240, 230)
    Color oldLace()        { return Color(253, 245, 230); }    // gamecoe::Color(253, 245, 230)
    Color papayaWhip()     { return Color(255, 239, 213); }    // gamecoe::Color(255, 239, 213)
    Color seaShell()       { return Color(255, 245, 238); }    // gamecoe::Color(255, 245, 238)
    Color mintCream()      { return Color(245, 255, 250); }    // gamecoe::Color(245, 255, 250)

    // Gray/White colors
    Color slateGray()      { return Color(112, 128, 144); }    // gamecoe::Color(112, 128, 144)
    Color lightSlateGray() { return Color(119, 136, 153); }    // gamecoe::Color(119, 136, 153)
    Color lightSteelBlue() { return Color(176, 196, 222); }    // gamecoe::Color(176, 196, 222)
    Color lavender()       { return Color(230, 230, 250); }    // gamecoe::Color(230, 230, 250)
    Color floralWhite()    { return Color(255, 250, 240); }    // gamecoe::Color(255, 250, 240)
    Color aliceBlue()      { return Color(240, 248, 255); }    // gamecoe::Color(240, 248, 255)
    Color ghostWhite()     { return Color(248, 248, 255); }    // gamecoe::Color(248, 248, 255)
    Color honeydew()       { return Color(240, 255, 240); }    // gamecoe::Color(240, 255, 240)
    Color ivory()          { return Color(255, 255, 240); }    // gamecoe::Color(255, 255, 240)
    Color azure()          { return Color(240, 255, 255); }    // gamecoe::Color(240, 255, 255)
    Color snow()           { return Color(255, 250, 250); }    // gamecoe::Color(255, 250, 250)
    Color dimGray()        { return Color(105, 105, 105); }    // gamecoe::Color(105, 105, 105)
    Color darkGray()       { return Color(169, 169, 169); }    // gamecoe::Color(169, 169, 169)
    Color lightGray()      { return Color(211, 211, 211); }    // gamecoe::Color(211, 211, 211)
    Color gainsboro()      { return Color(220, 220, 220); }    // gamecoe::Color(220, 220, 220)
    Color whiteSmoke()     { return Color(245, 245, 245); }    // gamecoe::Color(245, 245, 245)

} // namespace colorcoe
