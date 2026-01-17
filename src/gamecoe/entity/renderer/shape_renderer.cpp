#include <gamecoe/entity/renderer/shape_renderer.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/camera.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/paths.hpp>
#include <gamecoe/utils/consts.hpp>
#include <cassert>
#include <optional>
#include <utility>
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
        case gamecoe::Shape::Circle:
            return gamecoe::VertexArray::rectangle();
        case gamecoe::Shape::Box:
        case gamecoe::Shape::Sphere:
        default:
            return gamecoe::VertexArray::box();
        }
    }

    std::pair<std::string, std::string> shapeToShaderPaths(gamecoe::Shape shape)
    {
        switch (shape)
        {
        case gamecoe::Shape::Circle:
            return { "gamecoe/shaders/circle_renderer.vert", "gamecoe/shaders/circle_renderer.frag" };
        case gamecoe::Shape::Sphere:
            return { "gamecoe/shaders/sphere_renderer.vert", "gamecoe/shaders/sphere_renderer.frag" };
        case gamecoe::Shape::Triangle:
        case gamecoe::Shape::Rectangle:
        case gamecoe::Shape::Box:
        default:
            return { "gamecoe/shaders/shape_renderer.vert", "gamecoe/shaders/shape_renderer.frag" };
        }
    }
}

namespace gamecoe
{
    std::atomic<std::uint32_t> ShapeRenderer::s_counter = 0;
    std::optional<Shader> ShapeRenderer::s_shapeShader = std::nullopt;
    std::optional<Shader> ShapeRenderer::s_circleShader = std::nullopt;
    std::optional<Shader> ShapeRenderer::s_sphereShader = std::nullopt;

    std::optional<gamecoe::Shader> &ShapeRenderer::shapeToShader(gamecoe::Shape shape) const
    {
        switch (shape)
        {
        case Shape::Circle:
            return s_circleShader;
        case Shape::Sphere:
            return s_sphereShader;
        case Shape::Triangle:
        case Shape::Rectangle:
        case Shape::Box:
        default:
            return s_shapeShader;
        }
    }

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
            s_shapeShader.reset();
            s_circleShader.reset();
            s_sphereShader.reset();
            VertexArray::destroyShapeVAs();
        }
    }

    void ShapeRenderer::render() const
    {
        if (!m_active) return;
        
        std::optional<Shader> &shader = shapeToShader(m_shape);
        if (!shader)
        {
            auto shaderPaths = shapeToShaderPaths(m_shape);
            shader.emplace(resolvePath(shaderPaths.first), resolvePath(shaderPaths.second));
#if GAMECOE_HAS_UBO
            std::uint32_t cameraUniformBlockIndex = glGetUniformBlockIndex(shader->id(), "CameraMatrices");
            glUniformBlockBinding(shader->id(), cameraUniformBlockIndex, constcoe::CAMERA_UBO_BINDING_POINT);
#endif
        }

        shader->use();
        
#if !GAMECOE_HAS_UBO
        auto &camera = owner().game().mainCamera();
        shader->set("view", camera.viewMatrix());
        shader->set("projection", camera.projectionMatrix());
        shader->set("cameraPosition", camera.owner().transform().position());
#endif
        auto model = owner().transform().modelMatrix();
        shader->set("model", model);
        shader->set("color", m_color.normalized());
        if(m_shape == Shape::Sphere)
            shader->set("inverseModel", glm::inverse(model)); // TODO: If GL_CULL_FACE is enabled, need to glDisable() it

        m_vertexArray.bind();

#if GAMECOE_USE_OPENGL
        if (m_vertexArray.hasIndices())
            glDrawElements(GL_TRIANGLES, m_vertexArray.indexCount(), GL_UNSIGNED_INT, (void*)0);
        else
            glDrawArrays(GL_TRIANGLES, 0, m_vertexArray.vertexCount());
#endif
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

    std::unique_ptr<ShapeRenderer> ShapeRenderer::circle(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Circle, color, layer));
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::sphere(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Sphere, color, layer));
    }
} // namespace gamecoe
