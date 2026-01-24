#include <gamecoe/utils/collision.hpp>
#include <gamecoe/utils/geometry.hpp>
#include <glm/glm.hpp>
#include <cassert>
#include <set>
#include <utility>
#include <algorithm>

namespace gamecoe
{
    namespace collision
    {
        constexpr float g_epsilon = glm::epsilon<float>();

        namespace vertices // To avoid holding more than 1 static vertices array for each shape
        {
            const std::array<glm::vec3, 3> &triangle()
            {
                static const std::array<glm::vec3, 3> vertices = geometry::triangle::vertices();
                return vertices;
            }

            const std::array<glm::vec3, 4> &rectangle()
            {
                static const std::array<glm::vec3, 4> vertices = geometry::rectangle::vertices();
                return vertices;
            }

            const std::array<glm::vec3, 16> &circle()
            {
                static const std::array<glm::vec3, 16> vertices = geometry::circle::vertices();
                return vertices;
            }
        } // namespace vertices

        namespace helpers
        {
            // Returns if the two Transforms have uniform scales
            bool uniformScales(const Transform &t1, const Transform &t2)
            {
                auto scale1 = t1.worldScale();
                auto scale2 = t2.worldScale();

                bool epsilonEqual1 = glm::epsilonEqual(scale1.x, scale1.y, g_epsilon) &&
                                    glm::epsilonEqual(scale1.y, scale1.z, g_epsilon);
                if (!epsilonEqual1) return false;

                bool epsilonEqual2 = glm::epsilonEqual(scale2.x, scale2.y, g_epsilon) &&
                                    glm::epsilonEqual(scale2.y, scale2.z, g_epsilon);
                if (!epsilonEqual2) return false;

                return true;
            }

            // Returns if the two Transforms have the same z-axis position
            bool sameZPosition(const Transform &t1, const Transform &t2)
            {
                return glm::epsilonEqual(t1.worldPosition().z, t2.worldPosition().z, g_epsilon);
            }

            // Returns if the two Transforms are both on the xy plane
            bool xyPlane(const Transform &t1, const Transform &t2)
            {
                auto normal1 = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * t1.worldRotation());
                auto normal2 = glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * t2.worldRotation());
                glm::vec3 xyPlaneNormal(0.0f, 0.0f, 1.0f);

                bool t1InXY = std::abs(glm::dot(normal1, xyPlaneNormal)) > (1.0f - g_epsilon);
                bool t2InXY = std::abs(glm::dot(normal2, xyPlaneNormal)) > (1.0f - g_epsilon);

                return t1InXY && t2InXY;
            }

            // Returns if the two Transforms are both on the same plane
            bool coplanar(const Transform &t1, const Transform &t2)
            {
                return glm::all(glm::epsilonEqual(t1.worldRotation(), t2.worldRotation(), g_epsilon));
            }

