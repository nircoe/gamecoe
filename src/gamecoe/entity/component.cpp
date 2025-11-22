#include <gamecoe/entity/component.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/scene.hpp> // for future usage

namespace gamecoe
{

    template<typename Derived>
    GameObject &Component<Derived>::owner()
    {
        return m_owner;
    }

    template<typename Derived>
    const GameObject &Component<Derived>::owner() const
    {
        return m_owner;
    }

    template<typename Derived>
    Scene &Component<Derived>::scene()
    {
        return m_owner.scene();
    }

    template<typename Derived>
    const Scene &Component<Derived>::scene() const
    {
        return m_owner.scene();
    }
} // namespace gamecoe
