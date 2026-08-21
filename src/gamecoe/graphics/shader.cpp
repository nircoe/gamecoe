#include <gamecoe/graphics/shader.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

#if GAMECOE_USE_OPENGL
#include <glad/gl.h>
#endif

namespace gamecoe
{
    namespace graphics
    {
        namespace
        {
#if GAMECOE_USE_OPENGL
            std::string get_glsl_version()
            {
#if GAMECOE_OPENGL_VERSION(2, 0)
                return "110";
#elif GAMECOE_OPENGL_VERSION(2, 1)
                return "120";
#elif GAMECOE_OPENGL_VERSION(3, 0)
                return "130";
#elif GAMECOE_OPENGL_VERSION(3, 1)
                return "140";
#elif GAMECOE_OPENGL_VERSION(3, 2)
                return "150";
#elif GAMECOE_OPENGL_VERSION_AT_LEAST(3, 3)
                return std::to_string(GAMECOE_GRAPHICS_VERSION_MAJOR) +
                       std::to_string(GAMECOE_GRAPHICS_VERSION_MINOR) + "0";
#else
                return "";
#endif
            }

            std::string get_opengl_profile()
            {
#if GAMECOE_OPENGL_VERSION_AT_LEAST(3, 2)
                return GAMECOE_PROFILE_CORE ? " core" : " compatibility";
#else
                return "";
#endif
            }

            std::expected<std::string, error> prepare_for_preprocessor(const std::string &shader_code,
                                                                         const std::vector<std::string> &macros)
            {
                if (shader_code.find("#version") != std::string::npos)
                    return std::unexpected(
                            detail::invalid_argument(
                                "shader::create(): Please remove \"#version\" statement from your shaders. "
                                "gamecoe auto-injects the version based on your CMake configuration"));

                std::string defines = "#version " + get_glsl_version() + get_opengl_profile() + "\n";
                defines += "#define GAMECOE_HAS_UBO " + std::to_string(GAMECOE_HAS_UBO) + "\n";
                defines += "#define GAMECOE_HAS_DSA " + std::to_string(GAMECOE_HAS_DSA) + "\n";

                for (const auto &macro : macros)
                    defines += "#define " + macro + " 1\n";

                return defines + shader_code;
            }

            std::expected<std::pair<std::string, std::string>, error> read_shader_files(
                const std::string &vertex_path, const std::string &fragment_path,
                const std::vector<std::string> &macros)
            {
                std::stringstream vertex_stream;
                std::stringstream fragment_stream;

                try
                {
                    std::ifstream vertex_shader_file;
                    std::ifstream fragment_shader_file;

                    vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                    fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

                    vertex_shader_file.open(vertex_path);
                    fragment_shader_file.open(fragment_path);

                    vertex_stream << vertex_shader_file.rdbuf();
                    fragment_stream << fragment_shader_file.rdbuf();
                }
                catch (const std::ifstream::failure &f)
                {
                    return std::unexpected(
                            detail::make_error(
                                error_code::file_read_failure,
                                "shader::create(): Failed to read shader file: " + std::string(f.what())));
                }

                auto vertex_result = prepare_for_preprocessor(vertex_stream.str(), macros);
                if (!vertex_result)
                    return std::unexpected(vertex_result.error());

                auto fragment_result = prepare_for_preprocessor(fragment_stream.str(), macros);
                if (!fragment_result)
                    return std::unexpected(fragment_result.error());

                return std::make_pair(*vertex_result, *fragment_result);
            }

            std::expected<void, error> check_compile_or_link_status(std::uint32_t id, bool is_program,
                                                                      error_code failure_code)
            {
                GLint success = GL_FALSE;
                is_program ? glGetProgramiv(id, GL_LINK_STATUS, &success) : glGetShaderiv(id, GL_COMPILE_STATUS, &success);

                if (success)
                    return {};

                GLint log_length = 0;
                is_program ? glGetProgramiv(id, GL_INFO_LOG_LENGTH, &log_length) : glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

                std::string log;
                if (log_length > 0)
                {
                    log.resize(log_length, '\0');
                    is_program ? glGetProgramInfoLog(id, log_length, nullptr, &(log[0])) : glGetShaderInfoLog(id, log_length, nullptr, &(log[0]));
                    log.pop_back();
                }
                else
                {
                    log = is_program ? "program linking failed (driver returned no info log)"
                                      : "shader compilation failed (driver returned no info log)";
                }

                return std::unexpected(detail::make_error(failure_code, "shader::create(): " + log));
            }
#endif
        } // namespace

