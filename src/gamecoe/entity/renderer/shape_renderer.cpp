#include <gamecoe/entity/renderer/shape_renderer.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/camera.hpp>
#include <gamecoe/core/game.hpp>
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

    ShapeRenderer::ShapeRenderer(GameObject &owner, Shape shape, std::int8_t layer) : 
        Renderer(owner, layer), 
        m_shape(shape), 
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
        
        if (!s_shader.has_value())
            s_shader.emplace("assets/shaders/shape_renderer.vert", "assets/shaders/shape_renderer.frag");

        auto &camera = owner().game().mainCamera();

        glm::mat4 model = owner().transform().modelMatrix();
        glm::mat4 view = camera.viewMatrix();
        glm::mat4 projection = camera.projectionMatrix();

        s_shader->use();
        s_shader->set("model", model);
        s_shader->set("view", view);
        s_shader->set("projection", projection);

        m_vertexArray.bind();

        if (m_vertexArray.hasIndices())
            glDrawElements(GL_TRIANGLES, m_vertexArray.indexCount(), GL_UNSIGNED_INT, (void*)0);
        else
            glDrawArrays(GL_TRIANGLES, 0, m_vertexArray.vertexCount());
    }

    Shape ShapeRenderer::shape() const { return m_shape; }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::triangle(GameObject &owner, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Triangle, layer));
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::rectangle(GameObject &owner, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Rectangle, layer));
    }

    std::unique_ptr<ShapeRenderer> ShapeRenderer::cube(GameObject &owner, std::int8_t layer)
    {
        return std::unique_ptr<ShapeRenderer>(new ShapeRenderer(owner, Shape::Cube, layer));
    }
} // namespace gamecoe
