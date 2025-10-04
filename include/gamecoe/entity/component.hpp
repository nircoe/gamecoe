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
        GameObject* m_parent;
        bool m_active;

    public:
        Component() = delete;
        Component(GameObject* parent) : m_parent(parent), m_active(false) { };
        virtual ~Component() = default;

        static std::string staticType() { return Derived::TYPE_NAME; };
        // Returns the component type in string
        const std::string &type() const { return staticType(); };

        bool active() const { return m_active; };
    };
} // namespace gamecoe
