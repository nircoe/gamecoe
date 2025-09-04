#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace gamecoe
{
    class Shader
    {
        unsigned int m_id;
        mutable std::unordered_map<std::string, int> m_uniformLocation;

        static void logIfCreationFailed(const std::string &operation, unsigned int id);
        static void logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram = true);

        int getUniformLocation(const std::string &name) const;

    public:
        Shader() = delete;
        Shader(const std::string &vertexPath, const std::string &fragmentPath);
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;
        ~Shader();

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