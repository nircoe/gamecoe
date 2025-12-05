#pragma once
#include <gamecoe/entity/component.hpp>
#include <cstdint>

namespace gamecoe
{
    class Renderer : public Component<Renderer>
    {
    protected:
        std::int8_t m_layer;

    public:
        static constexpr const char* TYPE_NAME = "Renderer";

        Renderer(GameObject &owner, std::int8_t layer = 0);
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;
        virtual ~Renderer();

        virtual void render() const = 0;
        // TODO: add render(Camera &camera) overload when Camera class will be functional

        void setLayer(std::int8_t layer = 0);
        std::int8_t layer() const;
    };
} // namespace gamecoe