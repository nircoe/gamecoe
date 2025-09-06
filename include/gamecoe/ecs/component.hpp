#pragma once

namespace gamecoe
{
    class Component
    {
        // component members
    public:
        virtual ~Component() {};
        virtual void update() = 0;

        // more methods?
    };
} // namespace gamecoe