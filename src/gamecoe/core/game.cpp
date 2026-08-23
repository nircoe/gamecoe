#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/paths.hpp>
#include <gamecoe_config.hpp>
#include <utility>
#include <string>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

namespace gamecoe
{
    game::game(window &&main_window, const Color &background_color)
        : m_entities(), m_window(std::move(main_window)), m_scenes(), m_active_scenes(),
          m_background_color(background_color)
    {
    }

    game::game(game&& other) noexcept
        : m_entities(std::move(other.m_entities)), m_window(std::move(other.m_window)),
          m_scenes(std::move(other.m_scenes)), m_active_scenes(std::move(other.m_active_scenes)),
          m_background_color(other.m_background_color)
    {
        other.m_window.reset();
    }

    game::~game()
    {
        if (!m_window.has_value()) return; // already moved from, nothing to clean up

        m_entities.clear();
        m_window.reset();
        glfwTerminate();
        soundcoe::shutdown();
        logcoe::shutdown();
    }

    std::expected<game, error> game::create(const std::string &title, std::uint32_t width,
                                            std::uint32_t height, const Color &background_color,
                                            logcoe::LogLevel log_level
#if GAMECOE_USE_SOUNDCOE
                                            , const soundcoe::init_config &soundcoe_config
#endif
                                            )
    {
        struct garbage_collector
        {
            bool logcoe_up = false, glfw_up = false, soundcoe_up = false;

            garbage_collector() = default;
            garbage_collector(const garbage_collector&) = delete;
            garbage_collector& operator=(const garbage_collector&) = delete;

            ~garbage_collector()
            {
                if (soundcoe_up) soundcoe::shutdown();
                if (glfw_up)     glfwTerminate();
                if (logcoe_up)   logcoe::shutdown();
            }
        } gc;

        logcoe::initialize(logcoe::LogLevel::DEBUG, title);
        gc.logcoe_up = true;

        if (!glfwInit())
            return std::unexpected(detail::make_error(error_code::glfw_init_failure,
                                                      "game::create(): Failed to initialize glfw"));
        gc.glfw_up = true;

        auto main_window = window::create(title, width, height);
        if (!main_window) return std::unexpected(main_window.error());

#if GAMECOE_USE_OPENGL
        if (!gladLoadGL(glfwGetProcAddress))
            return std::unexpected(detail::make_error(error_code::glad_load_failure,
                                                      "game::create(): Failed to load glad"));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

#if GAMECOE_USE_SOUNDCOE
        auto audio_dir = resolve_path(soundcoe_config.audioRootDirectory);
        if (!audio_dir) return std::unexpected(audio_dir.error());

        soundcoe::init_config resolved_config = soundcoe_config;
        resolved_config.audioRootDirectory = *audio_dir;

        if (!soundcoe::initialize(resolved_config))
            return std::unexpected(detail::make_error(error_code::soundcoe_init_failure,
                                                      "game::create(): Failed to initialize soundcoe"));
        gc.soundcoe_up = true;
#endif

        gc.logcoe_up = gc.glfw_up = gc.soundcoe_up = false;

        logcoe::setLogLevel(log_level);
        logcoe::info("game::create(): created \"" + title + "\"");

        return game{ std::move(*main_window), background_color };
    }

    gamecoe::entities& game::entities()
    {
        return m_entities;
    }

    const gamecoe::entities& game::entities() const
    {
        return m_entities;
    }

    const Color& game::background_color() const
    {
        return m_background_color;
    }

    void game::set_background_color(const Color &background_color)
    {
        m_background_color = background_color;
    }

    void game::set_log_level(logcoe::LogLevel level)
    {
        logcoe::setLogLevel(level);
    }

    scene_status game::status(scene_id id) const
    {
        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::status(): scene is not registered");
        if (it == m_scenes.end()) return scene_status::unloaded;
        return it->second.status;
    }

    const std::vector<scene_id>& game::active_scenes() const
    {
        return m_active_scenes;
    }
} // namespace gamecoe
