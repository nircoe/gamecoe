#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <concepts>
#include <cstdint>
#include <gamecoe/entity/component.hpp>
// #include <gamecoe/entity/transform.hpp>
#include <logcoe.hpp>

namespace gamecoe
{
    class GameObject
    {
        static std::atomic<uint32_t> s_currentId;
        uint32_t m_id;
        // Transform m_transform; // after I'll implement this class
        std::unordered_map<std::string, std::unique_ptr<ComponentBase>> m_components;
        // std::unique_ptr<Renderer> m_renderer; // after I'll implement this class
        
        bool m_initialized;
        bool m_active;

        void setUpComponent(std::unique_ptr<ComponentBase> &component);

    public:
        GameObject() : m_id(++s_currentId),/* m_transform(this),*/ m_components(), m_initialized(false), m_active(false) { };
        ~GameObject() { m_components.clear(); m_initialized = m_active = false; };

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
        bool addComponent();

        template<std::derived_from<ComponentBase> T>
        void removeComponent();

        template<std::derived_from<ComponentBase> T>
        T &getComponent();
        
        template<std::derived_from<ComponentBase> T>
        const T &getComponent() const;

        template<std::derived_from<ComponentBase> T>
        bool hasComponent() const;

        // Transform &getTransform();
        // const Transform &getTransform() const;

        bool active() const;
    };

    template <std::derived_from<ComponentBase> T>
    inline bool GameObject::addComponent()
    {
        static const std::string componentType = T::staticType();
        if (m_components.contains(componentType)/* || componentType == Transform::staticType()*/)
        {
            logcoe::warning("The Game Object (id " + std::to_string(m_id) + ") already have a " + componentType + " component");
            return false;
        }
        // else if (std::is_base_of<Renderer, T>())
        // {
        //     if(m_renderer)
        //     {
        //         logcoe::warning("The Game Object (id " + std::to_string(m_id) + ") already have a renderer");
        //         return false;
        //     }

        //     m_renderer = std::make_unique<T>(this);
        //     setUpComponent(m_renderer);
        //     return true;
        // }

        m_components.emplace(componentType, std::make_unique<T>(this));
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

        if (!m_components.contains(componentType))
        {
            std::string message = "The Game Object (id " + std::to_string(m_id) + ") does not have a " + componentType + " component";
            logcoe::error(message);
            throw std::runtime_error(message);
        }
            
        return static_cast<T&>(*(m_components.at(componentType)));
    }

    template <std::derived_from<ComponentBase> T>
    inline const T &GameObject::getComponent() const
    {
        static const std::string componentType = T::staticType();

        if (!m_components.contains(componentType))
        {
            std::string message = "The Game Object (id " + std::to_string(m_id) + ") does not have a " + componentType + " component";
            logcoe::error(message);
            throw std::runtime_error(message);
        }
            
        return static_cast<const T&>(*(m_components.at(componentType)));
    }

    template <std::derived_from<ComponentBase> T>
    inline bool GameObject::hasComponent() const
    {
        static const std::string componentType = T::staticType();
        return m_components.contains(componentType);
    }

} // namespace gamecoe