        shader::shader(std::uint32_t program_id) : m_uniform_locations(), m_id(program_id) { }

        shader::shader(shader &&other) noexcept
            : m_uniform_locations(std::move(other.m_uniform_locations)), m_id(other.m_id)
        {
            other.reset();
        }

        shader &shader::operator=(shader &&other) noexcept
        {
            if (this == &other)
                return *this;

            destroy();

            m_id = other.m_id;
            m_uniform_locations = std::move(other.m_uniform_locations);

            other.reset();

            return *this;
        }

        void shader::destroy()
        {
#if GAMECOE_USE_OPENGL
            if (m_id != 0)
                glDeleteProgram(m_id);
#endif
        }

        void shader::reset() noexcept
        {
            m_id = 0;
            m_uniform_locations.clear();
        }

        shader::~shader()
        {
            destroy();
        }

        std::expected<shader, error> shader::create(
            [[maybe_unused]] const std::string &vertex_path, [[maybe_unused]] const std::string &fragment_path,
            [[maybe_unused]] const std::vector<std::string> &macros)
        {
#if GAMECOE_USE_OPENGL
            struct garbage_collector
            {
                std::uint32_t vertex = 0, fragment = 0, program = 0;

                garbage_collector() = default;
                garbage_collector(const garbage_collector &) = delete;
                garbage_collector &operator=(const garbage_collector &) = delete;

                ~garbage_collector()
                {
                    if (vertex != 0)   glDeleteShader(vertex);
                    if (fragment != 0) glDeleteShader(fragment);
                    if (program != 0)  glDeleteProgram(program);
                }
            } gc;

            auto shader_sources = read_shader_files(vertex_path, fragment_path, macros);
            if (!shader_sources)
                return std::unexpected(shader_sources.error());

            const char *vertex_code = shader_sources->first.c_str();
            const char *fragment_code = shader_sources->second.c_str();

            std::uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            gc.vertex = vertex_shader;
            auto vertex_check = detail::check_error("shader::create(): Vertex Shader:");
            if (!vertex_check)
                return std::unexpected(vertex_check.error());

            glShaderSource(vertex_shader, 1, &vertex_code, nullptr);
            glCompileShader(vertex_shader);
            auto vertex_compile = check_compile_or_link_status(vertex_shader, false, error_code::shader_compilation_failure);
            if (!vertex_compile)
                return std::unexpected(vertex_compile.error());

            std::uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            gc.fragment = fragment_shader;
            auto fragment_check = detail::check_error("shader::create(): Fragment Shader:");
            if (!fragment_check)
                return std::unexpected(fragment_check.error());

            glShaderSource(fragment_shader, 1, &fragment_code, nullptr);
            glCompileShader(fragment_shader);
            auto fragment_compile = check_compile_or_link_status(fragment_shader, false, error_code::shader_compilation_failure);
            if (!fragment_compile)
                return std::unexpected(fragment_compile.error());

            std::uint32_t program = glCreateProgram();
            gc.program = program;
            auto program_check = detail::check_error("shader::create(): Shader Program:");
            if (!program_check)
                return std::unexpected(program_check.error());

            glAttachShader(program, vertex_shader);
            glAttachShader(program, fragment_shader);
            glLinkProgram(program);
            auto link_check = check_compile_or_link_status(program, true, error_code::shader_link_failure);
            if (!link_check)
                return std::unexpected(link_check.error());

            gc.program = 0;
            return shader{program};
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "shader::create(): Only OpenGL supported at the moment"));
#endif
        }

        std::uint32_t shader::id() const
        {
            return m_id;
        }

        void shader::use() const
        {
            GAMECOE_ASSERT_LOG(m_id != 0, "shader::use(): cannot use a moved-from shader");
#if GAMECOE_USE_OPENGL
            glUseProgram(m_id);
#endif
        }

        std::int32_t shader::uniform_location(const std::string &name) const
        {
            auto it = m_uniform_locations.find(name);
            if (it != m_uniform_locations.end())
                return it->second;

            std::int32_t location = -1;
#if GAMECOE_USE_OPENGL
            location = glGetUniformLocation(m_id, name.c_str());
#endif
            if (location < 0)
                logcoe::warning("shader::set(): uniform \"" + name + "\" not found (stripped by compiler or misspelled)");

            m_uniform_locations[name] = location;
            return location;
        }

