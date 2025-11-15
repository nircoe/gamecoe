#pragma once

#include <string>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    class GameObject;

    class ComponentBase 
    { 
    public:
        virtual ~ComponentBase() = default;

        virtual void initialize() = 0;
        virtual void begin() = 0;
        // Do not forget to set m_active = true when override
        virtual void activate() = 0;
        // Do not forget to set m_active = false when override
        virtual void deactivate() = 0;
        virtual void update() = 0;
    };

    template<typename Derived>
    class Component : public ComponentBase
    {
    protected:
        GameObject* m_owner;
        bool m_active;

    public:
        Component() = delete;
        Component(GameObject* owner) : m_owner(owner), m_active(false) 
        {
            if (!owner)
                detail::throwError("Component owner cannot be nullptr");
        };
        virtual ~Component() = default;

        static std::string staticType() { return Derived::TYPE_NAME; };
        // Returns the component type in string
        const std::string &type() const { return staticType(); };

        GameObject *owner() { return m_owner; };
        const GameObject *owner() const { return m_owner; };

        bool active() const { return m_active; };
    };
} // namespace gamecoe
