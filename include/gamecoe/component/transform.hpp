#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <type_traits>

namespace gamecoe
{
    namespace components
    {
        struct transform
        {
            glm::vec3 position{0.0f};
            glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
            glm::vec3 scale{1.0f};

            [[nodiscard]] glm::mat4 translation_matrix() const noexcept { return glm::translate(glm::mat4(1.0f), position); }
            [[nodiscard]] glm::mat4 rotation_matrix() const noexcept { return glm::mat4_cast(rotation); }
            [[nodiscard]] glm::mat4 scale_matrix() const noexcept { return glm::scale(glm::mat4(1.0f), scale); }
            // Built by hand to skip two mat4 multiplications, this runs per entity per frame.
            [[nodiscard]] glm::mat4 local_matrix() const noexcept
            {
                glm::mat4 m = glm::mat4_cast(rotation);
                m[0] *= scale.x;
                m[1] *= scale.y;
                m[2] *= scale.z;
                m[3] = glm::vec4(position, 1.0f);
                return m;
            }

            [[nodiscard]] glm::vec3 forward() const noexcept { return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f)); }
            [[nodiscard]] glm::vec3 right() const noexcept { return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f)); }
            [[nodiscard]] glm::vec3 up() const noexcept { return glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f)); }

            [[nodiscard]] glm::vec3 euler_rotation() const noexcept { return glm::eulerAngles(rotation); }

            void translate(glm::vec3 offset) { position += offset; }
            void rotate(glm::vec3 euler_offset) { rotation = glm::quat(euler_offset) * rotation; }
            void rotate_around(glm::vec3 axis, float angle) { rotation = rotation * glm::angleAxis(angle, glm::normalize(axis)); }
        };

        static_assert(std::is_standard_layout_v<transform>);
    } // namespace components
} // namespace gamecoe
