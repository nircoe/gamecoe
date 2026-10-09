#pragma once

#include <algorithm>
#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/core/scene_id.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace gamecoe
{
    // Iterates the smallest pool's active partition and checks membership in the rest via
    // contains(), minimizing total contains() calls across the whole extraction. Entities in a
    // frozen scene are skipped unless the extraction came from extract_with_frozen(). Mutating any
    // pool (activate/deactivate/add/remove) during iteration invalidates the cached bound.
    // Don't keep an extraction across a move or clear of the entities it came from.
    template <typename... Components>
    class extraction
    {
        std::tuple<component_pool<std::remove_const_t<Components>>*...> m_pools;
        const component_pool<components::scene_tag> *m_scene_tags;
        const std::vector<scene_id> *m_frozen_scenes;
        std::size_t m_smallest_pool_index;
        std::size_t m_smallest_pool_size;

    public:
        class iterator
        {
            const extraction* m_extracted;
            std::size_t m_index;

            // std::get<Is> needs a compile-time index, but which pool is smallest is only known
            // at runtime (m_smallest_pool_index), so this loops over indices via a fold expression.
            // has_all_components() below can stay type-keyed (a plain fold over Components) since
            // it doesn't need to single out one specific pool.
            template <std::size_t... Is>
            entity get_current_entity(std::index_sequence<Is...>) const
            {
                if (m_index >= m_extracted->m_smallest_pool_size) return entity::invalid();

                entity e = entity::invalid();

                (void)((Is == m_extracted->m_smallest_pool_index ?
                    (e = std::get<Is>(m_extracted->m_pools)->get_entity_at_index(m_index), true) : false)
                || ...);

                return e;
            }

            bool has_all_components(entity e) const
            {
                return (std::get<component_pool<std::remove_const_t<Components>>*>(m_extracted->m_pools)->contains(e) && ...);
            }

            // Reads the frozen list on every call, so a scene frozen mid-iteration is skipped from the next entity on.
            bool in_frozen_scene(entity e) const
            {
                const std::vector<scene_id> *frozen = m_extracted->m_frozen_scenes;
                if (!frozen || frozen->empty() || !m_extracted->m_scene_tags) return false;

                const components::scene_tag *tag = m_extracted->m_scene_tags->try_get(e);
                return tag && std::find(frozen->begin(), frozen->end(), tag->id) != frozen->end();
            }

            void next()
            {
                while (m_index < m_extracted->m_smallest_pool_size)
                {
                    entity e = get_current_entity(std::index_sequence_for<Components...>{});

                    if (has_all_components(e) && !in_frozen_scene(e)) return;
                    ++m_index; // entity e is not in all pools, check the next one
                }
            }

            template <std::size_t... Is>
            std::tuple<entity, Components&...> get_current_tuple(std::index_sequence<Is...>) const
            {
                entity e = get_current_entity(std::index_sequence_for<Components...>{});

                return std::tuple<entity, Components&...> {
                    e,
                    *std::get<Is>(m_extracted->m_pools)->try_get(e)...
                };
            }

        public:
            iterator(const extraction* e, std::size_t index = 0) : m_extracted(e), m_index(index) { next(); }

            std::tuple<entity, Components&...> operator*() const
            {
                return get_current_tuple(std::index_sequence_for<Components...>{});
            }

            iterator& operator++() { ++m_index; next(); return *this; }
            iterator operator++(int) { iterator tmp = *this; ++m_index; next(); return tmp; }

            bool operator==(const iterator &other) const { return m_index == other.m_index; }
            bool operator!=(const iterator &other) const { return m_index != other.m_index; }
        };

        explicit extraction(component_pool<std::remove_const_t<Components>>*... pools,
                            const component_pool<components::scene_tag> *scene_tags,
                            const std::vector<scene_id> *frozen_scenes)
            : m_pools(pools...), m_scene_tags(scene_tags), m_frozen_scenes(frozen_scenes)
        {
            std::size_t sizes[] = { (pools ? pools->active_size() : std::size_t{0})... };

            m_smallest_pool_index = 0;
            m_smallest_pool_size = sizes[0];

            for(std::size_t i = 1; i < sizeof...(Components); ++i)
            {
                if (sizes[i] < m_smallest_pool_size)
                {
                    m_smallest_pool_size = sizes[i];
                    m_smallest_pool_index = i;
                }
            }
        }

        iterator begin() const { return iterator(this); }
        iterator end() const { return iterator(this, m_smallest_pool_size); }
        const iterator cbegin() const { return begin(); }
        const iterator cend() const { return end(); }
    };
} // namespace gamecoe