// set uniform macros
#if GAMECOE_HAS_DSA
#define setUniform1i(id, location, val)            glProgramUniform1i(id, location, val)
#define setUniform2iv(id, location, val)           glProgramUniform2iv(id, location, 1, val)
#define setUniform3iv(id, location, val)           glProgramUniform3iv(id, location, 1, val)
#define setUniform4iv(id, location, val)           glProgramUniform4iv(id, location, 1, val)
#define setUniform1f(id, location, val)            glProgramUniform1f(id, location, val)
#define setUniform2fv(id, location, val)           glProgramUniform2fv(id, location, 1, val)
#define setUniform3fv(id, location, val)           glProgramUniform3fv(id, location, 1, val)
#define setUniform4fv(id, location, val)           glProgramUniform4fv(id, location, 1, val)
#define setUniformMatrix2fv(id, location, val)     glProgramUniformMatrix2fv(id, location, 1, GL_FALSE, val)
#define setUniformMatrix3fv(id, location, val)     glProgramUniformMatrix3fv(id, location, 1, GL_FALSE, val)
#define setUniformMatrix4fv(id, location, val)     glProgramUniformMatrix4fv(id, location, 1, GL_FALSE, val)
#define setUniformQuat(id, location, val)          glProgramUniform4fv(id, location, 1, val)
#else
#define setUniform1i(id, location, val)            glUniform1i(location, val)
#define setUniform2iv(id, location, val)           glUniform2iv(location, 1, val)
#define setUniform3iv(id, location, val)           glUniform3iv(location, 1, val)
#define setUniform4iv(id, location, val)           glUniform4iv(location, 1, val)
#define setUniform1f(id, location, val)            glUniform1f(location, val)
#define setUniform2fv(id, location, val)           glUniform2fv(location, 1, val)
#define setUniform3fv(id, location, val)           glUniform3fv(location, 1, val)
#define setUniform4fv(id, location, val)           glUniform4fv(location, 1, val)
#define setUniformMatrix2fv(id, location, val)     glUniformMatrix2fv(location, 1, GL_FALSE, val)
#define setUniformMatrix3fv(id, location, val)     glUniformMatrix3fv(location, 1, GL_FALSE, val)
#define setUniformMatrix4fv(id, location, val)     glUniformMatrix4fv(location, 1, GL_FALSE, val)
#define setUniformQuat(id, location, val)          glUniform4fv(location, 1, val)
#endif

        void shader::set(const std::string &name, bool value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform1i(m_id, location, value);
#endif
        }

        void shader::set(const std::string &name, std::int32_t value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform1i(m_id, location, value);
#endif
        }

        void shader::set(const std::string &name, float value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform1f(m_id, location, value);
#endif
        }

        void shader::set(const std::string &name, const glm::vec2 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform2fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::vec3 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform3fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::vec4 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform4fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::ivec2 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform2iv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::ivec3 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform3iv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::ivec4 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniform4iv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::mat2 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniformMatrix2fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::mat3 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniformMatrix3fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::mat4 &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniformMatrix4fv(m_id, location, glm::value_ptr(value));
#endif
        }

        void shader::set(const std::string &name, const glm::quat &value) const
        {
            std::int32_t location = uniform_location(name);
#if GAMECOE_USE_OPENGL
            setUniformQuat(m_id, location, glm::value_ptr(value));
#endif
        }
    } // namespace graphics
} // namespace gamecoe
