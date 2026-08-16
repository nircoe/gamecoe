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
    
    glm::vec4 Color::normalized() const
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

    std::expected<Color, error> Color::fromHex(const std::string &hex)
    {
        if (!hex.starts_with('#') || (hex.length() != 7 && hex.length() != 9))
            return std::unexpected(
                    detail::invalid_argument(
                        "Color::fromHex(): The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal"));

        auto parseHexDigit = [](char c) -> std::expected<std::uint8_t, error> {
            if ('0' <= c && c <= '9')       return c - '0';
            else if ('A' <= c && c <= 'F')  return 10U + (c - 'A');
            else if ('a' <= c && c <= 'f')  return 10U + (c - 'a');

            return std::unexpected(
                    detail::invalid_argument(
                        "Color::fromHex(): The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal"));
        };

        std::uint8_t values[4] = { 0U, 0U, 0U, 255U };
        std::size_t iterations = (hex.length() - 1) / 2; // 3 or 4
        for(std::size_t i = 0; i < iterations; ++i)
        {
            char first = hex[(2 * i) + 1];
            char second = hex[(2 * i) + 2];

            auto firstResult = parseHexDigit(first);
            if (!firstResult) return std::unexpected(firstResult.error());

            auto secondResult = parseHexDigit(second);
            if (!secondResult) return std::unexpected(secondResult.error());

            values[i] = (firstResult.value() << 4) | secondResult.value();
        }

        return Color(values[0], values[1], values[2], values[3]);
    }

    Color Color::lerp(const Color &a, const Color &b, float t)
    {
        if (t < 0.0f)       t = 0.0f;
        else if (t > 1.0f)  t = 1.0f;

        std::int16_t redDiff = b.m_red - a.m_red;
        std::int16_t greenDiff = b.m_green - a.m_green;
        std::int16_t blueDiff = b.m_blue - a.m_blue;
        std::int16_t alphaDiff = b.m_alpha - a.m_alpha;

        return Color(static_cast<std::uint8_t>(a.m_red + static_cast<std::int16_t>(t * redDiff)),
                     static_cast<std::uint8_t>(a.m_green + static_cast<std::int16_t>(t * greenDiff)),
                     static_cast<std::uint8_t>(a.m_blue + static_cast<std::int16_t>(t * blueDiff)),
                     static_cast<std::uint8_t>(a.m_alpha + static_cast<std::int16_t>(t * alphaDiff)));
    }

} // namespace gamecoe
