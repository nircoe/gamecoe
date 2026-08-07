#pragma once

#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <gamecoe/core/scene_id.hpp>
#include <functional>
#include <optional>
#include <vector>
#include <cstdint>
#include <type_traits>
#include <cassert>

namespace gamecoe
{
    // Defers entity creation until flush() for building a scene async on a different thread. 
    // Placeholders from spawn() are invalidated by their buffer's own flush() - don't reuse them after.
    class command_buffer
    {
    public:
        struct placeholder
        {
        private:
            std::uint32_t m_index;
            explicit constexpr placeholder(std::uint32_t index) noexcept : m_index(index) { }
            friend class command_buffer;
        };

        class resolver
        {
            explicit resolver(const std::vector<entity>& created) noexcept : m_created(created) { }
            const std::vector<entity>& m_created;
            friend class command_buffer;

        public:
            entity resolve(placeholder p) const;
        };

        // Queues an entity with the given transform (default if omitted). No real entity until flush().
        placeholder spawn(components::transform t = components::transform{});

        // Queues a component add-or-assign, applied at flush().
        template <typename T>
        requires (!std::invocable<T, const resolver&>)
        void add(placeholder p, T value)
        {
            static_assert(!std::is_same_v<T, components::parent> && !std::is_same_v<T, components::children>,
                "command_buffer::add(): hierarchy components are managed - use command_buffer::set_parent() instead");

            m_commands.emplace_back([p, value = std::move(value)](entities& ents, const resolver& r) mutable
            {
                entity e = r.resolve(p);
                if (T* c = ents.get_component<T>(e)) *c = std::move(value);
                else ents.add_component<T>(e, std::move(value));
            });
        }

        // Creates all queued entities, runs all queued commands, then clears. Stamps scene_tag on every created entity if `scene` is set.
        void flush(entities& ents, std::optional<scene_id> scene = std::nullopt);

        std::size_t spawn_count() const noexcept { return m_spawn_transforms.size(); }
        bool empty() const noexcept { return m_spawn_transforms.empty() && m_commands.empty(); }
        void clear();

    private:
        using command = std::function<void(entities&, const resolver&)>;
        std::vector<components::transform> m_spawn_transforms;
        std::vector<command> m_commands;
    };
} // namespace gamecoe
