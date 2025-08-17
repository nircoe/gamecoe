#include <gamecoe/shader.hpp>
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <logcoe.hpp>

namespace gamecoe
{
    void shader::logIfCreationFailed(const std::string &operation, unsigned int id) // I made it a static function (it doesn't use any member variable or method)
    {
        if (id != 0) return;
    
        std::string error = "failed to create " + operation;
        logcoe::error(error);
        throw std::runtime_error(error);
    }

    void shader::logIfCompileOrLinkFailed(const std::string &operation, unsigned int id, bool isProgram) // changed it to static function as well, is it more efficient?
    {
        GLint success;
        isProgram ? 
            glGetProgramiv(id, GL_LINK_STATUS, &success) :
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if(success) return;

        GLint logLength = 0;
        isProgram ?
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength) :
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

        if(logLength <= 0) return;

        std::string log(logLength, '\0'); // there are no redundent chars, you just see it because of a string issue
        isProgram ?
            glGetProgramInfoLog(id, logLength, nullptr, &(log[0])) :
            glGetShaderInfoLog(id, logLength, nullptr, &(log[0]));
        log.pop_back();

        log = operation + ": " + log;
        logcoe::error(log);
        throw std::runtime_error(log);
    }

    shader::shader(const std::string &vertexPath, const std::string &fragmentPath) : m_id(0)
    {
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
        catch(const std::ifstream::failure &f)
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
        logIfCreationFailed("vertex shader", vertexShader);
        glShaderSource(vertexShader, 1, &vertexCode, nullptr);
        glCompileShader(vertexShader);
        logIfCompileOrLinkFailed("vertex shader compilation failed", vertexShader, false);

        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        logIfCreationFailed("fragment shader", fragmentShader);
        glShaderSource(fragmentShader, 1, &fragmentCode, nullptr);
        glCompileShader(fragmentShader);
        logIfCompileOrLinkFailed("fragment shader compilation failed", fragmentShader, false);

        // link shaders
        m_id = glCreateProgram();
        logIfCreationFailed("shader program", m_id);
        glAttachShader(m_id, vertexShader);
        glAttachShader(m_id, fragmentShader);
        glLinkProgram(m_id);
        logIfCompileOrLinkFailed("shader program linking failed", m_id);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    shader::shader(shader &&other) noexcept : m_id(other.m_id)
    {
        other.m_id = 0;
    }

    shader &shader::operator=(shader &&other) noexcept
    {
        if(this != &other)
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
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void shader::set(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void shader::set(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }
} // namespace gamecoe