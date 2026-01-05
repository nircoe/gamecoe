#include <gamecoe/core/scene.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

namespace gamecoe
{
    Scene::Scene(Game &game, const std::string &name, std::int8_t layer) :
                                                                m_name(name), 
                                                                m_layer(layer),
                                                                m_loaded(false),
                                                                m_active(false), 
                                                                m_activeGameObjects(), 
                                                                m_inactiveGameObjects(), 
                                                                m_renderersByLayer(),
                                                                m_game(game) { }

    Scene::~Scene()
    {
        if (m_active)
            deactivate();
        if (m_loaded)    
            unload();

        m_activeGameObjects.clear();
        m_inactiveGameObjects.clear();
        m_renderersByLayer.clear();
    }

    void Scene::load()
    {
        if (m_loaded)
            return logcoe::warning("Scene::load(): The scene \"" + m_name + "\" is already loaded");

        for (auto &[id, go] : m_activeGameObjects)
        {
            go->initialize();
        }

        for (auto &[id, go] : m_inactiveGameObjects)
        {
            go->initialize();
        }

        m_loaded = true;
    }

    void Scene::activate()
    {
        if (m_active)
            return logcoe::warning("Scene::activate(): The scene \"" + m_name + "\" is already active");

        if (!m_loaded)
            detail::throwError("Scene::activate(): The scene \"" + m_name + "\" is not loaded, please load it first");

        for (auto &[id, go] : m_activeGameObjects)
        {
            go->begin();
            go->activate();
        }

        for (auto &[id, go] : m_inactiveGameObjects)
        {
            go->begin();
        }

        m_active = true;
    }

    void Scene::deactivate()
    {
        if (!m_active)
            return logcoe::warning("Scene::deactivate(): The scene \"" + m_name + "\" is already inactive");

        for (auto &[id, go] : m_activeGameObjects)
        {
            go->deactivate();
        }

        m_active = false;
    }

    void Scene::unload()
    {
        if (!m_loaded)
            return logcoe::warning("Scene::unload(): The scene \"" + m_name + "\" is already unloaded");

        if (m_active)
            detail::throwError("Scene::unload(): The scene \"" + m_name + "\" is active, please deactivate it first");

        // TODO: Call resource cleanup on GameObjects when resource management is implemented

        m_loaded = false;
    }

    void Scene::update()
    {
        for (auto &[id, go] : m_activeGameObjects)
        {
            go->update(); // TODO: Introduce update layers or some ordering later on
        }
    }

    void Scene::render()
    {
        for (auto &[layer, objects] : m_renderersByLayer)
        {
            for (auto *go : objects)
            {
                if (go->active())
                    go->render();
            }
        }
    }

    GameObject &Scene::createGameObject(const std::string &name, bool active, std::optional<std::reference_wrapper<GameObject>> parent)
    {
        if (!name.empty() && findGameObject(name) != std::nullopt)
            detail::throwError("Scene::createGameObject(): There is already GameObject named \"" + name + 
                               "\", GameObject name should be unique!");

        std::unique_ptr<GameObject> go = std::make_unique<GameObject>(*this, name, parent);
        std::uint32_t id = go->id();
        active ? m_activeGameObjects.emplace(id, std::move(go)) :
                 m_inactiveGameObjects.emplace(id, std::move(go));
        
        return active ? *m_activeGameObjects[id] : *m_inactiveGameObjects[id];
    }

    void Scene::removeGameObject(std::uint32_t id)
    {
        // TODO: Handle parent-child hierarchy - consider either recursive child removal
        //       or automatic orphaning (set children's parent to std::nullopt) when parent is removed

        if (m_activeGameObjects.contains(id))
        {
            m_activeGameObjects[id]->deactivate();
            m_activeGameObjects.erase(id);
            return;
        }

        if (m_inactiveGameObjects.contains(id))
        {
            m_inactiveGameObjects.erase(id);
            return;
        }

        detail::throwError("Scene::removeGameObject(): The Game Object (id " + std::to_string(id) +
                           ") does not exist!");
    }

    void Scene::activateGameObject(std::uint32_t id)
    {
        if (m_inactiveGameObjects.contains(id))
        {
            m_inactiveGameObjects[id]->activate();
            m_activeGameObjects.emplace(id, std::move(m_inactiveGameObjects[id]));
            m_inactiveGameObjects.erase(id);
            return;
        }

        if (m_activeGameObjects.contains(id))
            return logcoe::warning("Scene::activateGameObject(): The Game Object " + m_activeGameObjects[id]->name() +
                          " (id " + std::to_string(id) + ") is already active");

        detail::throwError("Scene::activateGameObject(): The Game Object (id " + std::to_string(id) + 
                            ") does not exist!");
    }

