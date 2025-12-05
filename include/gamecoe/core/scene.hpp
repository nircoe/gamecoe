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
#include <gamecoe/entity/camera.hpp>
#include <gamecoe/core/window.hpp>
// #include <gamecoe/core/game.hpp>

namespace gamecoe
{
    class Scene
    {
        static std::atomic<uint32_t> s_currentId;

        std::uint32_t m_id;
        std::string m_name;
        std::int8_t m_layer;
        bool m_active;
        
        std::unordered_map<std::uint32_t, std::unique_ptr<GameObject>> m_activeGameObjects;
        std::unordered_map<std::uint32_t, std::unique_ptr<GameObject>> m_inactiveGameObjects;

        std::map<std::int8_t, std::vector<GameObject*>> m_renderersByLayer;

        // Game &m_game; // will be added later

    public:
        Scene(const std::string &name, std::int8_t layer = 0);
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
        void changeRendererLayer(std::int8_t oldLayer, std::int8_t newLayer, std::uint32_t id);

        std::uint32_t id() const;
        const std::string &name() const;

        void setLayer(std::int8_t layer = 0);
        std::int8_t layer() const;

        bool active() const;

        // Camera &mainCamera(); // will be added later
        // Game &game(); // will be added later

        std::optional<std::reference_wrapper<GameObject>> findGameObject(std::uint32_t id);
        std::optional<std::reference_wrapper<GameObject>> findGameObject(const std::string &name);
        std::optional<std::reference_wrapper<const GameObject>> findGameObject(std::uint32_t id) const;
        std::optional<std::reference_wrapper<const GameObject>> findGameObject(const std::string &name) const;
    };
} // namespace gamecoe