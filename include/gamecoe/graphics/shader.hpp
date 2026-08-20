#pragma once

#include <cstdint>
#include <expected>
#include <flat_map>
#include <gamecoe/utils/error.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace gamecoe
{
    namespace graphics
    {
        class shader
        {
            mutable std::flat_map<std::string, std::int32_t> m_uniform_locations;
            std::uint32_t m_id;

            explicit shader(std::uint32_t program_id);
            void destroy();
            std::int32_t uniform_location(const std::string &name) const;

        public:
            shader(const shader&) = delete;
            shader &operator=(const shader&) = delete;
            shader(shader &&other) noexcept;
            shader &operator=(shader &&other) noexcept;
            ~shader();

            [[nodiscard]] static std::expected<shader, error> create(
                const std::string &vertex_path, const std::string &fragment_path,
                const std::vector<std::string> &macros = {});

            std::uint32_t id() const;
            void use() const;

            void set(const std::string &name, bool value) const;
            void set(const std::string &name, std::int32_t value) const;
            void set(const std::string &name, float value) const;
            void set(const std::string &name, const glm::vec2 &value) const;
            void set(const std::string &name, const glm::vec3 &value) const;
            void set(const std::string &name, const glm::vec4 &value) const;
            void set(const std::string &name, const glm::ivec2 &value) const;
            void set(const std::string &name, const glm::ivec3 &value) const;
            void set(const std::string &name, const glm::ivec4 &value) const;
            void set(const std::string &name, const glm::mat2 &value) const;
            void set(const std::string &name, const glm::mat3 &value) const;
            void set(const std::string &name, const glm::mat4 &value) const;
            void set(const std::string &name, const glm::quat &value) const;
        };
    } // namespace graphics
} // namespace gamecoe
