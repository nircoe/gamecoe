#include <gamecoe/entity/renderer/renderer.hpp>

namespace gamecoe
{
    Renderer::Renderer(GameObject &owner, std::int8_t layer) : Component<Renderer>(owner), m_layer(layer) { }

    void Renderer::setLayer(std::int8_t layer) { m_layer = layer; }

    std::int8_t Renderer::layer() const { return m_layer; }
} // namespace gamecoe
