#include <colorcoe.hpp>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    bool Color::operator==(const Color &other) const
    { 
        return m_red == other.m_red && m_green == other.m_green && m_blue == other.m_blue && m_alpha == other.m_alpha;
    }

    bool Color::operator!=(const Color &other) const 
    { 
        return !(*this == other); 
    }

    std::uint8_t Color::red() const { return m_red; }

    std::uint8_t Color::green() const { return m_green; }
    
    std::uint8_t Color::blue() const { return m_blue; }
    
    std::uint8_t Color::alpha() const { return m_alpha; }
    
    std::array<float, 4> Color::normalized() const
    {
        return { static_cast<float>(m_red) / 255.0f, 
                 static_cast<float>(m_green) / 255.0f, 
                 static_cast<float>(m_blue) / 255.0f, 
                 static_cast<float>(m_alpha) / 255.0f };
    }

    std::uint32_t Color::rgba() const
    {
        return (static_cast<std::uint32_t>(m_red) << 24) |
               (static_cast<std::uint32_t>(m_green) << 16) |
               (static_cast<std::uint32_t>(m_blue) << 8) |
                static_cast<std::uint32_t>(m_alpha);
    }

    Color Color::fromNormalized(float red, float green, float blue, float alpha)
    {
        auto clamp = [](float value) -> std::uint8_t {
            if (value < 0.0f) return 0U;
            if (value > 1.0f) return 255U;
            
            return static_cast<std::uint8_t>(value * 255.0f);
        };
        return Color(clamp(red), clamp(green), clamp(blue), clamp(alpha));
    }

    Color Color::fromHex(const std::string &hex)
    {
        if (!hex.starts_with('#') || (hex.length() != 7 && hex.length() != 9))
            detail::invalidArgument("Color::fromHex: The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal");

        auto parseHexDigit = [](char c) -> std::uint8_t {
            if ('0' <= c && c <= '9')       return c - '0';
            else if ('A' <= c && c <= 'F')  return 10U + (c - 'A');
            else if ('a' <= c && c <= 'f')  return 10U + (c - 'a');

            detail::invalidArgument("Color::fromHex: The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal");
        };
        
        std::array<std::uint8_t, 4> values = { 0U, 0U, 0U, 255U };
        int iterations = (hex.length() - 1) / 2; // 3 or 4
        for(int i = 0; i < iterations; ++i)
        {
            char first = hex[(2 * i) + 1];
            char second = hex[(2 * i) + 2];

            values[i] = (parseHexDigit(first) << 4) | parseHexDigit(second);
        }

        return Color(values[0], values[1], values[2], values[3]);
    }

    Color Color::lerp(const Color &a, const Color &b, float t)
    {
        if (t < 0.0f)       t = 0.0f;
        else if (t > 1.0f)  t = 1.0f;

        int redDiff = b.m_red - a.m_red;
        int greenDiff = b.m_green - a.m_green;
        int blueDiff = b.m_blue - a.m_blue;
        int alphaDiff = b.m_alpha - a.m_alpha;

        return Color(a.m_red + (t * redDiff), 
                     a.m_green + (t * greenDiff), 
                     a.m_blue + (t * blueDiff), 
                     a.m_alpha + (t * alphaDiff));
    }

} // namespace gamecoe
