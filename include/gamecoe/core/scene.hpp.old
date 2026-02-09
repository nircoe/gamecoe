#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
#include <optional>
#include <functional>
#include <cstdint>
#include <gamecoe/entity/game_object.hpp>

namespace gamecoe
{
    class Game;
    
    class Scene
    {
        std::string m_name;
        std::int8_t m_layer;
        bool m_loaded;
        bool m_active;
        
        std::unordered_map<std::uint32_t, std::unique_ptr<GameObject>> m_activeGameObjects;
        std::unordered_map<std::uint32_t, std::unique_ptr<GameObject>> m_inactiveGameObjects;

        std::map<std::int8_t, std::vector<GameObject*>> m_renderersByLayer;

        Game &m_game;

    public:
        Scene(Game &game, const std::string &name, std::int8_t layer = 0);
        ~Scene();

        // need to be called after all GameObjects are part of the scene
        void load();
        void activate();
        void deactivate();
        void unload();

        void update();
        void render();

        GameObject &createGameObject(const std::string &name = "", bool active = true, std::optional<std::reference_wrapper<GameObject>> parent = std::nullopt);
        void removeGameObject(std::uint32_t id);

        void activateGameObject(std::uint32_t id);
        void deactivateGameObject(std::uint32_t id);

        void addRenderer(std::int8_t layer, GameObject* go);
        void removeRenderer(std::int8_t layer, std::uint32_t id);
        void changeRendererLayer(std::int8_t oldLayer, std::int8_t newLayer, std::uint32_t id);

        const std::string &name() const;

        void setLayer(std::int8_t layer = 0);
        std::int8_t layer() const;

        bool loaded() const;
        bool active() const;

        Game &game();
        const Game &game() const;

        std::optional<std::reference_wrapper<GameObject>> findGameObject(std::uint32_t id);
        std::optional<std::reference_wrapper<GameObject>> findGameObject(const std::string &name);
        std::optional<std::reference_wrapper<const GameObject>> findGameObject(std::uint32_t id) const;
        std::optional<std::reference_wrapper<const GameObject>> findGameObject(const std::string &name) const;
    };
} // namespace gamecoe