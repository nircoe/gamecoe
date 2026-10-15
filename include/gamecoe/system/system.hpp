#pragma once

#include <gamecoe/entity/entities.hpp>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace gamecoe
{
    class game;

    namespace systems
    {
        struct entry
        {
            std::string name;
            std::function<void(game&)> func;
            std::vector<std::uint32_t> reads;
            std::vector<std::uint32_t> writes;
        };

        namespace detail
        {
            template <typename...>
            inline constexpr bool unique_types_v = true;

            template <typename T, typename... Rest>
            inline constexpr bool unique_types_v<T, Rest...> =
                (!std::is_same_v<T, Rest> && ...) && unique_types_v<Rest...>;

            template <typename C>
            void add_access(entry &target)
            {
                const std::uint32_t id = entities::component_id<std::remove_cvref_t<C>>();
                if constexpr (std::is_const_v<std::remove_reference_t<C>>)
                    target.reads.push_back(id);
                else
                    target.writes.push_back(id);
            }
        } // namespace detail

        // const components go in reads, non-const in writes. Ids are entities::component_id,
        // the same ones that index the component pools.
        template <typename... Components, typename Func>
        [[nodiscard]] entry create(std::string name, Func&& func)
        {
            static_assert(detail::unique_types_v<std::remove_cvref_t<Components>...>,
                "systems::create(): duplicate component type "
                "(T, const T, T& and const T& count as the same component)");
            static_assert(std::is_invocable_v<std::decay_t<Func>&, game&>,
                "systems::create(): func must be callable as func(game&)");

            entry made{std::move(name), std::function<void(game&)>(std::forward<Func>(func)), {}, {}};
            (detail::add_access<Components>(made), ...);
            return made;
        }
    } // namespace systems
} // namespace gamecoe
