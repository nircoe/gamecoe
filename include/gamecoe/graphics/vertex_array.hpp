#pragma once

#include <cstdint>
#include <cstddef>
#include <expected>
#include <optional>
#include <gamecoe/graphics/buffer.hpp>
#include <gamecoe/utils/error.hpp>

namespace gamecoe
{
    namespace graphics
    {
        class vertex_array
        {
            std::size_t m_vertex_count;
            std::size_t m_index_count;
            buffer m_vertex_buffer;
            std::optional<buffer> m_index_buffer;
            std::uint32_t m_id;

            vertex_array(std::uint32_t id, buffer &&vertex_buffer, std::optional<buffer> &&index_buffer,
                         std::size_t vertex_count, std::size_t index_count);
            void destroy();

        public:
            vertex_array(const vertex_array&) = delete;
            vertex_array &operator=(const vertex_array&) = delete;
            vertex_array(vertex_array &&other) noexcept;
            vertex_array &operator=(vertex_array &&other) noexcept;
            ~vertex_array();

            void bind() const;
            void unbind() const;

            std::size_t vertex_count() const;
            std::size_t index_count() const;
            bool has_indices() const;

            [[nodiscard]] static std::expected<vertex_array, error> create(
                const float *vertices, std::size_t vertex_count, std::size_t vertex_size,
                const std::uint32_t *indices = nullptr, std::size_t index_count = 0);

            static const vertex_array *triangle();
            static const vertex_array *rectangle();
            static const vertex_array *box();
            static void destroy_shape_vertex_arrays();
        };
    } // namespace graphics
} // namespace gamecoe
