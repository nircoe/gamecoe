#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include <cstdint>
#include <gamecoe/core/window.hpp>
#include <gamecoe/core/scene.hpp>

namespace gamecoe
{
    class Game
    {
        std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
        std::unique_ptr<Window> m_mainWindow;
        // For games with multiply windows - optional
        std::vector<std::unique_ptr<Window>> m_additionalWindows;


    public:
        Game(); // maybe delete in the future
        Game(const std::string &title, uint32_t width, uint32_t height); // creates a single mainWindow with those arguments
        ~Game();

        std::vector<std::pair<std::string, uint8_t>> getActiveSceneLayers() const;
        bool setSceneLayer(const std::string &scene, uint8_t layer);

        bool addWindow(uint32_t width, uint32_t height); // use the main title? or different title for each window?
        // removeWindow() ?

        bool addScene(std::unique_ptr<Scene> scene); // this method should recieve unique_ptr?

        void gameLoop();
    };

} // namespace gamecoe