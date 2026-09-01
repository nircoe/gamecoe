#include <gamecoe/core/game.hpp>
#include <gamecoe/graphics/vertex_array.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/paths.hpp>
#include <gamecoe_config.hpp>
#include <inputcoe.hpp>
#include <timecoe.hpp>
#include <algorithm>
#include <limits>
#include <utility>
#include <string>

#if GAMECOE_USE_OPENGL
    #include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

namespace gamecoe
{
    namespace
    {
        bool g_game_alive = false;

        void shutdown_subsystems(bool logcoe_up, bool glfw_up, bool soundcoe_up)
        {
            if (soundcoe_up) soundcoe::shutdown();
            if (glfw_up)     glfwTerminate();
            if (logcoe_up)   logcoe::shutdown();
        }

        std::int8_t clamp_layer(int layer, const std::string &caller)
        {
            constexpr int min_layer = std::numeric_limits<std::int8_t>::min();
            constexpr int max_layer = std::numeric_limits<std::int8_t>::max();
            if (layer < min_layer || layer > max_layer)
            {
                logcoe::warning(caller + ": layer " + std::to_string(layer) + " out of range [" +
                                 std::to_string(min_layer) + ", " + std::to_string(max_layer) + "], clamped");
                layer = std::clamp(layer, min_layer, max_layer);
            }
            return static_cast<std::int8_t>(layer);
        }
    }

    game::game(gamecoe::window &&main_window, const Color &background_color)
        : m_window(std::move(main_window))
    {
        set_background_color(background_color);
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
        graphics::vertex_array::destroy_shape_vertex_arrays();
        m_window.reset();
        shutdown_subsystems(true, true, true);
        g_game_alive = false;
    }

    std::expected<game, error> game::create(const std::string &title, std::uint32_t width,
                                            std::uint32_t height, const Color &background_color,
                                            logcoe::LogLevel log_level
#if GAMECOE_USE_SOUNDCOE
                                            , const soundcoe::init_config &soundcoe_config
#endif
                                            )
    {
        GAMECOE_ASSERT_LOG(!g_game_alive, "game::create(): a game instance is already alive");
        if (g_game_alive)
            return std::unexpected(detail::make_error(error_code::game_already_alive,
                                                      "game::create(): a game instance is already alive"));

        struct garbage_collector
        {
            bool logcoe_up = false, glfw_up = false, soundcoe_up = false, game_claimed = false;

            garbage_collector() = default;
            garbage_collector(const garbage_collector&) = delete;
            garbage_collector& operator=(const garbage_collector&) = delete;

            ~garbage_collector()
            {
                shutdown_subsystems(logcoe_up, glfw_up, soundcoe_up);
                if (game_claimed) g_game_alive = false;
            }
        } gc;

        g_game_alive = true;
        gc.game_claimed = true;

        // Always init at DEBUG so window::create()'s own info log below isn't silently dropped;
        // narrows to the caller's requested log_level right after.
        logcoe::initialize(logcoe::LogLevel::DEBUG, title);
        gc.logcoe_up = true;

        if (!glfwInit())
            return std::unexpected(detail::make_error(error_code::glfw_init_failure,
                                                      "game::create(): Failed to initialize glfw"));
        gc.glfw_up = true;

        auto main_window = gamecoe::window::create(title, width, height);
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

        gc.logcoe_up = gc.glfw_up = gc.soundcoe_up = gc.game_claimed = false;

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

    const gamecoe::window* game::window() const
    {
        return m_window.has_value() ? &(*m_window) : nullptr;
    }

    const Color& game::background_color() const
    {
        return m_background_color;
    }

    void game::set_background_color(const Color &background_color)
    {
        GAMECOE_ASSERT_LOG(m_window.has_value(), "game::set_background_color(): called on a moved-from game");
        if (!m_window.has_value()) return;

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

    game::scene_metadata* game::find_scene(scene_id id)
    {
        auto it = m_scenes.find(id);
        return it != m_scenes.end() ? &it->second : nullptr;
    }

    const game::scene_metadata* game::find_scene(scene_id id) const
    {
        auto it = m_scenes.find(id);
        return it != m_scenes.end() ? &it->second : nullptr;
    }

    void game::insert_active_scene_sorted(scene_id id, std::int8_t layer)
    {
        auto pos = std::upper_bound(m_active_scenes.begin(), m_active_scenes.end(), id,
            [this, layer](scene_id, scene_id b) { return layer < m_scenes.find(b)->second.layer; });
        m_active_scenes.insert(pos, id);
    }

    bool game::has_scene(scene_id id) const
    {
        return m_scenes.contains(id);
    }

    scene_status game::status(scene_id id) const
    {
        const scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::status(): scene is not registered");
        if (meta == nullptr) return scene_status::unloaded;
        return meta->status;
    }

    std::int8_t game::scene_layer(scene_id id) const
    {
        const scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::scene_layer(): scene is not registered");
        if (meta == nullptr) return 0;
        return meta->layer;
    }

    void game::set_scene_layer(scene_id id, int layer)
    {
        scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::set_scene_layer(): scene is not registered");
        if (meta == nullptr) return;

        const std::int8_t new_layer = clamp_layer(layer, "game::set_scene_layer()");
        if (new_layer == meta->layer) return;

        const std::int8_t old_layer = meta->layer;

        if (meta->status == scene_status::active)
        {
            std::erase(m_active_scenes, id);
            insert_active_scene_sorted(id, new_layer);
        }

        meta->layer = new_layer;

        logcoe::debug("game::set_scene_layer(): scene \"" + to_string(id) + "\" layer changed from " +
                      std::to_string(old_layer) + " to " + std::to_string(new_layer));
    }

    std::vector<entity> game::scene_entities(scene_id id) const
    {
        std::vector<entity> matches;
        m_entities.for_each_all<components::scene_tag>(
            [id, &matches](entity e, const components::scene_tag &tag)
            {
                if (tag.id == id) matches.push_back(e);
            });
        return matches;
    }

    void game::create_scene(scene_id id, scene_builder builder, int layer)
    {
        GAMECOE_ASSERT_LOG(!m_playing, "game::create_scene(): scene cannot be created during game::play()");
        if (m_playing) return;
        const bool exists = m_scenes.contains(id);
        GAMECOE_ASSERT_LOG(!exists, "game::create_scene(): scene is already registered");
        if (exists) return;
        GAMECOE_ASSERT_LOG(builder != nullptr, "game::create_scene(): scene builder is null");
        if (!builder) return;

        m_scenes.emplace(id, scene_metadata(builder, clamp_layer(layer, "game::create_scene()")));
        logcoe::debug("game::create_scene(): registered scene \"" + to_string(id) + "\"");
    }

    void game::load_scene(scene_id id)
    {
        scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::load_scene(): scene is not registered");
        if (!meta) return;

        GAMECOE_ASSERT_LOG(meta->status == scene_status::unloaded, "game::load_scene(): scene is not unloaded");
        if (meta->status != scene_status::unloaded) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::loaded });
            return;
        }

