#include <gamecoe/graphics/shader.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <glm/gtc/type_ptr.hpp>

#if GAMECOE_USE_OPENGL
#include <glad/gl.h>
#endif

namespace gamecoe
{
    void Shader::logIfCompileOrLinkFailed(const std::string &operation, std::uint32_t id, bool isProgram) // do I really need that method?
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

    std::int32_t Shader::getUniformLocation(const std::string &name) const
    {
        auto it = m_uniformLocation.find(name);
        if (it != m_uniformLocation.end())
            return it->second;

        std::int32_t location = glGetUniformLocation(m_id, name.c_str());
        if(location < 0) detail::throwError("Shader::set(): Undefined Uniform: " + name);
        m_uniformLocation[name] = location;
        return location;
    }

    Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) : m_id(0)
    {
        struct garbage_collector
        {
            std::uint32_t m_vertex = 0, m_fragment = 0, m_program = 0;

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
        std::uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
        gc.m_vertex = vertexShader;
        detail::checkAndThrowError("Shader::Shader(): Vertex Shader:");
        glShaderSource(vertexShader, 1, &vertexCode, nullptr);
        glCompileShader(vertexShader);
        logIfCompileOrLinkFailed("vertex Shader compilation failed", vertexShader, false); // do I really need all of those logIFCompileOrLinkFailed() calls? glGetError() in detail::checkAndThrowError() won't handle it the right way?

        // fragment Shader
        std::uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        gc.m_fragment = fragmentShader;
        detail::checkAndThrowError("Shader::Shader(): Fragment Shader:");
        glShaderSource(fragmentShader, 1, &fragmentCode, nullptr);
        glCompileShader(fragmentShader);
        logIfCompileOrLinkFailed("fragment Shader compilation failed", fragmentShader, false);

        // Shader program
        std::uint32_t program = glCreateProgram();
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
#if !GAMECOE_HAS_DSA
        glUseProgram(m_id);
#endif
    }

    void Shader::set(const std::string &name, bool value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform1i(m_id, location, value);
#else
        glUniform1i(location, value);
#endif
    }

    void Shader::set(const std::string &name, int value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform1i(m_id, location, value);
#else
        glUniform1i(location, value);
#endif
    }

    void Shader::set(const std::string &name, float value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform1f(m_id, location, value);
#else
        glUniform1f(location, value);
#endif
    }

    void Shader::set(const std::string &name, const glm::vec2 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform2fv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform2fv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::vec3 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform3fv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform3fv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::vec4 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform4fv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform4fv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::ivec2 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform2iv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform2iv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::ivec3 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform3iv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform3iv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::ivec4 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform4iv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform4iv(location, 1, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::mat2 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniformMatrix2fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
#else
        glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::mat3 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniformMatrix3fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
#else
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::mat4 &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
#else
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
#endif
    }

    void Shader::set(const std::string &name, const glm::quat &value) const
    {
        std::int32_t location = getUniformLocation(name);
#if GAMECOE_HAS_DSA
        glProgramUniform4fv(m_id, location, 1, glm::value_ptr(value));
#else
        glUniform4fv(location, 1, glm::value_ptr(value));
#endif
    }
} // namespace gamecoe