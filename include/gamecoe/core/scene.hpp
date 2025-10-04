#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <memory>
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

        uint32_t m_id;
        std::string m_name;
        // Game *m_game;
        Window *m_window; // TODO: remove when Game class introduced
        std::unordered_map<uint32_t, std::unique_ptr<GameObject>> m_activeGameObjects;
        std::unordered_map<uint32_t, std::unique_ptr<GameObject>> m_inactiveGameObjects;
        std::map<uint8_t, std::vector<GameObject*>> m_renderersByLayer;
        GameObject m_mainCamera;
        bool m_active;

    public:
        Scene();
        ~Scene();

        void initialize();
        void begin();
        void activate();
        void deactivate();
        void update();
        void render();

        GameObject* createGameObject(const std::string &name);
        void addGameObject(std::unique_ptr<GameObject> obj);

        void removeGameObject(uint32_t id);

        const std::string &name() const;

        bool active() const;

        Camera &mainCamera();

        GameObject &getGameObject(uint32_t id);
        GameObject &getGameObject(const std::string &name);
        const GameObject &getGameObject(uint32_t id) const;
        const GameObject &getGameObject(const std::string &name) const;
    };
} // namespace gamecoe