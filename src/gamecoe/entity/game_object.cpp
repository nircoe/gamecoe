#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/scene.hpp>

namespace gamecoe
{
    std::atomic<std::uint32_t> GameObject::s_currentId = 0;

    GameObject::GameObject(Scene &scene, const std::string &name, std::optional<std::reference_wrapper<GameObject>> parent) : 
        m_id(++s_currentId), 
        m_name(name), 
        m_parent(parent), 
        m_transform(*this), 
        m_components(), 
        m_initialized(false), 
        m_active(false),
        m_scene(scene)
    {
        if (m_name == "")
            m_name = "GameObject" + std::to_string(m_id);
    }

    void GameObject::initialize()
    {
        m_initialized = true;
        for (auto& [type, component] : m_components)
        {
            component->initialize();
        }
    }

    void GameObject::begin()
    {
        for (auto& [type, component] : m_components)
        {
            component->begin();
        }
    }
      
    void GameObject::activate()
    {
        m_active = true;
        for (auto& [type, component] : m_components)
        {
            component->activate();
        }
    }
        
    void GameObject::deactivate()
    {
        for (auto& [type, component] : m_components)
        {
            component->deactivate();
        }

        m_active = false;
    }
  
    void GameObject::update()
    {
        for (auto& [type, component] : m_components)
        {
            component->update();
        }

        // m_renderer->update(); is it needed?
    }
        
    void GameObject::render()
    {
        if (m_renderer)
            m_renderer->render();
    }

    std::uint32_t GameObject::id() const
    {
        return m_id;
    }

    void GameObject::setName(const std::string &name)
    {
        m_name = name;
    }

    const std::string &GameObject::name() const
    {
        return m_name;
    }

    Transform &GameObject::transform()
    {
        return m_transform;
    }
    
    const Transform &GameObject::transform() const
    {
        return m_transform;
    }

    void GameObject::setParent(std::optional<std::reference_wrapper<GameObject>> parent)
    {
        m_parent = parent;
    }

    std::optional<std::reference_wrapper<GameObject>> GameObject::parent()
    {
        return m_parent;
    }

    std::optional<std::reference_wrapper<const GameObject>> GameObject::parent() const
    {
        return m_parent ?
                std::optional(std::cref(m_parent->get())) : 
                std::nullopt;
    }

    bool GameObject::hasParent() const
    {
        return m_parent.has_value();
    }

    bool GameObject::active() const
    {
        return m_active;
    }
    
    Scene &GameObject::scene()
    {
        return m_scene;
    }

    const Scene &GameObject::scene() const
    {
        return m_scene;
    }
} // namespace gamecoe
