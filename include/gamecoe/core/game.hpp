#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <optional>
#include <functional>
#include <string>
#include <cstdint>
#include <gamecoe/core/window.hpp>
#include <gamecoe/core/scene.hpp>
#include <gamecoe/entity/camera.hpp>

namespace gamecoe
{    
    class Game
    {
        std::optional<Window> m_mainWindow;
        std::optional<Scene> m_internalScene;
        Camera *m_mainCamera;

        std::unordered_map<std::string, std::unique_ptr<Scene>> m_activeScenes;
        std::unordered_map<std::string, std::unique_ptr<Scene>> m_inactiveScenes;

        // TODO: For games with multiply windows - optional
        // std::vector<std::unique_ptr<Window>> m_additionalWindows;

    public:
        Game(); // Default title "gamecoe", screen size 800x600 pixels
        Game(const std::string &title, uint32_t width, uint32_t height);
        ~Game();

        /*
        TODO: relevant methods for future multi-window support
        bool addWindow(uint32_t width, uint32_t height); // use the main title? or different title for each window?
        removeWindow() ?
        */

        Scene &createScene(const std::string &name, std::int8_t layer = 0); // puts the new scene in the inactive map and expect the user (AKA gamedev) to activate the scene he wants?

        void loadScene(const std::string &scene);
        void activateScene(const std::string &scene);
        void deactivateScene(const std::string &scene);
        void unloadScene(const std::string &scene);

        std::map<std::int8_t, std::vector<std::reference_wrapper<Scene>>> getActiveSceneLayers() const;
        void setSceneLayer(const std::string &scene, std::int8_t layer);

        Camera &mainCamera();
        const Camera &mainCamera() const;

        Window &mainWindow();
        const Window &mainWindow() const;

        std::optional<std::reference_wrapper<Scene>> findScene(const std::string &scene);
        std::optional<std::reference_wrapper<const Scene>> findScene(const std::string &scene) const;

        void play(); // The game play loop
    };

} // namespace gamecoe