#pragma once

#include <gamecoe/entity/renderer/renderer.hpp>
#include <gamecoe/graphics/vertex_array.hpp>
#include <gamecoe/graphics/shader.hpp>
#include <gamecoe_config.hpp>
#include <colorcoe.hpp>
#include <memory>
#include <atomic>
#include <cstdint>

namespace gamecoe
{
    enum class Shape
    {
        Triangle,
        Rectangle,
        Box,
        // TODO: Support more primitive shapes
    };

    class ShapeRenderer : public Renderer
    {
        static std::atomic<std::uint32_t> s_counter;
        static std::optional<Shader> s_shader;
#if GAMECOE_HAS_UBO
        static std::uint32_t s_cameraUniformBlockIndex;
#endif
        
        Shape m_shape;
        Color m_color;
        const VertexArray &m_vertexArray;
    
        ShapeRenderer(GameObject &owner, Shape shape, const Color &color, std::int8_t layer = 0);

    public:
        ShapeRenderer(const ShapeRenderer&) = delete;
        ShapeRenderer& operator=(const ShapeRenderer&) = delete;
        ShapeRenderer(ShapeRenderer&&) = delete;
        ShapeRenderer& operator=(ShapeRenderer&&) = delete;
        virtual ~ShapeRenderer() override;

        virtual void initialize() override {}
        virtual void begin() override {}
        virtual void activate() override { m_active = true; }
        virtual void deactivate() override { m_active = false; }
        virtual void update() override {}
        virtual void render() const override;

        Shape shape() const;
        Color color() const;

        void setColor(const Color &color);

        static std::unique_ptr<ShapeRenderer> triangle(GameObject &owner, const Color &color, std::int8_t layer = 0);
        static std::unique_ptr<ShapeRenderer> rectangle(GameObject &owner, const Color &color, std::int8_t layer = 0);
        static std::unique_ptr<ShapeRenderer> box(GameObject &owner, const Color &color, std::int8_t layer = 0);
    };

} // namespace gamecoe