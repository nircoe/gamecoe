#pragma once

#include <gamecoe/core/window.hpp>
#include <gamecoe/core/scene_id.hpp>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/system/system.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/utils/error.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>
#include <colorcoe.hpp>
#include <cstdint>
#include <expected>
#include <flat_map>
#include <initializer_list>
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
    void test_run_systems(game& g);
#endif

    class game
    {
        struct scene_metadata
        {
            command_buffer       pending;
            scene_builder        builder;
            std::int8_t          layer;
            scene_status         status = scene_status::unloaded;

            scene_metadata(scene_builder builder, std::int8_t layer) : builder(builder), layer(layer) {}
        };

        static_assert(std::is_nothrow_move_constructible_v<scene_metadata>,
            "scene_metadata must stay nothrow-movable - flat_map reallocation falls back to copying otherwise");

        struct pending_scene_op
        {
            scene_id      id;
            scene_status  target_status;
        };

        static_assert(std::is_nothrow_move_constructible_v<pending_scene_op>);

        gamecoe::entities m_entities;
        std::optional<gamecoe::window> m_window;
        std::flat_map<scene_id, scene_metadata> m_scenes;
        std::vector<scene_id> m_active_scenes;   // sorted by layer
        std::vector<pending_scene_op> m_pending_scene_ops;
        std::vector<system_entry> m_systems;
        Color m_background_color;
        bool m_playing = false;

        game(gamecoe::window &&main_window, const Color &background_color);
        scene_metadata* find_scene(scene_id id);
        const scene_metadata* find_scene(scene_id id) const;
        void insert_active_scene_sorted(scene_id id, std::int8_t layer);
        void prepare_to_play();
        void run_systems();
        void add_system(system_entry &&entry);

#if GAMECOE_USE_TESTCOE
        friend void test_prepare_to_play(game&);
        friend void test_run_systems(game&);
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

        // Frozen scene: still active and rendered, but skipped by entities::extract() and for_each().
        // Only an active scene can be frozen. deactivate_scene() and unload_scene() unfreeze it.
        void freeze_scene(scene_id id);
        void unfreeze_scene(scene_id id);
        void freeze_scenes(std::initializer_list<scene_id> ids);
        void unfreeze_scenes(std::initializer_list<scene_id> ids);

        // Freezes every scene that is active right now except the kept ones. Scenes activated later are not frozen.
        void freeze_all_except(scene_id keep);
        void freeze_all_except(std::initializer_list<scene_id> keep);

        void unfreeze_all();
        bool is_scene_frozen(scene_id id) const;

        bool has_scene(scene_id id) const;
        scene_status status(scene_id id) const;
        void set_scene_layer(scene_id id, int layer);
        std::int8_t scene_layer(scene_id id) const;
        // Returns a snapshot of the scene's entities (active + inactive)
        std::vector<entity> scene_entities(scene_id id) const;

        template <typename... Comps>
        entity create_entity(scene_id id, components::transform initial_transform = components::transform{}, Comps&&... comps);

        // Systems run once per frame, in registration order. Must be registered before play() starts.
        template <typename... Components, typename Func>
        void register_system(Func&& func);

        void play();
    };

    template <typename... Components, typename Func>
    void game::register_system(Func&& func)
    {
        GAMECOE_ASSERT_GUARD(m_window.has_value(), "game::register_system(): called on a moved-from game");
        add_system(make_system<Components...>(std::forward<Func>(func)));
    }

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
        GAMECOE_ASSERT_GUARD(meta != nullptr, "game::create_entity(): scene is not registered", entity::invalid());
        // entities can only be created into an active scene.
        GAMECOE_ASSERT_GUARD(meta->status == scene_status::active, "game::create_entity(): scene is not active", entity::invalid());

        entity e = m_entities.create(std::move(initial_transform), id);
        (m_entities.add_component<std::decay_t<Comps>>(e, std::forward<Comps>(comps)), ...);
        return e;
    }
} // namespace gamecoe
