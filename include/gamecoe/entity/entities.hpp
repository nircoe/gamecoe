#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <gamecoe/entity/extraction.hpp>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <cstdint>
#include <cassert>

namespace gamecoe
{
    namespace components
    {
        struct transform;
        struct parent;
        struct children;
    } // namespace components

    // True for the hierarchy-managed relationship components - see entities::set_parent()/remove_parent()
    template <typename T>
    concept hierarchy_component = std::is_same_v<T, components::parent> || std::is_same_v<T, components::children>;

    class entities
    {
        static std::uint32_t s_component_id;

        std::vector<std::unique_ptr<basic_component_pool>> m_pools;
        std::vector<std::uint32_t> m_recycle_ids;
        std::vector<std::uint16_t> m_generations;

        std::uint32_t m_current_entity_id{0};

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
            static_assert(!std::is_same_v<T, components::transform>,
                "entities::add_component(): transform is mandatory, added automatically by create()");
            static_assert(!hierarchy_component<T>,
                "entities::add_component(): hierarchy components are managed - use entities::set_parent() instead");

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
            static_assert(!std::is_same_v<T, components::transform>,
                "entities::remove_component(): transform is mandatory and cannot be removed");
            static_assert(!hierarchy_component<T>,
                "entities::remove_component(): hierarchy components are managed - use entities::remove_parent() instead");

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

        // Direct accessor for the mandatory transform component
        components::transform& transform(entity e);

        // Direct accessor for the mandatory transform component
        const components::transform& transform(entity e) const;

        // Sets child's parent, updating both sides.
        void set_parent(entity child, entity parent);

        // Removes child's parent link, updating both sides.
        void remove_parent(entity child);

        // Removes all of the entity's children, updating both sides.
        void remove_children(entity parent);

        // Iterates over all entities and their components and run func() on each of them
        template <typename T, typename Func>
        void for_each(Func &&func)
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return;

            auto pool = static_cast<component_pool<T>*>(m_pools[comp_id].get());
            pool->for_each(std::forward<Func>(func));
        }

        // Iterates over all entities and their components and run func() on each of them
        template <typename T, typename Func>
        void for_each(Func &&func) const
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return;

            auto pool = static_cast<const component_pool<T>*>(m_pools[comp_id].get());
            pool->for_each(std::forward<Func>(func));
        }

        template <typename... Components>
        extraction<Components...> extract()
        {
            return extraction<Components...>(get_pool<std::remove_const_t<Components>>()...);
        }

        template <typename... Components>
        extraction<std::add_const_t<Components>...> extract() const
        {
            return extraction<std::add_const_t<Components>...>(const_cast<entities*>(this)->get_pool<Components>()...);
        }
    };
} // namespace gamecoe
