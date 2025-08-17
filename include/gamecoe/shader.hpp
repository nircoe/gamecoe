#pragma once

#include <string>

namespace gamecoe
{
    class shader
    {
        unsigned int m_id;

        static void logIfCreationFailed(const std::string &operation, unsigned int id);
        static void logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram = true);

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
    };
} // namespace gamecoe