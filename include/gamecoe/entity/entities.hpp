#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <utility>
#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>
#include <cassert>

namespace gamecoe
{
    class entities
    {
        static std::atomic<std::uint32_t> s_component_id;

        std::vector<std::unique_ptr<basic_component_pool>> m_pools;
        std::vector<std::uint32_t> m_recycle_ids;
        std::vector<std::uint16_t> m_generations;

        std::atomic<std::uint32_t> m_current_entity_id;

        // Returns static and unique id for component T
        template <typename T>
        static std::uint32_t component_id()
        {
            static std::uint32_t s_componentT_id = s_component_id++;
            return s_componentT_id;
        }

        // Returns a pointer to the realtime type component_pool<T> of the T component, creates a new pool if not exists
        template <typename T>
        component_pool<T>* get_pool()
        {
            std::uint32_t comp_id = component_id<T>();

            if (comp_id >= m_pools.size())  m_pools.resize(comp_id + 1);
            if (!m_pools[comp_id])          m_pools[comp_id] = std::make_unique<component_pool<T>>();

            return static_cast<component_pool<T>*>(m_pools[comp_id].get());
        }

    public:
        entities() = default;
        entities(const entities&) = delete;
        entities(entities&&) = delete;
        entities &operator=(const entities&) = delete;
        entities &operator=(entities&&) = delete;

        ~entities() = default;

        // Creates an entity (may be recycled id)
        entity create();

        // Destroy an entity
        void destroy(entity e);

        // Destroys all entities
        void clear();

        // Returns if the entity handle given is valid
        bool valid(entity e) const;

        // Reserves capacity for a number of entities
        void reserve(std::size_t capacity);

        // Returns the number of alive entities
        std::size_t size() const;

        // Emplaces a new T component to entity e
        template <typename T, typename... Args>
        T& add_component(entity e, Args&&... args)
        {
            assert(valid(e) && "entities::add_component(): entity is not valid");

            auto pool = get_pool<T>();
            return pool->add(e, std::forward<Args>(args)...);
        }

        // Returns if entity e has a component T
        template <typename T>
        bool has_component(entity e) const
        {
            if (!valid(e)) return false;

            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return false;

            return m_pools[comp_id]->contains(e);
        }

        // Removes component T from entity e
        template <typename T>
        void remove_component(entity e)
        {
            if (!has_component<T>(e)) return;

            m_pools[component_id<T>()]->remove(e);
        }

        // Returns a pointer to component T of entity e (`nullptr` if entity doesn't have T component),
        // also note that the pointer may invalidated by any `add_component` call
        template <typename T>
        T* get_component(entity e)
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<component_pool<T>*>(m_pools[component_id<T>()].get());
            return &(pool->get(e));
        }

        // Returns a pointer to component T of entity e (`nullptr` if entity doesn't have T component),
        // also note that the pointer may invalidated by any `add_component` call
        template <typename T>
        const T* get_component(entity e) const
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<const component_pool<T>*>(m_pools[component_id<T>()].get());
            return &(pool->get(e));
        }

        // Iterates over all entities and their components and run func() on each of them
        template<typename T, typename Func>
        void for_each(Func &&func)
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return;

            auto pool = static_cast<component_pool<T>*>(m_pools[comp_id].get());
            pool->for_each(std::forward<Func>(func));
        }

        // Iterates over all entities and their components and run func() on each of them
        template<typename T, typename Func>
        void for_each(Func &&func) const
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return;

            auto pool = static_cast<const component_pool<T>*>(m_pools[comp_id].get());
            pool->for_each(std::forward<Func>(func));
        }
    };
} // namespace gamecoe