        const std::string scene_name = to_string(id);

        if (!soundcoe::preloadScene(scene_name))
            logcoe::warning("game::load_scene(): no audio preloaded for scene \"" + scene_name + "\"");

        meta->pending.reserve_from_last_build();
        meta->builder(meta->pending);

        meta->status = scene_status::loaded;
        logcoe::info("game::load_scene(): loaded scene \"" + scene_name + "\" (" +
                     std::to_string(meta->pending.spawn_count()) + " pending entities)");
    }

    void game::activate_scene(scene_id id)
    {
        scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::activate_scene(): scene is not registered");
        if (!meta) return;

        GAMECOE_ASSERT_LOG(meta->status == scene_status::loaded || meta->status == scene_status::inactive,
                           "game::activate_scene(): scene is not loaded or inactive");
        if (meta->status != scene_status::loaded && meta->status != scene_status::inactive) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::active });
            return;
        }

        // meta->status is set to active before the pending command_buffer flushes,
        // so if a potential command will call activate_scene on this id - it will see the status as active
        const scene_status prev_status = meta->status;
        meta->status = scene_status::active;

        std::size_t activated_count = 0;
        if (prev_status == scene_status::loaded)
        {
            activated_count = meta->pending.spawn_count();
            meta->pending.flush(m_entities, id);
        }
        else
        {
            // Only reactivate entities that were active before the deactivation of the scene.
            activated_count = meta->paused_active.size();
            for (entity e : meta->paused_active)
                if (m_entities.valid(e)) m_entities.activate(e);
            meta->paused_active.clear();
        }

        insert_active_scene_sorted(id, meta->layer);

        logcoe::info("game::activate_scene(): activated scene \"" + to_string(id) + "\" (" +
                     std::to_string(activated_count) + " entities)");
    }

    void game::deactivate_scene(scene_id id)
    {
        scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::deactivate_scene(): scene is not registered");
        if (!meta) return;

        GAMECOE_ASSERT_LOG(meta->status == scene_status::active, "game::deactivate_scene(): scene is not active");
        if (meta->status != scene_status::active) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::inactive });
            return;
        }

        std::erase(m_active_scenes, id);

        meta->paused_active.clear();
        m_entities.for_each<components::scene_tag>(
            [id, meta](entity e, const components::scene_tag &tag)
            {
                if (tag.id == id) meta->paused_active.push_back(e);
            });
        for (entity e : meta->paused_active)
            m_entities.deactivate(e);

        meta->status = scene_status::inactive;

        logcoe::info("game::deactivate_scene(): deactivated " + std::to_string(meta->paused_active.size()) +
                     " entities in scene \"" + to_string(id) + "\"");
    }

    void game::unload_scene(scene_id id)
    {
        scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::unload_scene(): scene is not registered");
        if (!meta) return;

        GAMECOE_ASSERT_LOG(meta->status != scene_status::unloaded, "game::unload_scene(): scene is already unloaded");
        if (meta->status == scene_status::unloaded) return;

        if (!m_playing)
        {
            // Not playing yet, queue this for prepare_to_play()'s startup drain instead of running now.
            m_pending_scene_ops.push_back({ id, scene_status::unloaded });
            return;
        }

        std::erase(m_active_scenes, id);

        const std::string scene_name = to_string(id);

        std::size_t destroyed_count = 0;
        if (meta->status != scene_status::loaded)
        {
            std::vector<entity> entities_to_destroy = scene_entities(id);
            for (entity e : entities_to_destroy)
            {
                if (m_entities.valid(e))
                {
                    m_entities.destroy(e);
                    ++destroyed_count;
                }
            }
        }

        if (!soundcoe::unloadScene(scene_name))
            logcoe::debug("game::unload_scene(): soundcoe had nothing loaded for scene \"" + scene_name + "\"");

        meta->pending.clear();
        meta->paused_active.clear();
        meta->status = scene_status::unloaded;

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
        GAMECOE_ASSERT_LOG(m_window.has_value(), "game::play(): called on a moved-from game");
        if (!m_window.has_value()) return;

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
