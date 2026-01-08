#pragma once

#include <cstddef>
#include <cstdint>

namespace gamecoe
{
    class GraphicsBuffer 
    {
        std::uint32_t m_id;
        std::uint32_t m_target;
        std::uint32_t m_usage;

        GraphicsBuffer(std::uint32_t target);

        void bind() const;
        void unbind() const;

    public:
        ~GraphicsBuffer();
        GraphicsBuffer(const GraphicsBuffer&) = delete;
        GraphicsBuffer &operator=(const GraphicsBuffer&) = delete;
        GraphicsBuffer(GraphicsBuffer &&other) noexcept;
        GraphicsBuffer &operator=(GraphicsBuffer &&other) noexcept;

        // Relevant only for Uniform Buffers
        void bindBase(std::uint32_t bindingPoint) const;

        void uploadData(const void* data, size_t size);

        std::uint32_t id() const;
        std::uint32_t target() const;

        static GraphicsBuffer createVertexBuffer();
        static GraphicsBuffer createIndexBuffer();
        static GraphicsBuffer createUniformBuffer();
    };

    inline GraphicsBuffer VertexBuffer() { return GraphicsBuffer::createVertexBuffer(); }
    inline GraphicsBuffer IndexBuffer() { return GraphicsBuffer::createIndexBuffer(); }
    inline GraphicsBuffer UniformBuffer() { return GraphicsBuffer::createUniformBuffer(); }
} // namespace gamecoe
