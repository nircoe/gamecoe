#include <colorcoe.hpp>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    bool color::operator==(const color &other) const
    {
        return m_red == other.m_red && m_green == other.m_green && m_blue == other.m_blue && m_alpha == other.m_alpha;
    }

    bool color::operator!=(const color &other) const
    {
        return !(*this == other);
    }

    std::uint8_t color::red() const { return m_red; }

    std::uint8_t color::green() const { return m_green; }

    std::uint8_t color::blue() const { return m_blue; }

    std::uint8_t color::alpha() const { return m_alpha; }

    glm::vec4 color::normalized() const
    {
        return { static_cast<float>(m_red) / 255.0f,
                 static_cast<float>(m_green) / 255.0f,
                 static_cast<float>(m_blue) / 255.0f,
                 static_cast<float>(m_alpha) / 255.0f };
    }

    std::uint32_t color::rgba() const
    {
        return (static_cast<std::uint32_t>(m_red) << 24) |
               (static_cast<std::uint32_t>(m_green) << 16) |
               (static_cast<std::uint32_t>(m_blue) << 8) |
                static_cast<std::uint32_t>(m_alpha);
    }

    color color::from_normalized(float red, float green, float blue, float alpha)
    {
        auto clamp = [](float value) -> std::uint8_t {
            if (value < 0.0f) return 0U;
            if (value > 1.0f) return 255U;

            return static_cast<std::uint8_t>(value * 255.0f);
        };
        return color(clamp(red), clamp(green), clamp(blue), clamp(alpha));
    }

    std::expected<color, error> color::from_hex(const std::string &hex)
    {
        if (!hex.starts_with('#') || (hex.length() != 7 && hex.length() != 9))
            return std::unexpected(
                    detail::invalid_argument(
                        "color::from_hex(): The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal"));

        auto parse_hex_digit = [](char c) -> std::expected<std::uint8_t, error> {
            if ('0' <= c && c <= '9')       return static_cast<std::uint8_t>(c - '0');
            else if ('A' <= c && c <= 'F')  return static_cast<std::uint8_t>(10U + (c - 'A'));
            else if ('a' <= c && c <= 'f')  return static_cast<std::uint8_t>(10U + (c - 'a'));

            return std::unexpected(
                    detail::invalid_argument(
                        "color::from_hex(): The string argument should be in format of \"#RRGGBBAA\" or \"#RRGGBB\" in hexadecimal"));
        };

        std::uint8_t values[4] = { 0U, 0U, 0U, 255U };
        std::size_t iterations = (hex.length() - 1) / 2; // 3 or 4
        for(std::size_t i = 0; i < iterations; ++i)
        {
            char first = hex[(2 * i) + 1];
            char second = hex[(2 * i) + 2];

            auto first_result = parse_hex_digit(first);
            if (!first_result) return std::unexpected(first_result.error());

            auto second_result = parse_hex_digit(second);
            if (!second_result) return std::unexpected(second_result.error());

            values[i] = (first_result.value() << 4) | second_result.value();
        }

        return color(values[0], values[1], values[2], values[3]);
    }

    color color::lerp(const color &a, const color &b, float t)
    {
        if (t < 0.0f)       t = 0.0f;
        else if (t > 1.0f)  t = 1.0f;

        std::int16_t red_diff = b.m_red - a.m_red;
        std::int16_t green_diff = b.m_green - a.m_green;
        std::int16_t blue_diff = b.m_blue - a.m_blue;
        std::int16_t alpha_diff = b.m_alpha - a.m_alpha;

        return color(static_cast<std::uint8_t>(a.m_red + static_cast<std::int16_t>(t * red_diff)),
                     static_cast<std::uint8_t>(a.m_green + static_cast<std::int16_t>(t * green_diff)),
                     static_cast<std::uint8_t>(a.m_blue + static_cast<std::int16_t>(t * blue_diff)),
                     static_cast<std::uint8_t>(a.m_alpha + static_cast<std::int16_t>(t * alpha_diff)));
    }

} // namespace gamecoe