    void Scene::deactivateGameObject(std::uint32_t id)
    {
        if (m_activeGameObjects.contains(id))
        {
            m_activeGameObjects[id]->deactivate();
            m_inactiveGameObjects.emplace(id, std::move(m_activeGameObjects[id]));
            m_activeGameObjects.erase(id);
            return;
        }

        if (m_inactiveGameObjects.contains(id))
            return logcoe::warning("Scene::deactivateGameObject(): The Game Object " + m_inactiveGameObjects[id]->name() +
                          " (id " + std::to_string(id) + ") is already inactive");

        detail::throwError("Scene::deactivateGameObject(): The Game Object (id " + std::to_string(id) + 
                           ") does not exist!");     
    }

    void Scene::addRenderer(std::int8_t layer, GameObject* go)
    {
        if (!go)
            detail::invalidArgument("Scene::addRenderer(): The GameObject* argument cannot be null");
        
        if (!m_renderersByLayer.contains(layer))
            m_renderersByLayer.emplace(layer, std::vector<GameObject*>());
        
        m_renderersByLayer[layer].push_back(go);
    }

    void Scene::removeRenderer(std::int8_t layer, std::uint32_t id)
    {
        if (!m_renderersByLayer.contains(layer))
            detail::throwError("Scene::removeRenderer(): The Rendering layer " + std::to_string(layer) + 
                               " is empty");

        auto &objects = m_renderersByLayer[layer];
        for (auto it = objects.begin(); it != objects.end(); ++it)
        {
            if ((*it)->id() == id)
            {
                objects.erase(it);
                if (objects.empty())
                    m_renderersByLayer.erase(layer);
                return;
            }
        }

        detail::throwError("Scene::removeRenderer(): The GameObject (id " + std::to_string(id) + 
                            ") is not part of Rendering layer " + std::to_string(layer));
    }

    void Scene::changeRendererLayer(std::int8_t oldLayer, std::int8_t newLayer, std::uint32_t id)
    {
        if (!m_renderersByLayer.contains(oldLayer))
            detail::throwError("Scene::changeRendererLayer(): The Rendering layer " + std::to_string(oldLayer) + 
                            " is empty, if you would like to add the GameObject Renderer to the layer " + 
                            std::to_string(newLayer) + ", please use Scene::addRenderer()");

        auto &objects = m_renderersByLayer[oldLayer];
        GameObject *go = nullptr;
        for (auto it = objects.begin(); it != objects.end(); ++it)
        {
            if ((*it)->id() == id)
            {
                go = *it;
                objects.erase(it);
                break;
            }
        }

        if(!go)
            detail::throwError("Scene::changeRendererLayer(): The GameObject (id " + std::to_string(id) + 
                            ") is not part of Rendering layer " + std::to_string(oldLayer) + 
                            ", if you would like to add the GameObject Renderer to the layer " + 
                            std::to_string(newLayer) + ", please use Scene::addRenderer()");

        if (!m_renderersByLayer.contains(newLayer))
            m_renderersByLayer.emplace(newLayer, std::vector<GameObject*>());
        
        m_renderersByLayer[newLayer].push_back(go);
    }

    const std::string &Scene::name() const
    {
        return m_name;
    }

    void Scene::setLayer(std::int8_t layer)
    {
        m_layer = layer;
    }

    std::int8_t Scene::layer() const
    {
        return m_layer;
    }

    bool Scene::loaded() const
    {
        return m_loaded;
    }

    bool Scene::active() const
    {
        return m_active;
    }

    Game &Scene::game()
    {
        return m_game;
    }

    const Game &Scene::game() const
    {
        return m_game;
    }

    std::optional<std::reference_wrapper<GameObject>> Scene::findGameObject(std::uint32_t id)
    {
        if (m_activeGameObjects.contains(id))
            return std::ref(*m_activeGameObjects[id]);

        if (m_inactiveGameObjects.contains(id))
            return std::ref(*m_inactiveGameObjects[id]);

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<GameObject>> Scene::findGameObject(const std::string &name)
    {
        for (auto &[id, go] : m_activeGameObjects)
        {
            if (go->name() == name)
                return std::ref(*go);
        }

        for (auto &[id, go] : m_inactiveGameObjects)
        {
            if (go->name() == name)
                return std::ref(*go);
        }

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const GameObject>> Scene::findGameObject(std::uint32_t id) const
    {
        if (m_activeGameObjects.contains(id))
            return std::cref(*m_activeGameObjects.at(id));

        if (m_inactiveGameObjects.contains(id))
            return std::cref(*m_inactiveGameObjects.at(id));

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const GameObject>> Scene::findGameObject(const std::string &name) const
    {
        for (auto &[id, go] : m_activeGameObjects)
        {
            if (go->name() == name)
                return std::cref(*go);
        }

        for (auto &[id, go] : m_inactiveGameObjects)
        {
            if (go->name() == name)
                return std::cref(*go);
        }

        return std::nullopt;
    }

} // namespace gamecoe
