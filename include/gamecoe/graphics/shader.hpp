#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

namespace gamecoe
{
    class Shader
    {
        std::uint32_t m_id;
        std::unordered_map<std::string, bool> m_macros;
        mutable std::unordered_map<std::string, std::int32_t> m_uniformLocation;

        static void logIfCompileOrLinkFailed(const std::string &operation, std::uint32_t id, bool isProgram = true);

        std::int32_t getUniformLocation(const std::string &name) const;

        void initializeMacros(const std::vector<std::string> &macros = {});
        std::string prepareForPreprocessor(const std::string &shaderCode);
        std::pair<std::string, std::string> readShaderFiles(const std::string &vertexPath, const std::string &fragmentPath);

    public:
        Shader() = delete;
        Shader(const std::string &vertexPath, const std::string &fragmentPath, const std::vector<std::string> &macros = {});
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;
        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;
        ~Shader();

        std::uint32_t id() const;

        void use();

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
} // namespace gamecoe