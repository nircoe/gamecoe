#include <gamecoe/shader.hpp>
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <logcoe.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gamecoe
{
    void shader::logIfCreationFailed(const std::string &operation, unsigned int id)
    {
        if (id != 0)
            return;

        std::string error = "failed to create " + operation;
        logcoe::error(error);
        throw std::runtime_error(error);
    }

    void shader::logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram)
    {
        GLint success;
        isProgram ? glGetProgramiv(id, GL_LINK_STATUS, &success) : glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if (success)
            return;

        GLint logLength = 0;
        isProgram ? glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength) : glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength <= 0)
            return;

        std::string log(logLength, '\0');
        isProgram ? glGetProgramInfoLog(id, logLength, nullptr, &(log[0])) : glGetShaderInfoLog(id, logLength, nullptr, &(log[0]));
        log.pop_back();

        log = operation + ": " + log;
        logcoe::error(log);
        throw std::runtime_error(log);
    }

    int shader::getUniformLocation(const std::string &name) const
    {
        auto it = m_uniformLocation.find(name);
        if (it != m_uniformLocation.end())
            return it->second;

        int location = glGetUniformLocation(m_id, name.c_str());
        m_uniformLocation[name] = location;
        return location;
    }

    shader::shader(const std::string &vertexPath, const std::string &fragmentPath) : m_id(0)
    {
        struct garbage_collector
        {
            unsigned int m_vertex = 0, m_fragment = 0, m_program = 0;

            garbage_collector() { }
            garbage_collector(const garbage_collector &) = delete;
            garbage_collector &operator=(const garbage_collector &) = delete;

            ~garbage_collector()
            {
                glDeleteShader(m_vertex);
                glDeleteShader(m_fragment);
                glDeleteProgram(m_program);
            }
        } gc;

        // reading the glsl files
        std::stringstream vertexStream;
        std::stringstream fragmentStream;

        try
        {
            std::ifstream vertexShaderFile;
            std::ifstream fragmentShaderFile;

            vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            vertexShaderFile.open(vertexPath);
            fragmentShaderFile.open(fragmentPath);

            vertexStream << vertexShaderFile.rdbuf();
            fragmentStream << fragmentShaderFile.rdbuf();
        }
        catch (const std::ifstream::failure &f)
        {
            std::string error = "failed to read shader file: " + std::string(f.what());
            logcoe::error(error);
            throw std::runtime_error(error);
        }

        std::string vertexSource = vertexStream.str();
        std::string fragmentSource = fragmentStream.str();

        const char *vertexCode = vertexSource.c_str();
        const char *fragmentCode = fragmentSource.c_str();

        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        gc.m_vertex = vertexShader;
        logIfCreationFailed("vertex shader", vertexShader);
        glShaderSource(vertexShader, 1, &vertexCode, nullptr);
        glCompileShader(vertexShader);
        logIfCompileOrLinkFailed("vertex shader compilation failed", vertexShader, false);

        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        gc.m_fragment = fragmentShader;
        logIfCreationFailed("fragment shader", fragmentShader);
        glShaderSource(fragmentShader, 1, &fragmentCode, nullptr);
        glCompileShader(fragmentShader);
        logIfCompileOrLinkFailed("fragment shader compilation failed", fragmentShader, false);

        // shader program
        unsigned int program = glCreateProgram();
        gc.m_program = program;
        logIfCreationFailed("shader program", program);
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        logIfCompileOrLinkFailed("shader program linking failed", program);
        gc.m_program = 0;
        m_id = program;
    }

    shader::shader(shader &&other) noexcept : m_id(other.m_id)
    {
        other.m_id = 0;
    }

    shader &shader::operator=(shader &&other) noexcept
    {
        if (this != &other)
        {
            glDeleteProgram(m_id);
            m_id = other.m_id;
            other.m_id = 0;
        }
        return *this;
    }

    shader::~shader()
    {
        glDeleteProgram(m_id);
    }

    void shader::use()
    {
        glUseProgram(m_id);
    }

    void shader::set(const std::string &name, bool value) const
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void shader::set(const std::string &name, int value) const
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void shader::set(const std::string &name, float value) const
    {
        glUniform1f(getUniformLocation(name), value);
    }

    void shader::set(const std::string &name, const glm::vec2 &value) const
    {
        glUniform2fv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::vec4 &value) const
    {
        glUniform4fv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::ivec2 &value) const
    {
        glUniform2iv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::ivec3 &value) const
    {
        glUniform3iv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::ivec4 &value) const
    {
        glUniform4iv(getUniformLocation(name), 1, &value[0]);
    }

    void shader::set(const std::string &name, const glm::mat2 &value) const
    {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void shader::set(const std::string &name, const glm::mat3 &value) const
    {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void shader::set(const std::string &name, const glm::mat4 &value) const
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]); // should we do like this or glm::value_ptr(value) ?
    }

    void shader::set(const std::string &name, const glm::quat &value) const
    {
        glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
    }
} // namespace gamecoe