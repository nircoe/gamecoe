#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/sparse_set.hpp>
#include <vector>
#include <optional>
#include <functional>
#include <cassert>

namespace gamecoe
{
    template <typename T>
    class component_pool : private sparse_set
    {
        std::vector<T> m_components;

    public:
        using sparse_set::contains;
        using sparse_set::size;
        using sparse_set::empty;

        component_pool() noexcept = default;
        component_pool(const component_pool&) = delete;
        component_pool& operator=(const component_pool&) = delete;
        component_pool(component_pool&&) noexcept = default;
        component_pool& operator=(component_pool&&) noexcept = default;

        ~component_pool() = default;

        void reserve(std::size_t capacity)
        {
            sparse_set::reserve(capacity);
            m_components.reserve(capacity);
        }

        template <typename... Args>
        T& add(const entity &e, Args&&... args)
        {
            if(contains(e)) 
            {
                assert(false && "component_pool::add(): entity already has this component");
                return m_components[sparse_set::index(e).value()];
            }

            sparse_set::insert(e);
            m_components.emplace_back(std::forward<Args>(args)...);

            return m_components.back();
        }

        void remove(const entity &e)
        {
            auto index = sparse_set::index(e);
            if(!index) return;

            sparse_set::erase(e);

            if (index.value() < m_components.size() - 1)
                m_components[index.value()] = std::move(m_components.back());

            m_components.pop_back();
        }

        std::optional<std::reference_wrapper<T>> get(const entity &e)
        {
            auto index = sparse_set::index(e);
            if(!index) return std::nullopt;

            return std::ref(m_components[index.value()]);
        }

        std::optional<std::reference_wrapper<const T>> get(const entity &e) const
        {
            auto index = sparse_set::index(e);
            if(!index) return std::nullopt;

            return std::cref(m_components[index.value()]);
        }

        template<typename Func>
        void for_each(Func &&func)
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(sparse_set::get_entity_at_index(i), m_components[i]);
        }

        template<typename Func>
        void for_each(Func &&func) const
        {
            auto size = m_components.size();
            for(std::size_t i = 0; i < size; ++i)
                func(sparse_set::get_entity_at_index(i), m_components[i]);
        }

        void clear()
        {
            sparse_set::clear();
            m_components.clear();
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
