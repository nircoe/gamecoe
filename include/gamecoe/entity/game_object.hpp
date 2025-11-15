#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <concepts>
#include <cstdint>
#include <gamecoe/entity/component.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/renderer/renderer.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <logcoe.hpp>

namespace gamecoe
{
    class GameObject
    {
        static std::atomic<std::uint32_t> s_currentId;
        
        std::uint32_t m_id;
        std::string m_name;

        GameObject *m_parent;
        Transform m_transform;
        std::unordered_map<std::string, std::unique_ptr<ComponentBase>> m_components;
        std::unique_ptr<Renderer> m_renderer;
        
        bool m_initialized;
        bool m_active;

        template<std::derived_from<ComponentBase> T>
        void setUpComponent(std::unique_ptr<T> &component);

    public:
        GameObject(const std::string &name = "", GameObject *parent = nullptr);
        ~GameObject() { m_components.clear(); m_renderer.reset(); m_initialized = m_active = false; };

        // Called only once, when initializing the GameObject
        void initialize();
        // Called once every beginning of a Scene that the GameObject is a part of
        void begin();
        // Called every time the GameObject is been activated
        void activate();
        // Called every time the GameObject is been deactivated
        void deactivate();
        // Called once per frame while the GameObject is active
        void update();
        // Called once per frame while the GameObject is active and owns a Renderer
        void render();

        template<std::derived_from<ComponentBase> T>
        bool addComponent(std::unique_ptr<T> component);

        template<std::derived_from<ComponentBase> T>
        void removeComponent();

        template<std::derived_from<ComponentBase> T>
        T &getComponent();
        
        template<std::derived_from<ComponentBase> T>
        const T &getComponent() const;

        template<std::derived_from<ComponentBase> T>
        bool hasComponent() const;

        void setName(const std::string &name);
        const std::string &name() const;

        Transform &transform();
        const Transform &transform() const;

        void setParent(GameObject *parent);
        GameObject *parent();
        const GameObject *parent() const;

        bool active() const;
    };

    template<std::derived_from<ComponentBase> T>
    void GameObject::setUpComponent(std::unique_ptr<T> &component)
    {
        if (!m_initialized) return;
        component->initialize();

        if (!m_active) return;
        component->activate();
    }

    template <std::derived_from<ComponentBase> T>
    inline bool GameObject::addComponent(std::unique_ptr<T> component)
    {
        static const std::string componentType = T::staticType();
        if (m_components.contains(componentType) || componentType == Transform::staticType())
        {
            logcoe::warning("The Game Object (id " + std::to_string(m_id) + ") already have a " + componentType + " component");
            return false;
        }
        else if (std::is_base_of<Renderer, T>())
        {
            if(m_renderer)
            {
                logcoe::warning("The Game Object (id " + std::to_string(m_id) + ") already have a renderer");
                return false;
            }

            m_renderer = std::move(component);
            setUpComponent(m_renderer);
            return true;
        }

        m_components.emplace(componentType, std::move(component));
        setUpComponent(m_components[componentType]);

        return true;
    }
    
    template <std::derived_from<ComponentBase> T>
    inline void GameObject::removeComponent()
    {
        static const std::string componentType = T::staticType();
        m_components.erase(componentType);
    }

    template <std::derived_from<ComponentBase> T>
    inline T &GameObject::getComponent()
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
            return m_transform;

        if (std::is_base_of<Renderer, T>())
        {
            if (!m_renderer)
                detail::throwError("The Game Object (id " + std::to_string(m_id) + ") does not have a " + 
                                    componentType + " component");

            return static_cast<T&>(*m_renderer);
        }
            

        if (!m_components.contains(componentType))
            detail::throwError("The Game Object (id " + std::to_string(m_id) + ") does not have a " + 
                                componentType + " component");
            
        return static_cast<T&>(*(m_components.at(componentType)));
    }

    template <std::derived_from<ComponentBase> T>
    inline const T &GameObject::getComponent() const
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
            return m_transform;

        if (std::is_base_of<Renderer, T>())
        {
            if (!m_renderer)
                detail::throwError("The Game Object (id " + std::to_string(m_id) + ") does not have a " + 
                                    componentType + " component");

            return static_cast<const T&>(*m_renderer);
        }

        if (!m_components.contains(componentType))
            detail::throwError("The Game Object (id " + std::to_string(m_id) + ") does not have a " + 
                                componentType + " component");
            
        return static_cast<const T&>(*(m_components.at(componentType)));
    }

    template <std::derived_from<ComponentBase> T>
    inline bool GameObject::hasComponent() const
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
            return true;

        if (std::is_base_of<Renderer, T>())
            return m_renderer != nullptr;

        return m_components.contains(componentType);
    }

} // namespace gamecoe
