#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace gamecoe
{
    class shader
    {
        unsigned int m_id;
        mutable std::unordered_map<std::string, int> m_uniformLocation;

        static void logIfCreationFailed(const std::string &operation, unsigned int id);
        static void logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram = true);

        int getUniformLocation(const std::string &name) const;

    public:
        shader() = delete;
        shader(const std::string &vertexPath, const std::string &fragmentPath);
        shader(const shader &) = delete;
        shader &operator=(const shader &) = delete;
        shader(shader &&other) noexcept;
        shader &operator=(shader &&other) noexcept;
        ~shader();

        void use();

        void set(const std::string &name, bool value) const;
        void set(const std::string &name, int value) const;
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
} // namespace gamecoe