#include <gamecoe/graphics/shader.hpp>
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <gamecoe/utils/error_handler.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gamecoe
{
    void Shader::logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram) // do I really need that method?
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

        detail::throwError("Shader::Shader(): " + operation + ": " + log);
    }

    int Shader::getUniformLocation(const std::string &name) const
    {
        auto it = m_uniformLocation.find(name);
        if (it != m_uniformLocation.end())
            return it->second;

        int location = glGetUniformLocation(m_id, name.c_str());
        if(location < 0) detail::throwError("Shader::set(): Undefined Uniform: " + name);
        m_uniformLocation[name] = location;
        return location;
    }

    Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) : m_id(0)
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
            detail::throwError("Shader::Shader(): Failed to read shader file: " + std::string(f.what()));
        }

        std::string vertexSource = vertexStream.str();
        std::string fragmentSource = fragmentStream.str();

        const char *vertexCode = vertexSource.c_str();
        const char *fragmentCode = fragmentSource.c_str();

        // vertex Shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        gc.m_vertex = vertexShader;
        detail::checkAndThrowError("Shader::Shader(): Vertex Shader:");
        glShaderSource(vertexShader, 1, &vertexCode, nullptr);
        glCompileShader(vertexShader);
        logIfCompileOrLinkFailed("vertex Shader compilation failed", vertexShader, false); // do I really need all of those logIFCompileOrLinkFailed() calls? glGetError() in detail::checkAndThrowError() won't handle it the right way?

        // fragment Shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        gc.m_fragment = fragmentShader;
        detail::checkAndThrowError("Shader::Shader(): Fragment Shader:");
        glShaderSource(fragmentShader, 1, &fragmentCode, nullptr);
        glCompileShader(fragmentShader);
        logIfCompileOrLinkFailed("fragment Shader compilation failed", fragmentShader, false);

        // Shader program
        unsigned int program = glCreateProgram();
        gc.m_program = program;
        detail::checkAndThrowError("Shader::Shader(): Shader Program:");
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        logIfCompileOrLinkFailed("Shader program linking failed", program);
        gc.m_program = 0;
        m_id = program;
    }

    Shader::Shader(Shader &&other) noexcept : m_id(other.m_id)
    {
        other.m_id = 0;
    }

    Shader &Shader::operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            glDeleteProgram(m_id);
            m_id = other.m_id;
            other.m_id = 0;
        }
        return *this;
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_id);
        m_uniformLocation.clear();
    }

    void Shader::use()
    {
        glUseProgram(m_id);
    }

    void Shader::set(const std::string &name, bool value) const
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void Shader::set(const std::string &name, int value) const
    {
        glUniform1i(getUniformLocation(name), value);
    }

    void Shader::set(const std::string &name, float value) const
    {
        glUniform1f(getUniformLocation(name), value);
    }

    void Shader::set(const std::string &name, const glm::vec2 &value) const
    {
        glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::vec4 &value) const
    {
        glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::ivec2 &value) const
    {
        glUniform2iv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::ivec3 &value) const
    {
        glUniform3iv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::ivec4 &value) const
    {
        glUniform4iv(getUniformLocation(name), 1, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::mat2 &value) const
    {
        glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::mat3 &value) const
    {
        glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::mat4 &value) const
    {
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::set(const std::string &name, const glm::quat &value) const
    {
        glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
    }
} // namespace gamecoe