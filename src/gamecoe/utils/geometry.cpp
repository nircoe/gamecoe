#include <gamecoe/utils/geometry.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>

namespace gamecoe
{
    namespace geometry
    {
        const std::array<glm::vec3, 16>& circle::vertices()
        {
            static const std::array<glm::vec3, 16> verts = []() {
                std::array<glm::vec3, 16> result;
                constexpr float radius = 0.5f;
                constexpr int count = 16;

                for (int i = 0; i < count; ++i) {
                    float angle = (glm::two_pi<float>() * i) / count;
                    result[i] = glm::vec3{
                        radius * std::cos(angle),
                        radius * std::sin(angle),
                        0.0f
                    };
                }

                return result;
            }();

            return verts;
        }

        const std::array<glm::vec3, 42>& sphere::vertices()
        {
            static const std::array<glm::vec3, 42> verts = []() {
                std::array<glm::vec3, 42> result;
                constexpr float radius = 0.5f;

                // UV Sphere: 6 rings (latitude) × 7 segments (longitude) = 42 vertices
                // This gives even distribution across the sphere surface
                constexpr int rings = 6;
                constexpr int segments = 7;
                int index = 0;

                for (int ring = 0; ring < rings; ++ring) {
                    float theta = glm::pi<float>() * (ring + 1) / (rings + 1); // Latitude angle
                    float sinTheta = std::sin(theta);
                    float cosTheta = std::cos(theta);

                    for (int seg = 0; seg < segments; ++seg) {
                        float phi = glm::two_pi<float>() * seg / segments; // Longitude angle
                        float sinPhi = std::sin(phi);
                        float cosPhi = std::cos(phi);

                        result[index++] = glm::vec3{
                            radius * sinTheta * cosPhi,
                            radius * cosTheta,
                            radius * sinTheta * sinPhi
                        };
                    }
                }

                return result;
            }();

            return verts;
        }
    } // namespace geometry
} // namespace gamecoe
