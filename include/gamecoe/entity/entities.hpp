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
#include <gamecoe/utils/error_handler.hpp>

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
        // Plain counters, not atomics. Atomics would imply a thread-safety guarantee
        // this ECS doesn't have yet, revisit once the threading model is decided.
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

        // Creates a new pool if not exists (lazy auto-registration).
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

        // May return a recycled id.
        entity create();

        // No-op if e is already invalid.
        void destroy(entity e);

        void clear();

        bool valid(entity e) const;

        // Also reserves capacity in every existing component pool, not just entity bookkeeping.
        void reserve(std::size_t capacity);

        std::size_t size() const;

        // Entity must already be valid, asserted.
        template <typename T, typename... Args>
        T& add_component(entity e, Args&&... args)
        {
            static_assert(!std::is_same_v<T, components::transform>,
                "entities::add_component(): transform is mandatory, added automatically by create()");
            static_assert(!hierarchy_component<T>,
                "entities::add_component(): hierarchy components are managed - use entities::set_parent() instead");

            GAMECOE_ASSERT_LOG(valid(e), "entities::add_component(): entity is not valid");

            auto pool = get_pool<T>();
            return pool->add(e, true, std::forward<Args>(args)...);
        }

        // Safe on an invalid entity, returns false rather than asserting.
        template <typename T>
        bool has_component(entity e) const
        {
            if (!valid(e)) return false;

            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return false;

            return m_pools[comp_id]->contains(e);
        }

        // No-op if e doesn't have T, or e is invalid.
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

        // Pointer may be invalidated by any add_component call (pool reallocation).
        template <typename T>
        T* get_component(entity e)
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<component_pool<T>*>(m_pools[component_id<T>()].get());
            return &(pool->get(e));
        }

        // Pointer may be invalidated by any add_component call (pool reallocation).
        template <typename T>
        const T* get_component(entity e) const
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<const component_pool<T>*>(m_pools[component_id<T>()].get());
            return &(pool->get(e));
        }

        // Transform always exists.
        components::transform& transform(entity e);

        // Transform always exists.
        const components::transform& transform(entity e) const;

        // Updates both sides.
        void set_parent(entity child, entity parent);

        // Updates both sides.
        void remove_parent(entity child);

        // Updates both sides.
        void remove_children(entity parent);

        template <typename T, typename Func>
        void for_each(Func &&func)
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return;

            auto pool = static_cast<component_pool<T>*>(m_pools[comp_id].get());
            pool->for_each(std::forward<Func>(func));
        }

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
