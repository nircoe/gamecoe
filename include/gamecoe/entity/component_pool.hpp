#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/sparse_set.hpp>
#include <vector>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    class basic_component_pool
    {
    protected:
        sparse_set m_entities;

    private:
        // Base always reserves the sparse_set, derived always reserves its dense array through this override,
        // so neither one can be forgotten.
        virtual void do_reserve(std::size_t capacity) = 0;

        // Derived mirrors the dense-array swap sparse_set performs internally onto its own parallel array.
        virtual void swap_components(std::uint32_t a, std::uint32_t b) = 0;

    public:
        virtual ~basic_component_pool() = default;
        virtual void remove(entity e) = 0;
        virtual void clear() = 0;

        bool contains(entity e) const noexcept { return m_entities.contains(e); }
        std::size_t size() const noexcept { return m_entities.size(); }
        bool empty() const noexcept { return m_entities.empty(); }
        void reserve(std::size_t capacity) { m_entities.reserve(capacity); do_reserve(capacity); }

        std::size_t active_size() const noexcept { return m_entities.active_size(); }

        bool is_active(entity e) const noexcept
        {
            auto index = m_entities.index(e);
            return index && m_entities.is_active(index.value());   // absent entity -> false, mirrors contains()
        }

        // Every pool an entity belongs to must agree on its active state - see entities::activate()/
        // deactivate(), which is the only caller that maintains that across all pools.
        void deactivate(entity e)
        {
            auto index = m_entities.index(e);
            if (!index) return;

            std::uint32_t i = index.value();
            if (!m_entities.is_active(i)) return;                                  // already inactive

            // active_size() and i must be captured before deactivate_at(), since that call changes active_count.
            std::uint32_t last_active = static_cast<std::uint32_t>(m_entities.active_size()) - 1;
            m_entities.deactivate_at(i);                                           // moves e past the active/inactive boundary
            if (i != last_active) swap_components(i, last_active);
        }

        // See deactivate() above for the shared active-state invariant this mirrors.
        void activate(entity e)
        {
            auto index = m_entities.index(e);
            if (!index) return;

            std::uint32_t i = index.value();
            if (m_entities.is_active(i)) return;                                   // already active

            // active_size() and i must be captured before activate_at(), since that call changes active_count.
            std::uint32_t first_inactive = static_cast<std::uint32_t>(m_entities.active_size());
            m_entities.activate_at(i);                                             // moves e past the active/inactive boundary
            if (i != first_inactive) swap_components(i, first_inactive);
        }

        entity get_entity_at_index(std::size_t index) const noexcept { return m_entities.get_entity_at_index(index); }
    };

    template <typename T>
    class component_pool : public basic_component_pool
    {
        std::vector<T> m_components;

        void do_reserve(std::size_t capacity) override
        {
            m_components.reserve(capacity);
        }

        void swap_components(std::uint32_t a, std::uint32_t b) override { std::swap(m_components[a], m_components[b]); }

    public:
        component_pool() noexcept = default;
        component_pool(const component_pool&) = delete;
        component_pool& operator=(const component_pool&) = delete;
        component_pool(component_pool&&) noexcept = default;
        component_pool& operator=(component_pool&&) noexcept = default;

        ~component_pool() override = default;

        void remove(entity e) override
        {
            auto index = m_entities.index(e);
            if (!index) return;

            std::uint32_t i      = index.value();
            std::uint32_t last   = static_cast<std::uint32_t>(m_components.size()) - 1;
            std::uint32_t active = static_cast<std::uint32_t>(m_entities.active_size());

            m_entities.erase_at(i);   // performs the boundary swap (if any) + swap-with-back + active_count fixup

            // Mirror erase_at()'s first swap: route the removed entry out through the active boundary.
            if (i < active)
            {
                std::uint32_t last_active = active - 1;
                if (i != last_active) m_components[i] = std::move(m_components[last_active]);   // guard: no self-move
                i = last_active;
            }

            // Mirror erase_at()'s swap-with-back.
            if (i != last) m_components[i] = std::move(m_components[last]);
            m_components.pop_back();
        }

        void clear() override
        {
            m_entities.clear();
            m_components.clear();
        }

        template <typename... Args>
        T& add(entity e, bool active, Args&&... args)
        {
            if (contains(e))
            {
                GAMECOE_ASSERT_LOG(false, "component_pool::add(): entity already has this component");
                // Release has no assert, so we overwrite instead of silently discarding the caller's new value.
                T &existing = m_components[m_entities.index(e).value()];
                existing = T(std::forward<Args>(args)...);
                return existing;   // active untouched - an existing entity's state isn't this call's business
            }

            // sparse_set::insert() appends then swaps the new entity down into the active partition,
            // so the component can't just be left at the back - mirror the same swap.
            const std::uint32_t back_index   = static_cast<std::uint32_t>(m_components.size());
            const std::uint32_t target_index = static_cast<std::uint32_t>(m_entities.active_size());

            m_components.emplace_back(std::forward<Args>(args)...);   // emplace first: if it throws, m_entities is untouched
            m_entities.insert(e);

            if (target_index != back_index) std::swap(m_components[target_index], m_components[back_index]);

            if (!active) deactivate(e);   // may relocate the entry again - re-resolve below either way

            return get(e);
        }

        // Asserts and returns T& (not nullable), callers here already checked contains().
        // entities::get_component<T>() returns a nullable pointer instead since its callers don't always know.
        T& get(entity e)
        {
            auto index = m_entities.index(e);
            GAMECOE_ASSERT_LOG(index, "component_pool::get(): entity does not exist in the pool");

            return m_components[index.value()];
        }

        const T& get(entity e) const
        {
            auto index = m_entities.index(e);
            GAMECOE_ASSERT_LOG(index, "component_pool::get(): entity does not exist in the pool");

            return m_components[index.value()];
        }

        template<typename Func>
        void for_each(Func &&func)
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        template<typename Func>
        void for_each(Func &&func) const
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        // Iterators invalidated by add/remove (dense array reallocation)
        T *begin() noexcept { return m_components.data(); }
        T *end() noexcept { return m_components.data() + m_components.size(); }
        const T *begin() const noexcept { return m_components.data(); }
        const T *end() const noexcept { return m_components.data() + m_components.size(); }
        const T *cbegin() const noexcept { return begin(); }
        const T *cend() const noexcept { return end(); }
    };
} // namespace gamecoe
