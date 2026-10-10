#pragma once

#include <gamecoe/entity/entities.hpp>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace gamecoe
{
    class game;

    struct system_entry
    {
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
        void add_access(system_entry &entry)
        {
            const std::uint32_t id = entities::component_id<std::remove_cvref_t<C>>();
            if constexpr (std::is_const_v<std::remove_reference_t<C>>)
                entry.reads.push_back(id);
            else
                entry.writes.push_back(id);
        }
    } // namespace detail

    // const components go in reads, non-const in writes. Ids are entities::component_id,
    // the same ones that index the component pools.
    template <typename... Components, typename Func>
    [[nodiscard]] system_entry make_system(Func&& func)
    {
        static_assert(detail::unique_types_v<std::remove_cvref_t<Components>...>,
            "make_system(): duplicate component type (T, const T, T& and const T& count as the same component)");
        static_assert(std::is_invocable_v<std::decay_t<Func>&, game&>,
            "make_system(): func must be callable as func(game&)");

        system_entry entry{std::function<void(game&)>(std::forward<Func>(func)), {}, {}};
        (detail::add_access<Components>(entry), ...);
        return entry;
    }
} // namespace gamecoe
