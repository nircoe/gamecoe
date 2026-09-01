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
#include <utility>
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
        std::optional<gamecoe::window> m_window;
        std::flat_map<scene_id, scene_metadata> m_scenes;
        std::vector<scene_id> m_active_scenes;   // sorted by layer
        std::vector<pending_scene_op> m_pending_scene_ops;
        Color m_background_color;
        bool m_playing = false;

        game(gamecoe::window &&main_window, const Color &background_color);
        scene_metadata* find_scene(scene_id id);
        const scene_metadata* find_scene(scene_id id) const;
        void insert_active_scene_sorted(scene_id id, std::int8_t layer);
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
        const gamecoe::window* window() const;

        const Color& background_color() const;
        void set_background_color(const Color &background_color);

        void set_log_level(logcoe::LogLevel level);

        void create_scene(scene_id id, scene_builder builder, int layer = 0);
        void load_scene(scene_id id);
        void activate_scene(scene_id id);
        void deactivate_scene(scene_id id);
        void unload_scene(scene_id id);

        bool has_scene(scene_id id) const;
        scene_status status(scene_id id) const;
        void set_scene_layer(scene_id id, int layer);
        std::int8_t scene_layer(scene_id id) const;
        // Returns a snapshot of the scene's entities (active + inactive)
        std::vector<entity> scene_entities(scene_id id) const;

        template <typename... Comps>
        entity create_entity(scene_id id, components::transform initial_transform = components::transform{}, Comps&&... comps);

        void play();
    };

    template <typename... Comps>
    entity game::create_entity(scene_id id, components::transform initial_transform, Comps&&... comps)
    {
        static_assert((!std::is_same_v<std::decay_t<Comps>, components::scene_tag> && ...),
            "game::create_entity(): scene_tag is stamped from the scene_id argument, don't pass one");
        static_assert((!std::is_same_v<std::decay_t<Comps>, components::transform> && ...),
            "game::create_entity(): transform is a built-in component, use the initial_transform parameter");
        static_assert((!hierarchy_component<std::decay_t<Comps>> && ...),
            "game::create_entity(): hierarchy components are managed - use entities::set_parent() instead");

        const scene_metadata* meta = find_scene(id);
        GAMECOE_ASSERT_LOG(meta != nullptr, "game::create_entity(): scene is not registered");
        // entities can only be created into an active scene.
        GAMECOE_ASSERT_LOG(meta == nullptr || meta->status == scene_status::active,
                           "game::create_entity(): scene is not active");
        if (meta == nullptr || meta->status != scene_status::active) return entity::invalid();

        entity e = m_entities.create(std::move(initial_transform));
        m_entities.add_component<components::scene_tag>(e, components::scene_tag{ id });
        (m_entities.add_component<std::decay_t<Comps>>(e, std::forward<Comps>(comps)), ...);
        return e;
    }
} // namespace gamecoe
