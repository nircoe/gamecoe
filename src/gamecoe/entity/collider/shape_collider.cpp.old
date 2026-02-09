#include <gamecoe/entity/collider/shape_collider.hpp>
#include <gamecoe/entity/game_object.hpp>

namespace gamecoe
{
    ShapeCollider::ShapeCollider(GameObject &owner, Shape shape,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer) : 
        Collider(owner, onCollisionBegin, onCollision, onCollisionEnd, layer), m_shape(shape)
    { 
        
    }

    ShapeCollider::~ShapeCollider()
    {

    }

    void ShapeCollider::initialize()
    {

    }

    void ShapeCollider::begin()
    {

    }

    void ShapeCollider::activate() 
    {
        Collider::activate();
        m_active = true;
    }

    void ShapeCollider::deactivate() 
    {
        Collider::deactivate();
        m_active = false;
    }

    void ShapeCollider::update()
    {

    }

    Shape ShapeCollider::shape() const
    {
        return m_shape;
    }

    std::unique_ptr<ShapeCollider> ShapeCollider::triangle(GameObject &owner,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer)
    {
        return std::unique_ptr<ShapeCollider>(new ShapeCollider(owner, Shape::Triangle,
            onCollisionBegin, onCollision, onCollisionEnd, layer));
    }

    std::unique_ptr<ShapeCollider> ShapeCollider::rectangle(GameObject &owner,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer)
    {
        return std::unique_ptr<ShapeCollider>(new ShapeCollider(owner, Shape::Rectangle,
            onCollisionBegin, onCollision, onCollisionEnd, layer));
    }

    std::unique_ptr<ShapeCollider> ShapeCollider::box(GameObject &owner,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer)
    {
        return std::unique_ptr<ShapeCollider>(new ShapeCollider(owner, Shape::Box,
            onCollisionBegin, onCollision, onCollisionEnd, layer));
    }

    std::unique_ptr<ShapeCollider> ShapeCollider::circle(GameObject &owner,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer)
    {
        return std::unique_ptr<ShapeCollider>(new ShapeCollider(owner, Shape::Circle,
            onCollisionBegin, onCollision, onCollisionEnd, layer));
    }

    std::unique_ptr<ShapeCollider> ShapeCollider::sphere(GameObject &owner,
        const std::function<void(Collider&)> &onCollisionBegin,
        const std::function<void(Collider&)> &onCollision,
        const std::function<void(Collider&)> &onCollisionEnd,
        std::int8_t layer)
    {
        return std::unique_ptr<ShapeCollider>(new ShapeCollider(owner, Shape::Sphere,
            onCollisionBegin, onCollision, onCollisionEnd, layer));
    }
} // namespace gamecoe
