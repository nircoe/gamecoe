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
    public: // internal structs/classes
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
    
    private:
        // std::function requires copyable captured values (all current components qualify) -
        // revisit std::move_only_function once CI toolchains confirm C++23 library support
        using command = std::function<void(entities&, const resolver&)>;
        std::vector<components::transform> m_spawn_transforms;
        std::vector<command> m_commands;

        template <typename T>
        static void apply(entities& ents, entity e, T value)
        {
            static_assert(!hierarchy_component<T>,
                "command_buffer::add(): hierarchy components are managed - use command_buffer::set_parent() instead");

            if constexpr (std::is_same_v<T, components::transform>)
                ents.transform(e) = std::move(value);
            else
            {
                if (T* c = ents.get_component<T>(e)) *c = std::move(value);
                else ents.add_component<T>(e, std::move(value));
            }
        }
    
    public:
        // Queues an entity with the given transform (default if omitted). No real entity until flush().
        placeholder spawn(components::transform t = components::transform{});

        // Queues a component add-or-assign, applied at flush().
        template <typename T>
        requires (!std::invocable<T, const resolver&>)
        void add(placeholder p, T value)
        {
            m_commands.emplace_back([p, value = std::move(value)](entities& ents, const resolver& r) mutable
            {
                entity e = r.resolve(p);
                apply<T>(ents, e, std::move(value));
            });
        }

        // Queues a component computed from a resolver callable, applied at flush().
        template <typename Callable>
        requires std::invocable<Callable, const resolver&>
        void add(placeholder p, Callable&& fn)
        {
            using T = std::decay_t<std::invoke_result_t<Callable, const resolver&>>;
            static_assert(!std::is_void_v<T>, "command_buffer::add(): callable must return a component value");

            m_commands.emplace_back([p, fn = std::forward<Callable>(fn)](entities& ents, const resolver& r) mutable
            {
                entity e = r.resolve(p);
                apply<T>(ents, e, fn(r));
            });
        }

        // Queues a set_parent, applied at flush() via the live entities::set_parent().
        void set_parent(placeholder child, placeholder parent);

        // Creates all queued entities, runs all queued commands, then clears. Stamps scene_tag on every created entity if `scene` is set.
        void flush(entities& ents, std::optional<scene_id> scene = std::nullopt);

        std::size_t spawn_count() const noexcept { return m_spawn_transforms.size(); }
        bool empty() const noexcept { return m_spawn_transforms.empty() && m_commands.empty(); }
        void clear();
    };
} // namespace gamecoe
