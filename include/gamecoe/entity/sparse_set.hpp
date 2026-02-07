#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
#include <memory>
#include <array>

namespace gamecoe
{
    class sparse_set
    {
        static constexpr std::uint16_t PAGE_SIZE = 1024;
        static constexpr std::uint32_t TOMBSTONE = std::numeric_limits<std::uint32_t>::max();

        using page_type = std::array<std::uint32_t, PAGE_SIZE>;
        using page_ptr_type = std::unique_ptr<page_type>;

        std::vector<page_ptr_type> m_sparse;
        std::vector<entity> m_dense;

        std::uint16_t index_in_page(const entity &e) const noexcept { return e.id() % PAGE_SIZE; }
        std::uint32_t page_index(const entity &e) const noexcept { return e.id() / PAGE_SIZE; }

        std::uint32_t unpack_dense_index(std::uint32_t packed) const noexcept { return packed & entity::ID_MASK; }
        std::uint16_t unpack_generation(std::uint32_t packed) const noexcept { return packed >> entity::ID_BITS; }
        std::uint32_t pack_dense_index(std::uint32_t dense_index, std::uint16_t generation) const noexcept
        {
            return (generation << entity::ID_BITS) | dense_index;
        }

    protected:
        entity get_entity_at_index(std::size_t index) const noexcept
        {
            assert(index < m_dense.size() && "sparse_set::get_entity_at_index(): index out of bounds");
            return m_dense[index];
        }
        
    public:
        sparse_set() noexcept = default;
        sparse_set(const sparse_set&) = delete;
        sparse_set& operator=(const sparse_set&) = delete;
        sparse_set(sparse_set&&) noexcept = default;
        sparse_set& operator=(sparse_set&&) noexcept = default;

        ~sparse_set() = default;

        void reserve(std::size_t capacity) { m_dense.reserve(capacity); }

        void insert(const entity &e)
        {
            if (contains(e)) return;
            assert(m_dense.size() <= entity::MAX_ENTITIES && "sparse_set::insert(): max entities exceeded"); // sanity check

            auto page_i = page_index(e);
            if (page_i >= m_sparse.size()) 
                m_sparse.resize(page_i + 1);

            auto &page = m_sparse[page_i];
            if (!page) 
            {
                page = std::make_unique<page_type>();
                page->fill(TOMBSTONE);
            }
            
            (*page)[index_in_page(e)] = pack_dense_index(m_dense.size(), e.generation());
            m_dense.push_back(e);
        }

        void erase(const entity &e)
        {
            if (!contains(e)) return;

            auto &page = m_sparse[page_index(e)];
            auto dense_index = unpack_dense_index((*page)[index_in_page(e)]);

            auto swapped = m_dense.back();
            m_dense[dense_index] = swapped;
            m_dense.pop_back();

            (*page)[index_in_page(e)] = TOMBSTONE;

            if (swapped == e) return;
            
            auto &swapped_page = m_sparse[page_index(swapped)];
            (*swapped_page)[index_in_page(swapped)] = pack_dense_index(dense_index, swapped.generation());
        }

        std::optional<std::uint32_t> index(const entity &e) const
        {
            if (!contains(e)) return std::nullopt;

            return unpack_dense_index((*m_sparse[page_index(e)])[index_in_page(e)]);
        }

        bool contains(const entity &e) const noexcept
        { 
            auto page_i = page_index(e);

            return page_i < m_sparse.size() && m_sparse[page_i] && 
                   unpack_generation((*m_sparse[page_i])[index_in_page(e)]) == e.generation();
        }

        void clear() { m_sparse.clear(); m_dense.clear(); }
        std::size_t size() const noexcept { return m_dense.size(); }
        bool empty() const noexcept { return m_dense.empty(); }

        // Iterators invalidated by insert/erase (dense array reallocation)
        entity *begin() noexcept { return m_dense.data(); }
        entity *end() noexcept { return m_dense.data() + m_dense.size(); }
        const entity *begin() const noexcept { return m_dense.data(); }
        const entity *end() const noexcept { return m_dense.data() + m_dense.size(); }
        const entity *cbegin() const noexcept { return begin(); }
        const entity *cend() const noexcept { return end(); }
    };
} // namespace gamecoe
