#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>

namespace gamecoe
{
    namespace geometry
    {
        namespace triangle
        {
            [[nodiscard]] constexpr std::array<glm::vec3, 3> vertices() noexcept
            {
                return
                {
                    glm::vec3{-0.5f, -0.5f, 0.0f},
                    glm::vec3{ 0.5f, -0.5f, 0.0f},
                    glm::vec3{ 0.0f,  0.5f, 0.0f}
                };
            }

            [[nodiscard]] constexpr std::array<float, 9> vertices_flat() noexcept
            {
                constexpr auto verts = vertices();
                return
                {
                    verts[0].x, verts[0].y, verts[0].z,
                    verts[1].x, verts[1].y, verts[1].z,
                    verts[2].x, verts[2].y, verts[2].z
                };
            }

            [[nodiscard]] constexpr std::array<std::uint32_t, 3> indices() noexcept
            {
                return
                {
                    0, 1, 2,
                };
            }
        } // namespace triangle


        namespace rectangle
        {
            [[nodiscard]] constexpr std::array<glm::vec3, 4> vertices() noexcept
            {
                return
                {
                    glm::vec3{-0.5f, -0.5f, 0.0f},
                    glm::vec3{ 0.5f, -0.5f, 0.0f},
                    glm::vec3{ 0.5f,  0.5f, 0.0f},
                    glm::vec3{-0.5f,  0.5f, 0.0f}
                };
            }

            [[nodiscard]] constexpr std::array<float, 12> vertices_flat() noexcept
            {
                constexpr auto verts = vertices();
                return
                {
                    verts[0].x, verts[0].y, verts[0].z,
                    verts[1].x, verts[1].y, verts[1].z,
                    verts[2].x, verts[2].y, verts[2].z,
                    verts[3].x, verts[3].y, verts[3].z
                };
            }

            [[nodiscard]] constexpr std::array<std::uint32_t, 6> indices() noexcept
            {
                return
                {
                    0, 1, 2,
                    2, 3, 0
                };
            }
        } // namespace rectangle

        namespace box
        {
            [[nodiscard]] constexpr std::array<glm::vec3, 8> vertices() noexcept
            {
                return
                {
                    glm::vec3{-0.5f, -0.5f,  0.5f},
                    glm::vec3{ 0.5f, -0.5f,  0.5f},
                    glm::vec3{ 0.5f,  0.5f,  0.5f},
                    glm::vec3{-0.5f,  0.5f,  0.5f},
                    glm::vec3{-0.5f, -0.5f, -0.5f},
                    glm::vec3{ 0.5f, -0.5f, -0.5f},
                    glm::vec3{ 0.5f,  0.5f, -0.5f},
                    glm::vec3{-0.5f,  0.5f, -0.5f}
                };
            }

            [[nodiscard]] constexpr std::array<float, 24> vertices_flat() noexcept
            {
                constexpr auto verts = vertices();
                return
                {
                    verts[0].x, verts[0].y, verts[0].z,
                    verts[1].x, verts[1].y, verts[1].z,
                    verts[2].x, verts[2].y, verts[2].z,
                    verts[3].x, verts[3].y, verts[3].z,
                    verts[4].x, verts[4].y, verts[4].z,
                    verts[5].x, verts[5].y, verts[5].z,
                    verts[6].x, verts[6].y, verts[6].z,
                    verts[7].x, verts[7].y, verts[7].z
                };
            }

            [[nodiscard]] constexpr std::array<std::uint32_t, 36> indices() noexcept
            {
                return
                {
                    0, 1, 2, 2, 3, 0,
                    4, 5, 6, 6, 7, 4,
                    7, 3, 0, 0, 4, 7,
                    1, 5, 6, 6, 2, 1,
                    4, 0, 1, 1, 5, 4,
                    3, 7, 6, 6, 2, 3
                };
            }
        } // namespace box

        namespace circle
        {
            [[nodiscard]] const std::array<glm::vec3, 16>& vertices();
        } // namespace circle

        namespace sphere
        {
            [[nodiscard]] const std::array<glm::vec3, 42>& vertices();
        } // namespace sphere

        template<std::size_t N>
        [[nodiscard]] std::array<glm::vec3, N> transform_vertices(const std::array<glm::vec3, N> &vertices, const glm::mat4 &matrix)
        {
            std::array<glm::vec3, N> transformed;
            for (std::size_t i = 0; i < N; ++i)
                transformed[i] = glm::vec3(matrix * glm::vec4(vertices[i], 1.0f));

            return transformed;
        }
    } // namespace geometry
} // namespace gamecoe
