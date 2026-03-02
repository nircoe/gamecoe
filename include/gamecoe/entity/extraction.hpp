#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gamecoe
{
    template <typename... Components>
    class extraction
    {
        std::tuple<component_pool<std::remove_const_t<Components>>*...> m_pools;
        std::size_t m_smallest_pool_index;
        std::size_t m_smallest_pool_size;

    public:
        class iterator
        {
            const extraction* m_extracted;
            std::size_t m_index;

            template <std::size_t... Is>
            entity get_current_entity(std::index_sequence<Is...>) const
            {
                if (m_index >= m_extracted->m_smallest_pool_size) return entity::invalid();

                entity e = entity::invalid();

                ((Is == m_extracted->m_smallest_pool_index ?
                    (e = std::get<Is>(m_extracted->m_pools)->get_entity_at_index(m_index), true) : false) 
                || ...);

                return e;
            }

            bool has_all_components(entity e) const
            {
                return (std::get<component_pool<std::remove_const_t<Components>>*>(m_extracted->m_pools)->contains(e) && ...);
            }

            void next()
            {
                while (m_index < m_extracted->m_smallest_pool_size)
                {
                    entity e = get_current_entity(std::index_sequence_for<Components...>{});

                    if (has_all_components(e)) return;
                    ++m_index; // entity e is not in all pools -> check the next one
                }
            }

            template <std::size_t... Is>
            std::tuple<entity, Components&...> get_current_tuple(std::index_sequence<Is...>) const
            {
                entity e = get_current_entity(std::index_sequence_for<Components...>{});

                return std::tuple<entity, Components&...> {
                    e,
                    std::get<Is>(m_extracted->m_pools)->get(e)...
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

        explicit extraction(component_pool<std::remove_const_t<Components>>*... pools) : m_pools(pools...)
        {
            std::size_t sizes[] = { pools->size()... };

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
