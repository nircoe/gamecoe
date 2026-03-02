#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/sparse_set.hpp>
#include <vector>
#include <cassert>

namespace gamecoe
{
    class basic_component_pool
    {
    protected:
        sparse_set m_entities;
        
    private:
        virtual void do_reserve(std::size_t capacity) = 0;

    public:
        virtual ~basic_component_pool() = default;
        virtual void remove(entity e) = 0;
        virtual void clear() = 0;

        bool contains(entity e) const noexcept { return m_entities.contains(e); }
        std::size_t size() const noexcept { return m_entities.size(); }
        bool empty() const noexcept { return m_entities.empty(); }
        void reserve(std::size_t capacity) { m_entities.reserve(capacity); do_reserve(capacity); }

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
            if(!index) return;

            std::uint32_t i = index.value();
            m_entities.erase_at(i);

            if (i != m_components.size() - 1)
                m_components[i] = std::move(m_components.back());

            m_components.pop_back();
        }

        void clear() override
        {
            m_entities.clear();
            m_components.clear();
        }

        template <typename... Args>
        T& add(entity e, Args&&... args)
        {
            if(contains(e)) 
            {
                assert(false && "component_pool::add(): entity already has this component");
                return m_components[m_entities.index(e).value()];
            }

            m_entities.insert(e);
            m_components.emplace_back(std::forward<Args>(args)...);

            return m_components.back();
        }

        T& get(entity e)
        {
            auto index = m_entities.index(e);
            assert(index && "component_pool::get(): entity does not exist in the pool");

            return m_components[index.value()];
        }

        const T& get(entity e) const
        {
            auto index = m_entities.index(e);
            assert(index && "component_pool::get(): entity does not exist in the pool");

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
