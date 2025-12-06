#include <gamecoe/core/game.hpp>
#include <gamecoe/core/window.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <logcoe.hpp>
#include <gamecoe_config.h>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>      
#endif

namespace gamecoe
{
    Game::Game() : Game("gamecoe", 800, 600) { }

    Game::Game(const std::string &title, uint32_t width, uint32_t height) :
                    m_mainWindow(width, height, title),
                    m_engineScene(*this, "EngineScene"), // maybe empty string name? so it will be unique
                    m_activeScenes(),
                    m_inactiveScenes()
    {
        logcoe::initialize(logcoe::LogLevel::INFO, title);
        // TODO: more initializations, datacoe, soundcoe, etc...

        auto &cameraGameObject = m_engineScene.createGameObject("MainCamera");
        cameraGameObject.addComponent<Camera>(std::make_unique<Camera>(cameraGameObject));
        m_mainCamera = &(cameraGameObject.getComponent<Camera>());
    }

    Game::~Game()
    {
        m_activeScenes.clear();
        m_inactiveScenes.clear();

        // TODO: more shutdowns, datacoe, soundcoe, etc...
        logcoe::shutdown();
    }

    Scene &Game::createScene(const std::string &name, std::int8_t layer)
    {
        if (m_inactiveScenes.contains(name) || m_activeScenes.contains(name))
            detail::throwError("Game::createScene: Scene with the name \"" + name + 
                               "\" already exist, scene name must be unique!"); // exist or exists? how I should write it?
        
        std::unique_ptr<Scene> scene = std::make_unique<Scene>(*this, name, layer);
        m_inactiveScenes.emplace(name, std::move(scene));

        return *m_inactiveScenes[name]; // if the user (AKA gamedev) will hold this reference, and we will later on move this unique_ptr<Scene> to the activeScenes map, will the reference the gamedev hold remain valid?
    }

    void Game::loadScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
        {
            logcoe::warning("Game::loadScene: The scene \"" + scene + "\" is already loaded and active");
            return;
        }

        if (!m_inactiveScenes.contains(scene))
            detail::throwError("Game::loadScene: The scene \"" + scene + "\" does not exist");
        
        m_inactiveScenes[scene]->load();
    }

    void Game::activateScene(const std::string &scene)
    {
        if (m_activeScenes.contains(scene))
        {
            logcoe::warning("Game::activateScene: The scene \"" + scene + "\" is already active");
            return;
        }

        if (!m_inactiveScenes.contains(scene))
            detail::throwError("Game::activateScene: The scene \"" + scene + "\" does not exist");
        
        m_inactiveScenes[scene]->activate();
        m_activeScenes.emplace(scene, std::move(m_inactiveScenes[scene]));
        m_inactiveScenes.erase(scene);
    }

    void Game::deactivateScene(const std::string &scene)
    {
    }

    void Game::unloadScene(const std::string &scene)
    {
    }

    std::map<std::int8_t, std::vector<std::reference_wrapper<Scene>>> Game::getActiveSceneLayers() const
    {
        std::map<std::int8_t, std::vector<std::reference_wrapper<Scene>>> activeSceneLayers;

        for (auto &[name, scene] : m_activeScenes)
        {
            std::int8_t layer = scene->layer();
            if (!activeSceneLayers.contains(layer))
                activeSceneLayers.emplace(layer, std::vector<Scene*>());
            
            activeSceneLayers[layer].push_back(std::ref(*scene));
        }

        return activeSceneLayers;
        // should I store this map as a class member like in Scene class (renderers map) or should I calculate it?
        // I think this method will be called only in the play() method for the order of iterating over the scenes in render() calls (maybe in update() calls as well?)
        // maybe we should make this method private? or maybe the users (AKA gamedevs) will have some other use for it?
    }

    bool Game::setSceneLayer(const std::string &scene, std::int8_t layer)
    {
        if (m_activeScenes.contains(scene))
            m_activeScenes[scene]->setLayer(layer);

        if (m_inactiveScenes.contains(scene))
            m_inactiveScenes[scene]->setLayer(layer);

        detail::throwError("Game::setSceneLayer: The Scene named \"" + scene + "\" does not exist");
    }

    Camera &Game::mainCamera()
    {
        if (!m_mainCamera)
        { // not suppose to get in here
            logcoe::error("Main Camera should have been created in the Game constructor and not been nullptr");
            assert(false && "Main Camera should have been created in the Game constructor and not been nullptr");
        }
        return *m_mainCamera;
    }

    const Camera &Game::mainCamera() const
    {
        if (!m_mainCamera)
        { // not suppose to get in here
            logcoe::error("Main Camera should have been created in the Game constructor and not been nullptr");
            assert(false && "Main Camera should have been created in the Game constructor and not been nullptr");
        }
        return *m_mainCamera;
    }

    Window &Game::mainWindow()
    {
        return m_mainWindow;
    }

    const Window &Game::mainWindow() const
    {
        return m_mainWindow;
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
        while (m_mainWindow.active())
        {
#if GAMECOE_USE_OPENGL
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // TODO: Introduce gamecoe::Color or something like that... and add Color class member and parameter for the constructor or setter...
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: check if we need those bits anytime, or let the gamedevs decide somehow
#endif
            auto scenesByLayers = getActiveSceneLayers(); // maybe rename this method? getActiveScenesByLayers?

            for (auto &[layer, scenes] : scenesByLayers)
            {
                for (auto &scene : scenes)
                    scene.get().update();
            }

            for (auto &[layer, scenes] : scenesByLayers)
            {
                for (auto &scene : scenes)
                    scene.get().render();
            }
            // should the for loops order and structure be any different?
        }
    }

} // namespace gamecoe
