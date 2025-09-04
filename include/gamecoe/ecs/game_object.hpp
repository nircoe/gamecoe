#pragma once

#include <string>
#include <unordered_map>
#include "component.hpp"

namespace gamecoe
{
    class GameObject
    {
        size_t m_id;
        std::unordered_map<std::string, Component*> m_components;

    public:
        GameObject();
        virtual ~GameObject();

        virtual void update();
        virtual void render();
    };
} // namespace gamecoe