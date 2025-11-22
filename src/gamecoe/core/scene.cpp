#include <gamecoe/core/scene.hpp>

namespace gamecoe
{
    std::atomic<uint32_t> Scene::s_currentId = 0;

    Scene::Scene(const std::string &name)
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::load()
    {
    }

    void Scene::activate()
    {
    }

    void Scene::deactivate()
    {
    }

    void Scene::unload()
    {
    }

    void Scene::update()
    {
    }

    void Scene::render()
    {
    }

    GameObject &Scene::createGameObject(const std::string &name)
    {
    }

    void Scene::removeGameObject(std::uint32_t id)
    {
    }

    void Scene::activateGameObject(std::uint32_t id)
    {
    }

    void Scene::deactivateGameObject(std::uint32_t id)
    {
    }

    std::uint32_t Scene::id() const
    {
    }

    const std::string &Scene::name() const
    {
    }

    void Scene::setLayer(std::int8_t layer)
    {
    }

    std::int8_t Scene::layer() const
    {
    }

    bool Scene::active() const
    {
    }

    std::optional<std::reference_wrapper<GameObject>> Scene::findGameObject(std::uint32_t id)
    {
    }

    std::optional<std::reference_wrapper<GameObject>> Scene::findGameObject(const std::string &name)
    {
    }

    std::optional<std::reference_wrapper<const GameObject>> Scene::findGameObject(std::uint32_t id) const
    {
    }

    std::optional<std::reference_wrapper<const GameObject>> Scene::findGameObject(const std::string &name) const
    {
    }

} // namespace gamecoe
