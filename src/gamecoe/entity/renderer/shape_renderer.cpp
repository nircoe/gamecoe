#include <gamecoe/entity/renderer/shape_renderer.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/camera.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/paths.hpp>
#include <gamecoe/utils/consts.hpp>
#include <cassert>
#include <optional>
#include <glm/glm.hpp>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

namespace
{
    const gamecoe::VertexArray& shapeToVertexArray(gamecoe::Shape shape)
    {
        switch (shape)
        {
        case gamecoe::Shape::Triangle:
            return gamecoe::VertexArray::triangle();
        case gamecoe::Shape::Rectangle:
            return gamecoe::VertexArray::rectangle();
        case gamecoe::Shape::Box:
            return gamecoe::VertexArray::box();
        
        default:
            assert(false && "Need to add support for a new Shape");
            break;
        }
    }
}

namespace gamecoe
{
    std::atomic<std::uint32_t> ShapeRenderer::s_counter = 0;
    std::optional<Shader> ShapeRenderer::s_shader = std::nullopt;
#if GAMECOE_HAS_UBO
    std::uint32_t ShapeRenderer::s_cameraUniformBlockIndex = 0;
#endif

    ShapeRenderer::ShapeRenderer(GameObject &owner, Shape shape, const Color &color, std::int8_t layer) : 
        Renderer(owner, layer), 
        m_shape(shape),
        m_color(color),
        m_vertexArray(shapeToVertexArray(shape)) 
    { 
        ++s_counter;
    }

    ShapeRenderer::~ShapeRenderer()
    {
        --s_counter;
        if (s_counter == 0)
        {
            s_shader.reset();
            VertexArray::destroyShapeVAs();
        }
    }

    void ShapeRenderer::render() const
    {
        if (!m_active) return;
        
        if (!s_shader)
        {
            // TODO: Support passing macros into Shader class 
            // In order to unite those 2 vertex shaders with #ifdef statements
#if GAMECOE_HAS_UBO
            s_shader.emplace(
                resolvePath("gamecoe/shaders/shape_renderer_ubo.vert"), 
                resolvePath("gamecoe/shaders/shape_renderer.frag")
            );
            s_cameraUniformBlockIndex = glGetUniformBlockIndex(s_shader->id(), "CameraMatrices");
            glUniformBlockBinding(s_shader->id(), s_cameraUniformBlockIndex, constcoe::CAMERA_UBO_BINDING_POINT);
#else
            s_shader.emplace(
                resolvePath("gamecoe/shaders/shape_renderer.vert"), 
                resolvePath("gamecoe/shaders/shape_renderer.frag")
            );
#endif
        }

#if !GAMECOE_HAS_DSA
        s_shader->use();
#endif
#if !GAMECOE_HAS_UBO
        auto &camera = owner().game().mainCamera();
        s_shader->set("view", camera.viewMatrix());
        s_shader->set("projection", camera.projectionMatrix());
#endif
        s_shader->set("model", owner().transform().modelMatrix());
        s_shader->set("color", m_color.normalized());

        m_vertexArray.bind();

        if (m_vertexArray.hasIndices())
            glDrawElements(GL_TRIANGLES, m_vertexArray.indexCount(), GL_UNSIGNED_INT, (void*)0);
        else
            glDrawArrays(GL_TRIANGLES, 0, m_vertexArray.vertexCount());
    }

    Shape ShapeRenderer::shape() const { return m_shape; }

    Color ShapeRenderer::color() const { return m_color; }

    void ShapeRenderer::setColor(const Color &color)
    {
        m_color = color;
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::triangle(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Triangle, color, layer));
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::rectangle(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Rectangle, color, layer));
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::box(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Box, color, layer));
    }
} // namespace gamecoe
