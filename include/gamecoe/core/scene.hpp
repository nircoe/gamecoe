#pragma once

#include <unordered_map>
#include <gamecoe/ecs/game_object.hpp>

namespace gamecoe
{
    class Scene
    {
        size_t m_id;
        std::unordered_map<size_t, GameObject*> m_gameObjects;

    public:
        Scene();
        virtual ~Scene();

        virtual void update();
        virtual void render();
    };
} // namespace gamecoe