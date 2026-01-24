#pragma once

#include <gamecoe/entity/component.hpp>
#include <functional>
#include <unordered_set>
#include <cstdint>

namespace gamecoe
{
    enum class Shape;

    class Collider : public Component<Collider>
    {
    protected: 
        std::int8_t m_layer;
        mutable std::unordered_set<std::uint32_t> m_collidedWith;

        std::function<void(const Collider&)> m_onCollisionBegin;    // First frame of a collision callback
        std::function<void(const Collider&)> m_onCollision;         // Continuous collision callback
        std::function<void(const Collider&)> m_onCollisionEnd;      // First frame after a collision ends callback

    public:
        static constexpr const char* TYPE_NAME = "Collider";

        Collider(GameObject &owner, const std::function<void(const Collider&)> &onCollisionBegin = nullptr, 
                                    const std::function<void(const Collider&)> &onCollision = nullptr, 
                                    const std::function<void(const Collider&)> &onCollisionEnd = nullptr,
                                    std::int8_t layer = 0);

        Collider(const Collider &other) = delete;
        Collider& operator=(const Collider &other) = delete;
        Collider(Collider &&other) = delete;
        Collider& operator=(Collider &&other) = delete;

        virtual ~Collider() = 0;

        virtual void initialize() override;
        virtual void begin() override;
        virtual void activate() override;
        virtual void deactivate() override;
        virtual void update() override;

        // Sets callback method for the first frame of a collision
        void setOnCollisionBegin(const std::function<void(const Collider&)> &onCollisionBegin);
        // Sets callback method for a continuous collision
        void setOnCollision(const std::function<void(const Collider&)> &onCollision);
        // Sets callback method for the first frame after a collision ends
        void setOnCollisionEnd(const std::function<void(const Collider&)> &onCollisionEnd);

        void setLayer(std::int8_t layer);
        std::int8_t layer() const;

        virtual Shape shape() const;
        void collide(const Collider &other) const;
    };
} // namespace gamecoe
