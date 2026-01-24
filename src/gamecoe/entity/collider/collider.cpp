#include <gamecoe/entity/collider/collider.hpp>
#include <gamecoe/entity/game_object.hpp>
#include <gamecoe/core/game.hpp>
#include <gamecoe/core/scene.hpp>
#include <gamecoe/utils/collision.hpp>
#include <gamecoe/utils/shape.hpp>

namespace gamecoe
{
    Collider::Collider(GameObject &owner,
        const std::function<void(const Collider&)> &onCollisionBegin, 
        const std::function<void(const Collider&)> &onCollision, 
        const std::function<void(const Collider&)> &onCollisionEnd,
        std::int8_t layer) : Component<Collider>(owner),
        m_layer(layer),
        m_collidedWith(),
        m_onCollisionBegin(onCollisionBegin), 
        m_onCollision(onCollision), 
        m_onCollisionEnd(onCollisionEnd)
    { 
        
    }

    Collider::~Collider()
    {
        m_onCollisionBegin = nullptr;
        m_onCollision = nullptr;
        m_onCollisionEnd = nullptr;
    }

    void Collider::initialize()
    {

    }

    void Collider::begin()
    {

    }

    void Collider::activate() 
    {
        m_owner.game().addCollider(*this, m_layer);
    }

    void Collider::deactivate() 
    {
        auto id = m_owner.id();
        for (auto otherId : m_collidedWith)
        {
            auto go = m_owner.scene().findGameObject(otherId);

            if (go && go->get().hasComponent<Collider>())
            {
                auto &col = go->get().getComponent<Collider>();
                if (m_onCollisionEnd) m_onCollisionEnd(col);
                if (col.m_onCollisionEnd) col.m_onCollisionEnd(*this);
                col.m_collidedWith.erase(m_owner.id());
            }
        }
        m_collidedWith.clear();
        m_owner.game().removeCollider(*this, m_layer);
    }

    void Collider::setOnCollisionBegin(const std::function<void(const Collider&)> &onCollisionBegin)
    {
        m_onCollisionBegin = onCollisionBegin;
    }

    void Collider::setOnCollision(const std::function<void(const Collider&)> &onCollision)
    {
        m_onCollision = onCollision;
    }

    void Collider::setOnCollisionEnd(const std::function<void(const Collider&)> &onCollisionEnd)
    {
        m_onCollisionEnd = onCollisionEnd;
    }

    void Collider::setLayer(std::int8_t layer)
    {
        m_owner.game().updateColliderLayer(*this, m_layer, layer);
        m_layer = layer;
    }

    std::int8_t Collider::layer() const
    {
        return m_layer;
    }

    Shape Collider::shape() const 
    {
        return Shape::Invalid;
    }

    void Collider::collide(const Collider &other) const
    {
        if (!m_active || !other.active()) return;

        // ShapeCollider/ShapeCollider collision detection
        if (shape() != Shape::Invalid && other.shape() != Shape::Invalid)
        {
            bool collided = collision::detect(m_owner.transform(), shape(), other.owner().transform(), other.shape());

            std::uint32_t id = m_owner.id();
            std::uint32_t otherId = other.owner().id();

            if (auto it = m_collidedWith.find(otherId); it != m_collidedWith.end())
            {
                if (collided)
                {
                    if (m_onCollision) m_onCollision(other);
                    if (other.m_onCollision) other.m_onCollision(*this);
                    return;
                } 

                if (m_onCollisionEnd) m_onCollisionEnd(other);
                if (other.m_onCollisionEnd) other.m_onCollisionEnd(*this);
                m_collidedWith.erase(it);
                other.m_collidedWith.erase(id);
                return;
            }

            if (!collided) return;

            m_collidedWith.insert(otherId);
            other.m_collidedWith.insert(id);
            if (m_onCollisionBegin) m_onCollisionBegin(other);
            if (other.m_onCollisionBegin) other.m_onCollisionBegin(*this);
        }
        // TODO: treat other Colliders
    }
} // namespace gamecoe