            // Returns if the two Transform both have the identity rotation
            bool identityRotation(const Transform& t)
            {
                glm::quat identity(1.0f, 0.0f, 0.0f, 0.0f);
                return glm::all(glm::epsilonEqual(t.worldRotation(), identity, g_epsilon));
            }
        } // namespace helpers

        namespace simple2D
        {
            bool uniformCircles(const Transform &t1, const Transform &t2)
            {
                float radiusSum = (t1.worldScale().x / 2.0f) + (t2.worldScale().x / 2.0f); // default radius is 0.5f for scale 1.0f
                glm::vec2 diff = t1.worldPosition() - t2.worldPosition();
                float distanceSquared = glm::dot(diff, diff);
                return distanceSquared <= radiusSum * radiusSum;
            }

            bool aabbTest(const Transform &t1, const Transform &t2)
            {
                auto center1 = t1.worldPosition2D();
                auto center2 = t2.worldPosition2D();

                auto scale1 = t1.worldScale2D();
                auto scale2 = t2.worldScale2D();
                float halfWidth1 = scale1.x / 2.0f;
                float halfwidth2 = scale2.x / 2.0f;
                float halfheight1 = scale1.y / 2.0f;
                float halfheight2 = scale2.y / 2.0f;

                if (center1.x + halfWidth1 >= center2.x - halfwidth2 &&
                    center1.x - halfWidth1 <= center2.x + halfwidth2 &&
                    center1.y + halfheight1 >= center2.y - halfheight2 &&
                    center1.y - halfheight1 <= center2.y + halfheight2) return true;
                
                return false;
            }
        } // namespace simpleCollisions

        namespace complex2D
        {
            bool lines(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, const glm::vec3 &v4)
            {
                float denominator = ((v4.y - v3.y) * (v2.x - v1.x)) - ((v4.x - v3.x) * (v2.y - v1.y));
                
                // check for parallel lines
                if (std::abs(denominator) < g_epsilon) 
                    return false;

                float uA = (((v4.x - v3.x) * (v1.y - v3.y)) - ((v4.y - v3.y) * (v1.x - v3.x))) / denominator;
                float uB = (((v2.x - v1.x) * (v1.y - v3.y)) - ((v2.y - v1.y) * (v1.x - v3.x))) / denominator;
                
                // check collision
                return ((0.0f <= uA && uA <= 1.0f) && (0.0f <= uB && uB <= 1.0f));
            }

            template<std::size_t N>
            bool pointInsidePolygon(const std::array<glm::vec3, N> &polygon, const glm::vec3 &point)
            {
                bool inside = false;

                for (std::size_t i = 0; i < N; ++i)
                {
                    std::size_t next = (i == N - 1) ? 0 : i + 1;
                    const glm::vec3 &v1 = polygon[i];
                    const glm::vec3 &v2 = polygon[next];

                    // if point.y is between v1.y and v2.y
                    // AND
                    // if ray from point crosses edge - Jordan Curve Theorem (Ray Casting)
                    // then toggle inside/outside
                    if (((v1.y >= point.y && point.y > v2.y) || (v1.y < point.y && point.y <= v2.y)) &&
                        (point.x < ((((v2.x - v1.x) * (point.y - v1.y)) / (v2.y - v1.y)) + v1.x)))
                        inside = !inside;
                }

                return inside;
            }

            template<std::size_t N>
            bool polygonWithLine(const std::array<glm::vec3, N> &polygon, const glm::vec3 &v1, const glm::vec3 &v2)
            {
                for (std::size_t i = 0; i < N; ++i)
                {
                    std::size_t next = (i == N - 1) ? 0 : i + 1;
                    // check collision
                    if (lines(v1, v2, polygon[i], polygon[next])) return true;
                }

                return false;
            }

            template<std::size_t N1, std::size_t N2>
            bool polygons(const std::array<glm::vec3, N1> &polygon1, const std::array<glm::vec3, N2> &polygon2)
            {
                for (std::size_t i = 0; i < N1; ++i)
                {
                    std::size_t next = (i == N1 - 1) ? 0 : i + 1;
                    // check collision
                    if (polygonWithLine(polygon2, polygon1[i], polygon1[next])) return true;
                }

                // check if one polygon is inside the other
                if (pointInsidePolygon(polygon1, polygon2[0]) || pointInsidePolygon(polygon2, polygon1[0])) 
                    return true;

                return false;
            }
        } // namespace complex2D

        bool triangles(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::identityRotation(t1) && helpers::identityRotation(t2) && !simple2D::aabbTest(t1, t2))
                    return false;

                const auto &vertices = vertices::triangle();
                const auto triangleVertices1 = geometry::transformVertices(vertices, t1.modelMatrix());
                const auto triangleVertices2 = geometry::transformVertices(vertices, t2.modelMatrix());
                return complex2D::polygons(triangleVertices1, triangleVertices2);
            }

            // Triangles rotated in the 3D space
            return false; // not supported yet
        }

        bool rectangles(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::identityRotation(t1) && helpers::identityRotation(t2))
                    return simple2D::aabbTest(t1, t2);
                
                const auto &vertices = vertices::rectangle();
                const auto rectangleVertices1 = geometry::transformVertices(vertices, t1.modelMatrix());
                const auto rectangleVertices2 = geometry::transformVertices(vertices, t2.modelMatrix());
                return complex2D::polygons(rectangleVertices1, rectangleVertices2);
            }

            // Rectangles rotated in the 3D space
            return false; // not supported yet
        }

        bool boxes([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool circles(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::uniformScales(t1, t2))
                    return simple2D::uniformCircles(t1, t2);

                const auto &vertices = vertices::circle();
                auto circleVertices1 = geometry::transformVertices(vertices, t1.modelMatrix());
                auto circleVertices2 = geometry::transformVertices(vertices, t2.modelMatrix());
                return complex2D::polygons(circleVertices1, circleVertices2);
            }

            // Circles rotated in the 3D space
            return false; // not supported yet
        }

        bool spheres([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool triangleWithRectangle(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::identityRotation(t1) && helpers::identityRotation(t2) && 
                    !simple2D::aabbTest(t1, t2))
                    return false;

                const auto triangle = geometry::transformVertices(vertices::triangle(), t1.modelMatrix());
                const auto rectangle = geometry::transformVertices(vertices::rectangle(), t2.modelMatrix());

                return complex2D::polygons(triangle, rectangle);
            }           
            
            // triangle/rectangle rotated in the 3D space
            return false; // not supported yet
        }

        bool triangleWithBox([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool triangleWithCircle(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::identityRotation(t1) && helpers::identityRotation(t2) && 
                    !simple2D::aabbTest(t1, t2))
                    return false;

                const auto triangle = geometry::transformVertices(vertices::triangle(), t1.modelMatrix());
                const auto circle = geometry::transformVertices(vertices::circle(), t2.modelMatrix());

                return complex2D::polygons(triangle, circle);
            }

            // triangle/circle rotated in the 3D space
            return false; // not supported yet
        }

        bool triangleWithSphere([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool rectangleWithBox([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool rectangleWithCircle(const Transform &t1, const Transform &t2)
        {
            if (helpers::xyPlane(t1, t2) && helpers::sameZPosition(t1, t2))
            {
                if (helpers::identityRotation(t1) && helpers::identityRotation(t2) && 
                    !simple2D::aabbTest(t1, t2))
                    return false;

                const auto rectangle = geometry::transformVertices(vertices::rectangle(), t1.modelMatrix());
                const auto circle = geometry::transformVertices(vertices::circle(), t2.modelMatrix());

                return complex2D::polygons(rectangle, circle);
            }

            // rectangle/circle rotated in the 3D space
            return false; // not supported yet
        }

        bool rectangleWithSphere([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool boxWithCircle([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool boxWithSphere([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool circleWithSphere([[maybe_unused]] const Transform &t1, [[maybe_unused]] const Transform &t2)
        {
            return false; // not supported yet
        }

        bool detect(const Transform &t1, Shape s1, const Transform &t2, Shape s2)
        {
            assert(s1 != Shape::Invalid && s2 != Shape::Invalid); // not suppose to happen

            bool shapeOrder = s1 < s2;
            std::pair<Shape, Shape> shapes = shapeOrder ? 
                                                std::pair<Shape, Shape>(s1, s2) : 
                                                std::pair<Shape, Shape>(s2, s1);
            const Transform &first = shapeOrder ? t1 : t2;
            const Transform &second = shapeOrder ? t2 : t1;

            switch(shapes.first)
            {
            case Shape::Triangle:
                switch(shapes.second)
                {
                case Shape::Triangle:
                    return triangles(first, second);
                case Shape::Rectangle:
                    return triangleWithRectangle(first, second);
                case Shape::Box:
                    return triangleWithBox(first, second);
                case Shape::Circle:
                    return triangleWithCircle(first, second);
                case Shape::Sphere:
                default:
                    return triangleWithSphere(first, second);
                }
                break;
            case Shape::Rectangle:
                switch(shapes.second)
                {
                case Shape::Rectangle:
                    return rectangles(first, second);
                case Shape::Box:
                    return rectangleWithBox(first, second);
                case Shape::Circle:
                    return rectangleWithCircle(first, second);
                case Shape::Sphere:
                default:
                    return rectangleWithSphere(first, second);
                }
                break;
            case Shape::Box:
                switch(shapes.second)
                {
                case Shape::Box:
                    return boxes(first, second);
                case Shape::Circle:
                    return boxWithCircle(first, second);
                case Shape::Sphere:
                default:
                    return boxWithSphere(first, second);
                }
                break;
            case Shape::Circle:
                switch(shapes.second)
                {
                case Shape::Circle:
                    return circles(first, second);
                case Shape::Sphere:
                default:
                    return circleWithSphere(first, second);
                }
                break;
            case Shape::Sphere:
            default:
                return spheres(first, second);
            }

            assert(false);
            return false;
        }
    } // namespace collision
} // namespace gamecoe
