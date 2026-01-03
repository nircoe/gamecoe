#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <concepts>
#include <optional>
#include <functional>
#include <cstdint>
#include <gamecoe/entity/component.hpp>
#include <gamecoe/entity/transform.hpp>
#include <gamecoe/entity/renderer/renderer.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.h>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

namespace gamecoe
{
    class Scene;
    class Game;

    class GameObject
    {
        static std::atomic<std::uint32_t> s_currentId;
        
        std::uint32_t m_id;
        std::string m_name;

        std::optional<std::reference_wrapper<GameObject>> m_parent;
        Transform m_transform;
        std::unordered_map<std::string, std::unique_ptr<ComponentBase>> m_components;
        std::unique_ptr<Renderer> m_renderer;
        
        bool m_initialized;
        bool m_active;

        Scene &m_scene;

        template<std::derived_from<ComponentBase> T>
        void setUpComponent(std::unique_ptr<T> &component);

    public:
        GameObject(Scene &scene, const std::string &name = "", std::optional<std::reference_wrapper<GameObject>> parent = std::nullopt);
        ~GameObject() { m_components.clear(); m_renderer.reset(); m_initialized = m_active = false; };

        // Called only once, when initializing the GameObject
        void initialize();
        // Called once every beginning of a Scene containing the GameObject
        void begin();
        // Called every time the GameObject is activated
        void activate();
        // Called every time the GameObject is deactivated
        void deactivate();
        // Called once per frame while the GameObject is active
        void update();
        // Called once per frame while the GameObject is active and owns a Renderer
        void render();

        template<std::derived_from<ComponentBase> T>
        void addComponent(std::unique_ptr<T> component);

        template<std::derived_from<ComponentBase> T>
        void removeComponent();

        template<std::derived_from<ComponentBase> T>
        T &getComponent();
        
        template<std::derived_from<ComponentBase> T>
        const T &getComponent() const;

        template<std::derived_from<ComponentBase> T>
        bool hasComponent() const;

        void setRenderer(std::unique_ptr<Renderer> renderer, bool replace = false);
        void removeRenderer();
        bool hasRenderer() const;

        std::uint32_t id() const;

        void setName(const std::string &name);
        const std::string &name() const;

        Transform &transform();
        const Transform &transform() const;

        Renderer &renderer();
        const Renderer &renderer() const;

        void setParent(std::optional<std::reference_wrapper<GameObject>> parent);
        std::optional<std::reference_wrapper<GameObject>> parent();
        std::optional<std::reference_wrapper<const GameObject>> parent() const;
        bool hasParent() const;

        bool active() const;

        Scene &scene();
        const Scene &scene() const;

        Game &game();
        const Game &game() const;
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
    inline void GameObject::addComponent(std::unique_ptr<T> component)
    {
        static const std::string componentType = T::staticType();
        if (m_components.contains(componentType))
            detail::throwError("GameObject::addComponent(): The Game Object \"" + m_name + "\" already have a " + 
                               componentType + " component");
        
        if (componentType == Transform::staticType())
            detail::invalidArgument("GameObject::addComponent(): GameObject have built-in Transform Component");
        
        if constexpr (std::is_base_of_v<Renderer, T>)
        {
#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::addComponent(): Adding Renderer via setRenderer() (prefer setRenderer() for clarity)");
#endif
            return setRenderer(std::move(component));
        }

        m_components.emplace(componentType, std::move(component));
        setUpComponent(m_components[componentType]);
    }
    
    template <std::derived_from<ComponentBase> T>
    inline void GameObject::removeComponent()
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
            detail::invalidArgument("GameObject::removeComponent(): You cannot remove Transform from any GameObject");
        
        if constexpr (std::is_base_of_v<Renderer, T>)
        {
#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::removeComponent(): Removing Renderer via removeRenderer() (prefer removeRenderer() for clarity)");
#endif
            return removeRenderer();
        }

        if (!m_components.contains(componentType))
            detail::throwError("GameObject::removeComponent(): The Game Object \"" + m_name + 
                               "\" does not have an " + componentType + " component");

        m_components.erase(componentType);
    }

    template <std::derived_from<ComponentBase> T>
    inline T &GameObject::getComponent()
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
        {
#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::getComponent(): Returning Transform (prefer transform() for direct access)");
#endif
            return m_transform;
        }

        if (std::is_base_of_v<Renderer, T>)
        {
            if (!m_renderer)
                detail::throwError("GameObject::getComponent(): The Game Object \"" + m_name + "\" does not have a Renderer");

#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::getComponent(): Returning Renderer (prefer renderer() for direct access)");
#endif
            return static_cast<T&>(*m_renderer);
        }

        auto it = m_components.find(componentType);
        if (it == m_components.end())
            detail::throwError("GameObject::getComponent(): The Game Object \"" + m_name + "\" does not have a " + 
                                componentType + " component");
            
        return static_cast<T&>(*(it->second));
    }

    template <std::derived_from<ComponentBase> T>
    inline const T &GameObject::getComponent() const
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
        {
#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::getComponent(): Returning Transform (prefer transform() for direct access)");
#endif
            return m_transform;
        }

        if (std::is_base_of_v<Renderer, T>)
        {
            if (!m_renderer)
                detail::throwError("GameObject::getComponent(): The Game Object \"" + m_name + "\" does not have a Renderer");

#if GAMECOE_USE_LOGCOE
            logcoe::debug("GameObject::getComponent(): Returning Renderer (prefer renderer() for direct access)");
#endif
            return static_cast<const T&>(*m_renderer);
        }

        auto it = m_components.find(componentType);
        if (it == m_components.end())
            detail::throwError("GameObject::getComponent(): The Game Object \"" + m_name + "\" does not have a " + 
                                componentType + " component");
            
        return static_cast<const T&>(*(it->second));
    }

    template <std::derived_from<ComponentBase> T>
    inline bool GameObject::hasComponent() const
    {
        static const std::string componentType = T::staticType();

        if (componentType == Transform::staticType())
            return true;

        if (std::is_base_of_v<Renderer, T>)
            return m_renderer != nullptr;

        return m_components.contains(componentType);
    }

} // namespace gamecoe
