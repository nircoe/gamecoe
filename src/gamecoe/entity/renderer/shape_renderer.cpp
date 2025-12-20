#include <gamecoe/entity/renderer/shape_renderer.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/camera.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/paths.hpp>
#include <cassert>
#include <optional>
#include <glad/gl.h>
#include <glm/glm.hpp>



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
        case gamecoe::Shape::Cube:
            return gamecoe::VertexArray::cube();
        
        default:
            assert(false && "Need to add support for a new Shape");
            break;
        }
    }
}

namespace gamecoe
{
    std::atomic<uint32_t> ShapeRenderer::s_counter = 0;
    std::optional<Shader> ShapeRenderer::s_shader = std::nullopt;

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
            s_shader.emplace(
                resolvePath("gamecoe/shaders/shape_renderer.vert"), 
                resolvePath("gamecoe/shaders/shape_renderer.frag")
            );

        auto &camera = owner().game().mainCamera();

        glm::mat4 model = owner().transform().modelMatrix();
        glm::mat4 view = camera.viewMatrix();
        glm::mat4 projection = camera.projectionMatrix();

        auto colorNormalized = m_color.normalized();
        glm::vec4 color = { colorNormalized[0], colorNormalized[1], colorNormalized[2], colorNormalized[3] };

        s_shader->use();
        s_shader->set("model", model);
        s_shader->set("view", view);
        s_shader->set("projection", projection);
        s_shader->set("color", color);

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

    std::unique_ptr<ShapeRenderer> ShapeRenderer::cube(GameObject &owner, const Color &color, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Cube, color, layer));
    }
} // namespace gamecoe
