#include <gamecoe/core/game.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/paths.hpp>
#include <gamecoe_config.hpp>
#include <inputcoe.hpp>
#include <timecoe.hpp>
#include <algorithm>
#include <utility>
#include <string>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

namespace gamecoe
{
    game::game(window &&main_window, const Color &background_color)
        : m_window(std::move(main_window)), m_background_color(background_color)
    {
#if GAMECOE_USE_OPENGL
        auto bg = m_background_color.normalized();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
#endif
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

        // Always init at DEBUG so window::create()'s own info log below isn't silently dropped;
        // narrows to the caller's requested log_level right after.
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
        resolved_config.audioRootDirectory = std::move(*audio_dir);

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
#if GAMECOE_USE_OPENGL
        auto bg = m_background_color.normalized();
        glClearColor(bg.r, bg.g, bg.b, bg.a);
#endif
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

    void game::create_scene(scene_id id, scene_builder builder, std::int8_t layer)
    {
        GAMECOE_ASSERT_LOG(!m_playing, "game::create_scene(): scene cannot be created during game::play()");
        if (m_playing) return;
        const bool exists = m_scenes.contains(id);
        GAMECOE_ASSERT_LOG(!exists, "game::create_scene(): scene is already registered");
        if (exists) return;
        GAMECOE_ASSERT_LOG(builder != nullptr, "game::create_scene(): scene builder is null");
        if (!builder) return;

        m_scenes.emplace(id, scene_metadata(builder, layer));
        logcoe::debug("game::create_scene(): registered scene \"" + to_string(id) + "\"");
    }

    void game::load_scene(scene_id id)
    {
        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::load_scene(): scene is not registered");
        if (it == m_scenes.end()) return;

        scene_metadata &meta = it->second;
        GAMECOE_ASSERT_LOG(meta.status == scene_status::unloaded, "game::load_scene(): scene is not unloaded");
        if (meta.status != scene_status::unloaded) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::loaded });
            return;
        }

        const std::string scene_name = to_string(id);

        if (!soundcoe::preloadScene(scene_name))
            logcoe::warning("game::load_scene(): no audio preloaded for scene \"" + scene_name + "\"");

        meta.builder(meta.pending);

        meta.status = scene_status::loaded;
        logcoe::info("game::load_scene(): loaded scene \"" + scene_name + "\" (" +
                     std::to_string(meta.pending.spawn_count()) + " pending entities)");
    }

    std::vector<entity> game::collect_scene_entities(scene_id id)
    {
        std::vector<entity> matches;
        collect_scene_entities(id, matches);
        return matches;
    }

    void game::collect_scene_entities(scene_id id, std::vector<entity>& out)
    {
        out.clear();
        m_entities.for_each_all<components::scene_tag>(
            [id, &out](entity e, const components::scene_tag &tag)
            {
                if (tag.id == id) out.push_back(e);
            });
    }

    void game::activate_scene(scene_id id)
    {
        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::activate_scene(): scene is not registered");
        if (it == m_scenes.end()) return;

        scene_metadata &meta = it->second;
        GAMECOE_ASSERT_LOG(meta.status == scene_status::loaded || meta.status == scene_status::inactive,
                           "game::activate_scene(): scene is not loaded or inactive");
        if (meta.status != scene_status::loaded && meta.status != scene_status::inactive) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::active });
            return;
        }

        // meta.status is set to active before the pending command_buffer flushes,
        // so if a potential command will call activate_scene on this id - it will see the status as active
        const scene_status prev_status = meta.status;
        meta.status = scene_status::active;

        std::size_t activated_count = 0;
        if (prev_status == scene_status::loaded)
        {
            activated_count = meta.pending.spawn_count();
            meta.pending.flush(m_entities, id);
        }
        else
        {
            // Only reactivate entities that were active before the deactivation of the scene.
            activated_count = meta.paused_active.size();
            for (entity e : meta.paused_active)
                if (m_entities.valid(e)) m_entities.activate(e);
            meta.paused_active.clear();
        }

        // Inserted sorted by layer so play()'s iteration order is already correct, no later re-shuffle.
        const std::int8_t new_layer = meta.layer;
        auto pos = std::upper_bound(m_active_scenes.begin(), m_active_scenes.end(), id,
            [this, new_layer](scene_id, scene_id b) { return new_layer < m_scenes.at(b).layer; });
        m_active_scenes.insert(pos, id);

        logcoe::info("game::activate_scene(): activated scene \"" + to_string(id) + "\" (" +
                     std::to_string(activated_count) + " entities)");
    }

    void game::deactivate_scene(scene_id id)
    {
        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::deactivate_scene(): scene is not registered");
        if (it == m_scenes.end()) return;

        scene_metadata &meta = it->second;
        GAMECOE_ASSERT_LOG(meta.status == scene_status::active, "game::deactivate_scene(): scene is not active");
        if (meta.status != scene_status::active) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::inactive });
            return;
        }

        std::erase(m_active_scenes, id);

        collect_scene_entities(id, meta.paused_active);

        std::size_t actives = 0;
        for (entity e : meta.paused_active)
        {
            if (!m_entities.is_active(e)) continue;
            m_entities.deactivate(e);
            meta.paused_active[actives++] = e;
        }
        meta.paused_active.erase(meta.paused_active.begin() + actives, meta.paused_active.end());

        meta.status = scene_status::inactive;

        logcoe::info("game::deactivate_scene(): deactivated scene \"" + to_string(id) + "\" (" +
                     std::to_string(meta.paused_active.size()) + " entities)");
    }

    void game::unload_scene(scene_id id)
    {
        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::unload_scene(): scene is not registered");
        if (it == m_scenes.end()) return;

        scene_metadata &meta = it->second;
        GAMECOE_ASSERT_LOG(meta.status != scene_status::unloaded, "game::unload_scene(): scene is already unloaded");
        if (meta.status == scene_status::unloaded) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::unloaded });
            return;
        }

        std::erase(m_active_scenes, id);

        const std::string scene_name = to_string(id);

        std::size_t destroyed_count = 0;
        if (meta.status != scene_status::loaded)
        {
            std::vector<entity> scene_entities = collect_scene_entities(id);
            destroyed_count = scene_entities.size();
            for (entity e : scene_entities)
                m_entities.destroy(e);
        }

        if (!soundcoe::unloadScene(scene_name))
            logcoe::debug("game::unload_scene(): soundcoe had nothing loaded for scene \"" + scene_name + "\"");

        meta.pending.clear();
        meta.paused_active.clear();
        meta.status = scene_status::unloaded;

        logcoe::info("game::unload_scene(): unloaded scene \"" + scene_name + "\" (" +
                     std::to_string(destroyed_count) + " destroyed entities)");
    }

    void game::prepare_to_play()
    {
        m_playing = true;

        for (const pending_scene_op &op : m_pending_scene_ops)
        {
            switch (op.target_status)
            {
                case scene_status::unloaded: unload_scene(op.id);     break;
                case scene_status::loaded:   load_scene(op.id);       break;
                case scene_status::active:   activate_scene(op.id);   break;
                case scene_status::inactive: deactivate_scene(op.id); break;
            }
        }

        m_pending_scene_ops.clear();
    }

    void game::play()
    {
        prepare_to_play();

        while (m_window->active())
        {
            timecoe::detail::update();
            inputcoe::detail::update();

#if GAMECOE_USE_OPENGL
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
            // System execution, rendering and collision wire in here via later tickets.

            soundcoe::update();
        }
    }
} // namespace gamecoe
