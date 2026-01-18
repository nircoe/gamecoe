#include <gamecoe/entity/renderer/renderer.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/scene.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <exception>

namespace gamecoe
{
    Renderer::Renderer(GameObject &owner, std::int8_t layer) : Component<Renderer>(owner), m_layer(layer), m_visible(true)
    { 
        m_owner.scene().addRenderer(m_layer, &m_owner);
    }

    Renderer::~Renderer()
    {
        try { m_owner.scene().removeRenderer(m_layer ,m_owner.id()); }
        catch (const std::exception &e) { detail::throwError("Renderer::~Renderer(): " + std::string(e.what())); }
    }

    void Renderer::render() const
    {
        if (visible()) renderImpl();
    }

    void Renderer::setLayer(std::int8_t layer) 
    {
        std::int8_t oldLayer = m_layer;
        m_layer = layer; 
        m_owner.scene().changeRendererLayer(oldLayer, m_layer, m_owner.id());
    }

    std::int8_t Renderer::layer() const { return m_layer; }
} // namespace gamecoe
