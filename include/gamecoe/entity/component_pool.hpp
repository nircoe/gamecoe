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
            return index && m_entities.is_active(index.value());
        }

        // Every pool an entity belongs to must agree on its active state - see entities::activate()/
        // deactivate(), which is the only caller that maintains that across all pools.
        void deactivate(entity e)
        {
            auto index = m_entities.index(e);
            if (!index) return;

            std::uint32_t i = index.value();
            if (auto partner = m_entities.deactivate_at(i); partner && partner.value() != i)
                swap_components(i, partner.value());
        }

        void activate(entity e)
        {
            auto index = m_entities.index(e);
            if (!index) return;

            std::uint32_t i = index.value();
            if (auto partner = m_entities.activate_at(i); partner && partner.value() != i)
                swap_components(i, partner.value());
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

        ~component_pool() override = default;

        void remove(entity e) override
        {
            auto index = m_entities.index(e);
            if (!index) return;

            deactivate(e);   // No-op if e is already inactive. Otherwise moves e (and its component) to the first-inactive slot.

            std::uint32_t i    = m_entities.index(e).value();   // re-resolve: deactivate() may have moved it
            std::uint32_t last = static_cast<std::uint32_t>(m_components.size()) - 1;

            m_entities.erase_at(i);   // both endpoints are now inactive, so this is a pure swap-with-back + pop

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
                return m_components[m_entities.index(e).value()];   // no-op - active is ignored, existing value is untouched
            }

            const std::uint32_t back_index   = static_cast<std::uint32_t>(m_components.size());
            const std::uint32_t target_index = static_cast<std::uint32_t>(m_entities.active_size());

            m_components.emplace_back(std::forward<Args>(args)...);   // emplace first: if it throws, m_entities is untouched
            m_entities.insert(e, active);

            // An inactive entry stays at the back, where both arrays already agree. An active one is
            // swapped down into the boundary slot by insert(), so mirror that swap here to keep
            // m_components[i] paired with the entity at dense index i.
            if (!active) return m_components[back_index];

            if (target_index != back_index) std::swap(m_components[target_index], m_components[back_index]);

            return m_components[target_index];
        }

        // Asserts and returns T& (not nullable), callers here already checked contains().
        // entities::get_component<T>() returns a nullable pointer instead since its callers don't always know.
        // try_get() below is for callers in neither position: they haven't already checked contains(),
        // but want the single lookup either way instead of a separate contains() + get() pair.
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

        // Single lookup, nullable - for callers that don't already know the entity is present
        // (unlike get(), which asserts and is for callers that already checked contains()).
        T* try_get(entity e)
        {
            auto index = m_entities.index(e);
            return index ? &m_components[index.value()] : nullptr;
        }

        const T* try_get(entity e) const
        {
            auto index = m_entities.index(e);
            return index ? &m_components[index.value()] : nullptr;
        }

        // Active partition only - a deactivated entity (e.g. one in a paused scene) is skipped,
        // same bound extract<>() uses. Activating/deactivating an entity from inside the callback
        // reorders the dense array mid-loop, same hazard as extraction (see extraction.hpp).
        template<typename Func>
        void for_each(Func &&func)
        {
            auto size = active_size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        template<typename Func>
        void for_each(Func &&func) const
        {
            auto size = active_size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        // Full scan over both partitions, unlike for_each(). Mutating any pool (activate/deactivate/add/remove)
        // during iteration invalidates the cached bound (see begin()/end() comment).
        template<typename Func>
        void for_each_all(Func &&func)
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        template<typename Func>
        void for_each_all(Func &&func) const
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(m_entities.get_entity_at_index(i), m_components[i]);
        }

        // Iterators invalidated by add/remove (dense array reallocation). Span both partitions
        // (active and inactive) - for active-only iteration use for_each()/extract() instead.
        T *begin() noexcept { return m_components.data(); }
        T *end() noexcept { return m_components.data() + m_components.size(); }
        const T *begin() const noexcept { return m_components.data(); }
        const T *end() const noexcept { return m_components.data() + m_components.size(); }
        const T *cbegin() const noexcept { return begin(); }
        const T *cend() const noexcept { return end(); }
    };
} // namespace gamecoe
