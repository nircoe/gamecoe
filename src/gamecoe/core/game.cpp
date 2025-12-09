#include <gamecoe/core/game.hpp>
#include <gamecoe/core/window.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <logcoe.hpp>
#include <cassert>
#include <gamecoe_config.h>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

#include <GLFW/glfw3.h>

namespace gamecoe
{
    Game::Game() : Game("gamecoe", 800, 600) { }

    Game::Game(const std::string &title, uint32_t width, uint32_t height) :
                    m_mainWindow(std::nullopt),
                    m_internalScene(std::nullopt),
                    m_activeScenes(),
                    m_inactiveScenes()
    {
        struct GarbageCollector
        {
            bool m_succeed = false;
            ~GarbageCollector()
            {
                if(m_succeed) return;
                
                glfwTerminate();
                logcoe::shutdown();
            }
        } gc;

        logcoe::initialize(logcoe::LogLevel::INFO, title);
        // TODO: more initializations, datacoe, soundcoe, etc...

        if(!glfwInit())
            detail::throwError("Game::Game: Failed to initialize glfw");
        
        m_mainWindow.emplace(width, height, title);
        m_internalScene.emplace(*this, "gamecoe::Internal");

        auto &cameraGameObject = m_internalScene->createGameObject("MainCamera");
        cameraGameObject.addComponent<Camera>(std::make_unique<Camera>(cameraGameObject));
        m_mainCamera = &(cameraGameObject.getComponent<Camera>());

        gc.m_succeed = true;
    }

    Game::~Game()
    {
        m_activeScenes.clear();
        m_inactiveScenes.clear();

        // TODO: more shutdowns, datacoe, soundcoe, etc...
        glfwTerminate();
        logcoe::shutdown();
    }

    Scene &Game::createScene(const std::string &name, std::int8_t layer)
    {
        if (m_inactiveScenes.contains(name) || m_activeScenes.contains(name))
            detail::throwError("Game::createScene: Scene with the name \"" + name + 
                               "\" already exists, scene name must be unique!");
        
        std::unique_ptr<Scene> scene = std::make_unique<Scene>(*this, name, layer);
        m_inactiveScenes.emplace(name, std::move(scene));

        return *m_inactiveScenes[name];
    }

    void Game::loadScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
            return logcoe::warning("Game::loadScene: The scene \"" + scene + "\" is already loaded and active");

        if (!m_inactiveScenes.contains(scene))
            detail::throwError("Game::loadScene: The scene \"" + scene + "\" does not exist");
        
        if (m_inactiveScenes[scene]->loaded())
            return logcoe::warning("Game::loadScene: The scene \"" + scene + "\" is already loaded");

        m_inactiveScenes[scene]->load();
    }

    void Game::activateScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
            return logcoe::warning("Game::activateScene: The scene \"" + scene + "\" is already active");

        if (!m_inactiveScenes.contains(scene))
            detail::throwError("Game::activateScene: The scene \"" + scene + "\" does not exist");
        
        if (!m_inactiveScenes[scene]->loaded())
            detail::throwError("Game::activateScene: The scene \"" + scene + "\" is not loaded, please load it first");

        m_inactiveScenes[scene]->activate();
        m_activeScenes.emplace(scene, std::move(m_inactiveScenes[scene]));
        m_inactiveScenes.erase(scene);
    }

    void Game::deactivateScene(const std::string &scene)
    {
        if (m_inactiveScenes.contains(scene))
            return logcoe::warning("Game::deactivateScene: The scene \"" + scene + "\" is already inactive");

        if (!m_activeScenes.contains(scene))
            detail::throwError("Game::deactivateScene: The Scene \"" + scene + "\" does not exist");

        m_activeScenes[scene]->deactivate();
        m_inactiveScenes.emplace(scene, std::move(m_activeScenes[scene]));
        m_activeScenes.erase(scene);
    }

    void Game::unloadScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
            detail::throwError("Game::unloadScene: The scene \"" + scene + "\" is active, please deactivate it first");

        if (!m_inactiveScenes.contains(scene))
            detail::throwError("Game::unloadScene: The scene \"" + scene + "\" does not exist");

        if (!m_inactiveScenes[scene]->loaded())
            return logcoe::warning("Game::unloadScene: The scene \"" + scene + "\" is already unloaded");
        
        m_inactiveScenes[scene]->unload();
    }

    std::map<std::int8_t, std::vector<std::reference_wrapper<Scene>>> Game::getActiveSceneLayers() const
    {
        std::map<std::int8_t, std::vector<std::reference_wrapper<Scene>>> activeSceneLayers;

        for (auto &[name, scene] : m_activeScenes)
        {
            std::int8_t layer = scene->layer();
            if (!activeSceneLayers.contains(layer))
                activeSceneLayers.emplace(layer, std::vector<std::reference_wrapper<Scene>>());
            
            activeSceneLayers[layer].push_back(std::ref(*scene));
        }

        return activeSceneLayers;
    }

    void Game::setSceneLayer(const std::string &scene, std::int8_t layer)
    {
        if (m_activeScenes.contains(scene))
            return m_activeScenes[scene]->setLayer(layer);

        if (m_inactiveScenes.contains(scene))
            return m_inactiveScenes[scene]->setLayer(layer);

        detail::throwError("Game::setSceneLayer: The Scene named \"" + scene + "\" does not exist");
    }

    Camera &Game::mainCamera()
    {
        assert(m_mainCamera && "Main Camera should have been created in the Game constructor and not been nullptr");

        return *m_mainCamera;
    }

    const Camera &Game::mainCamera() const
    {
        assert(m_mainCamera && "Main Camera should have been created in the Game constructor and not been nullptr");

        return *m_mainCamera;
    }

    Window &Game::mainWindow()
    {
        return *m_mainWindow;
    }

    const Window &Game::mainWindow() const
    {
        return *m_mainWindow;
    }

    std::optional<std::reference_wrapper<Scene>> Game::findScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
            return std::ref(*m_activeScenes[scene]);

        if (m_inactiveScenes.contains(scene))
            return std::ref(*m_inactiveScenes[scene]);

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const Scene>> Game::findScene(const std::string &scene) const
    {
        if (m_activeScenes.contains(scene))
            return std::cref(*m_activeScenes.at(scene));

        if (m_inactiveScenes.contains(scene))
            return std::cref(*m_inactiveScenes.at(scene));

        return std::nullopt;
    }

    void Game::play()
    {
        while (m_mainWindow->active())
        {
            // TODO: processInput(); - Handle keyboard/mouse input at first

#if GAMECOE_USE_OPENGL
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // TODO: Introduce gamecoe::Color or something like that... and add Color class member and parameter for the constructor or setter...
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: check if we need those bits anytime, or let the gamedevs decide somehow
#endif
            auto scenesByLayers = getActiveSceneLayers();

            for (auto &[layer, scenes] : scenesByLayers)
            {
                for (auto &scene : scenes)
                    scene.get().update();
            }

            // TODO: Physics/Collision system

            for (auto &[layer, scenes] : scenesByLayers)
            {
                for (auto &scene : scenes)
                    scene.get().render();
            }

            // TODO: soundcoe::update();
        }
    }

} // namespace gamecoe
