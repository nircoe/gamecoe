#pragma once

#include <string>

namespace gamecoe
{
    class GameObject;

    class ComponentBase 
    { 
    public:
        virtual ~ComponentBase() = default;

        virtual void initialize() = 0;
        virtual void begin() = 0;
        virtual void activate() = 0;
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
        Component(GameObject* owner) : m_owner(owner), m_active(false) { };
        virtual ~Component() = default;

        static std::string staticType() { return Derived::TYPE_NAME; };
        // Returns the component type in string
        const std::string &type() const { return staticType(); };

        GameObject *owner() { return m_owner; };
        const GameObject *owner() const { return m_owner; };

        bool active() const { return m_active; };
    };
} // namespace gamecoe
