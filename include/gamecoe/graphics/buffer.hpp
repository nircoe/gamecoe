#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <gamecoe/utils/error.hpp>

namespace gamecoe
{
    namespace graphics
    {
        class buffer
        {
            std::uint32_t m_id;
            std::uint32_t m_target;
            std::uint32_t m_usage;
            bool m_allocated;

            buffer(std::uint32_t id, std::uint32_t target, std::uint32_t usage);

            [[nodiscard]] static std::expected<buffer, error> create(std::uint32_t target, std::uint32_t binding_point = 0);

            void bind() const;
            void unbind() const;

        public:
            buffer(const buffer&) = delete;
            buffer &operator=(const buffer&) = delete;
            buffer(buffer &&other) noexcept;
            buffer &operator=(buffer &&other) noexcept;
            ~buffer();

            [[nodiscard]] std::expected<void, error> upload_data(const void *data, std::size_t size);

            std::uint32_t id() const;
            std::uint32_t target() const;

            [[nodiscard]] static std::expected<buffer, error> create_vertex_buffer();
            [[nodiscard]] static std::expected<buffer, error> create_index_buffer();
            [[nodiscard]] static std::expected<buffer, error> create_uniform_buffer(std::uint32_t binding_point);
        };

        [[nodiscard]] inline std::expected<buffer, error> vertex_buffer() { return buffer::create_vertex_buffer(); }
        [[nodiscard]] inline std::expected<buffer, error> index_buffer() { return buffer::create_index_buffer(); }
        [[nodiscard]] inline std::expected<buffer, error> uniform_buffer(std::uint32_t binding_point)
        {
            return buffer::create_uniform_buffer(binding_point);
        }
    } // namespace graphics
} // namespace gamecoe
