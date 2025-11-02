#pragma once

namespace gamecoe
{
    class GraphicsBuffer 
    {
        unsigned int m_id;
        unsigned int m_target;

        GraphicsBuffer(unsigned int target);

    public:
        ~GraphicsBuffer();
        GraphicsBuffer(const GraphicsBuffer&) = delete;
        GraphicsBuffer &operator=(const GraphicsBuffer&) = delete;
        GraphicsBuffer(GraphicsBuffer &&other) noexcept;
        GraphicsBuffer &operator=(GraphicsBuffer &&other) noexcept;

        void bind() const;
        void unbind() const;

        static GraphicsBuffer createVertexBuffer();
        static GraphicsBuffer createIndexBuffer();
    };

    inline GraphicsBuffer VertexBuffer() { return GraphicsBuffer::createVertexBuffer(); }
    inline GraphicsBuffer IndexBuffer() { return GraphicsBuffer::createIndexBuffer(); }
} // namespace gamecoe
