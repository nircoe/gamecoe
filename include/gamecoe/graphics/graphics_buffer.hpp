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
        bool m_allocated;

        GraphicsBuffer(std::uint32_t target, std::uint32_t bindingPoint = 0);

        void bind() const;
        void unbind() const;

    public:
        ~GraphicsBuffer();
        GraphicsBuffer(const GraphicsBuffer&) = delete;
        GraphicsBuffer &operator=(const GraphicsBuffer&) = delete;
        GraphicsBuffer(GraphicsBuffer &&other) noexcept;
        GraphicsBuffer &operator=(GraphicsBuffer &&other) noexcept;

        void uploadData(const void* data, size_t size);

        std::uint32_t id() const;
        std::uint32_t target() const;

        static GraphicsBuffer createVertexBuffer();
        static GraphicsBuffer createIndexBuffer();
        static GraphicsBuffer createUniformBuffer(std::uint32_t bindingPoint);
    };

    inline GraphicsBuffer VertexBuffer() { return GraphicsBuffer::createVertexBuffer(); }
    inline GraphicsBuffer IndexBuffer() { return GraphicsBuffer::createIndexBuffer(); }
    inline GraphicsBuffer UniformBuffer(std::uint32_t bindingPoint) { return GraphicsBuffer::createUniformBuffer(bindingPoint); }
} // namespace gamecoe
