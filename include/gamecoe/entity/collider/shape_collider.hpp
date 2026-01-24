#pragma once

#include <gamecoe/entity/component.hpp>
#include <gamecoe/entity/collider/collider.hpp>
#include <gamecoe/utils/shape.hpp>
#include <memory>
#include <cstdint>

namespace gamecoe
{
    class ShapeCollider : public Collider
    {
        Shape m_shape;

        ShapeCollider(GameObject &owner, Shape shape, 
                        const std::function<void(Collider&)> &onCollisionBegin = nullptr, 
                        const std::function<void(Collider&)> &onCollision = nullptr, 
                        const std::function<void(Collider&)> &onCollisionEnd = nullptr, 
                        std::int8_t layer = 0);

    public:
        ShapeCollider(const ShapeCollider &other) = delete;
        ShapeCollider& operator=(const ShapeCollider &other) = delete;
        ShapeCollider(ShapeCollider &&other) = delete;
        ShapeCollider& operator=(ShapeCollider &&other) = delete;
        virtual ~ShapeCollider() override;

        virtual void initialize() override;
        virtual void begin() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;

        virtual Shape shape() const override;

        static std::unique_ptr<ShapeCollider> triangle(GameObject &owner,
                                                        const std::function<void(Collider&)> &onCollisionBegin = nullptr,
                                                        const std::function<void(Collider&)> &onCollision = nullptr,
                                                        const std::function<void(Collider&)> &onCollisionEnd = nullptr,
                                                        std::int8_t layer = 0);
        static std::unique_ptr<ShapeCollider> rectangle(GameObject &owner,
                                                        const std::function<void(Collider&)> &onCollisionBegin = nullptr,
                                                        const std::function<void(Collider&)> &onCollision = nullptr,
                                                        const std::function<void(Collider&)> &onCollisionEnd = nullptr,
                                                        std::int8_t layer = 0);
        static std::unique_ptr<ShapeCollider> box(GameObject &owner,
                                                        const std::function<void(Collider&)> &onCollisionBegin = nullptr,
                                                        const std::function<void(Collider&)> &onCollision = nullptr,
                                                        const std::function<void(Collider&)> &onCollisionEnd = nullptr,
                                                        std::int8_t layer = 0);
        static std::unique_ptr<ShapeCollider> circle(GameObject &owner,
                                                        const std::function<void(Collider&)> &onCollisionBegin = nullptr,
                                                        const std::function<void(Collider&)> &onCollision = nullptr,
                                                        const std::function<void(Collider&)> &onCollisionEnd = nullptr,
                                                        std::int8_t layer = 0);
        static std::unique_ptr<ShapeCollider> sphere(GameObject &owner,
                                                        const std::function<void(Collider&)> &onCollisionBegin = nullptr,
                                                        const std::function<void(Collider&)> &onCollision = nullptr,
                                                        const std::function<void(Collider&)> &onCollisionEnd = nullptr,
                                                        std::int8_t layer = 0);
    };
} // namespace gamecoe
