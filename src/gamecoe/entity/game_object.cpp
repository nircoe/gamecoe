#include <gamecoe/entity/game_object.hpp>

namespace gamecoe
{
    std::atomic<std::uint32_t> GameObject::s_currentId = 0;

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

    Transform &GameObject::transform()
    {
        return m_transform;
    }
    
    const Transform &GameObject::transform() const
    {
        return m_transform;
    }

    GameObject *GameObject::parent()
    {
        return m_parent;
    }

    const GameObject *GameObject::parent() const
    {
        return m_parent;
    }

    bool GameObject::active() const
    {
        return m_active;
    }    
} // namespace gamecoe
