#pragma once
#include <gamecoe/entity/component.hpp>
#include <cstdint>

namespace gamecoe
{
    class Renderer : public Component<Renderer>
    {
    protected:
        std::int8_t m_layer;
        mutable bool m_visible;

        virtual bool visible() const = 0;
        virtual void renderImpl() const = 0;

    public:
        static constexpr const char* TYPE_NAME = "Renderer";

        Renderer(GameObject &owner, std::int8_t layer = 0);
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;
        virtual ~Renderer();

        void render() const;

        void setLayer(std::int8_t layer = 0);
        std::int8_t layer() const;
    };
} // namespace gamecoe