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

    struct scene_metadata
    {
        command_buffer       pending;
        std::vector<entity>  paused_active;   // snapshot of the active entities before the scene was deactivated
        scene_builder        builder = nullptr;
        std::int8_t          layer   = 0;
        scene_status         status  = scene_status::unloaded;
    };

    class game
    {
        gamecoe::entities m_entities;
        std::optional<window> m_window;
        std::flat_map<scene_id, scene_metadata> m_scenes;
        std::vector<scene_id> m_active_scenes;   // sorted by layer
        Color m_background_color;

        game(window &&main_window, const Color &background_color);
        // Returns a snapshot of the scene's entities
        std::vector<entity> collect_scene_entities(scene_id id);

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
} // namespace gamecoe
