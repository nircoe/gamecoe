#pragma once

#include <gamecoe/core/window.hpp>
#include <gamecoe/core/scene_id.hpp>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/utils/error.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <colorcoe.hpp>
#include <cstdint>
#include <expected>
#include <flat_map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

#if GAMECOE_USE_SOUNDCOE
    #include <soundcoe.hpp>
#endif

namespace gamecoe
{
    using scene_builder = void(*)(command_buffer&);

    enum class scene_status : std::uint8_t { unloaded, loaded, active, inactive };

    class game;

#if GAMECOE_USE_TESTCOE
    void test_prepare_to_play(game& g);
#endif

    class game
    {
        struct scene_metadata
        {
            command_buffer       pending;
            std::vector<entity>  paused_active;   // snapshot of the active entities before the scene was deactivated
            scene_builder        builder;
            std::int8_t          layer;
            scene_status         status = scene_status::unloaded;

            scene_metadata(scene_builder builder, std::int8_t layer) : builder(builder), layer(layer) {}
        };

        struct pending_scene_op
        {
            scene_id      id;
            scene_status  target_status;
        };

        gamecoe::entities m_entities;
        std::optional<window> m_window;
        std::flat_map<scene_id, scene_metadata> m_scenes;
        std::vector<scene_id> m_active_scenes;   // sorted by layer
        std::vector<pending_scene_op> m_pending_scene_ops;
        Color m_background_color;
        bool m_playing = false;

        game(window &&main_window, const Color &background_color);
        // Returns a snapshot of the scene's entities
        std::vector<entity> collect_scene_entities(scene_id id);
        // Same, but fills (clearing first) an existing buffer instead of allocating a new one
        void collect_scene_entities(scene_id id, std::vector<entity>& out);
        void prepare_to_play();

#if GAMECOE_USE_TESTCOE
        friend void test_prepare_to_play(game&);
#endif

    public:
        game(const game&) = delete;
        game& operator=(const game&) = delete;
        game(game&& other) noexcept;
        game& operator=(game&&) = delete;
        ~game();

        [[nodiscard]] static std::expected<game, error> create(
            const std::string &title = "gamecoe", std::uint32_t width = 800, std::uint32_t height = 600,
            const Color &background_color = colorcoe::darkSlateGray(),
            logcoe::LogLevel log_level = logcoe::LogLevel::DEBUG
#if GAMECOE_USE_SOUNDCOE
            , const soundcoe::init_config &soundcoe_config = soundcoe::init_config{}
#endif
            );

        gamecoe::entities& entities();
        const gamecoe::entities& entities() const;

        const Color& background_color() const;
        void set_background_color(const Color &background_color);

        void set_log_level(logcoe::LogLevel level);

        void create_scene(scene_id id, scene_builder builder, std::int8_t layer = 0);
        void load_scene(scene_id id);
        void activate_scene(scene_id id);
        void deactivate_scene(scene_id id);
        void unload_scene(scene_id id);

        scene_status status(scene_id id) const;
        const std::vector<scene_id>& active_scenes() const;

        template <typename... Comps>
        entity create_entity(scene_id id, Comps&&... comps);

        void play();
    };

    template <typename... Comps>
    entity game::create_entity(scene_id id, Comps&&... comps)
    {
        static_assert((!std::is_same_v<std::decay_t<Comps>, components::scene_tag> && ...),
            "game::create_entity(): scene_tag is stamped from the scene_id argument, don't pass one");

        auto it = m_scenes.find(id);
        GAMECOE_ASSERT_LOG(it != m_scenes.end(), "game::create_entity(): scene is not registered");
        // entities can only be created into an active scene.
        GAMECOE_ASSERT_LOG(it == m_scenes.end() || it->second.status == scene_status::active,
                           "game::create_entity(): scene is not active");
        if (it == m_scenes.end() || it->second.status != scene_status::active) return entity::invalid();

        entity e = m_entities.create();
        m_entities.add_component<components::scene_tag>(e, components::scene_tag{ id });
        (detail::apply_component<std::decay_t<Comps>>(m_entities, e, std::forward<Comps>(comps)), ...);
        return e;
    }
} // namespace gamecoe
