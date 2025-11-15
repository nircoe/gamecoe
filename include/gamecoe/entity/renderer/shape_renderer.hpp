#pragma once

#include <gamecoe/entity/renderer/renderer.hpp>
#include <gamecoe/graphics/vertex_array.hpp>
#include <gamecoe/graphics/shader.hpp>
#include <memory>
#include <atomic>

namespace gamecoe
{
    enum class Shape
    {
        Triangle,
        Rectangle,
        Cube,
        // TODO: Support more primitive shapes
    };

    class ShapeRenderer : public Renderer
    {
        static std::atomic<uint32_t> s_counter;
        static std::optional<Shader> s_shader;
        
        Shape m_shape;
        const VertexArray &m_vertexArray;
    
        ShapeRenderer(GameObject *owner, Shape shape, std::int8_t layer = 0);

    public:
        virtual ~ShapeRenderer() override;

        virtual void initialize() override {}
        virtual void begin() override {}
        virtual void activate() override { m_active = true; }
        virtual void deactivate() override { m_active = false; }
        virtual void update() override {}
        virtual void render() const override;

        Shape shape() const;

        static std::unique_ptr<ShapeRenderer> triangle(GameObject *owner, std::int8_t layer = 0);
        static std::unique_ptr<ShapeRenderer> rectangle(GameObject *owner, std::int8_t layer = 0);
        static std::unique_ptr<ShapeRenderer> cube(GameObject *owner, std::int8_t layer = 0);
    };

} // namespace gamecoe