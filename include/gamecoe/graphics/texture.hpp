#pragma once

#include <cstdint>
#include <string>
#include <expected>
#include <gamecoe/utils/error.hpp>

namespace gamecoe
{
    namespace graphics
    {
        enum class texture_wrap { repeat, clamp_to_edge, clamp_to_border, mirrored_repeat };
        enum class texture_filter { nearest, linear, nearest_mipmap_nearest, nearest_mipmap_linear,
                                    linear_mipmap_nearest, linear_mipmap_linear };

        class texture
        {
            std::uint32_t m_id;
            std::int32_t m_dimension;

            texture(std::uint32_t id, std::int32_t dimension);
            void destroy();

        public:
            texture(const texture&) = delete;
            texture &operator=(const texture&) = delete;
            texture(texture &&other) noexcept;
            texture &operator=(texture &&other) noexcept;
            ~texture();

            [[nodiscard]] static std::expected<texture, error> create_2d(
                const std::string &image, bool flip_vertically = true, bool generate_mipmap = true);

            void bind() const;
            void unbind() const;
            std::uint32_t id() const;
            std::int32_t dimension() const;

            [[nodiscard]] std::expected<void, error> set_parameters(
                texture_wrap wrap_s, texture_wrap wrap_t, texture_filter min_filter, texture_filter mag_filter);
        };
    } // namespace graphics
} // namespace gamecoe
