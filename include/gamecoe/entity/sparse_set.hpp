#pragma once

#include <gamecoe/entity/entity.hpp>
#include <cstdint>
#include <vector>
#include <memory>
#include <array>

namespace gamecoe
{
    class sparse_set
    {
        static constexpr std::uint16_t PAGE_SIZE = 1024;

        using page_type = std::array<std::uint32_t, PAGE_SIZE>;
        using page_ptr_type = std::unique_ptr<page_type>;

        std::vector<page_ptr_type> m_sparse;
        std::vector<entity> m_dense;

        std::uint16_t index_in_page(const entity &e) const { return e.id() % PAGE_SIZE; }
        std::uint32_t page_index(const entity &e) const { return e.id() / PAGE_SIZE; }
    
    public:
        sparse_set() = default;
        sparse_set(const sparse_set&) = delete;
        sparse_set& operator=(const sparse_set&) = delete;
        sparse_set(sparse_set&&) = default;
        sparse_set& operator=(sparse_set&&) = default;

        ~sparse_set() = default;

        void insert(const entity &e)
        {
            auto page_i = page_index(e);
            if (page_i >= m_sparse.size()) m_sparse.resize(page_i + 1);

            auto &page = m_sparse[page_i];
            if (!page) page = std::make_unique<page_type>();
            
            auto &dense_index = (*page)[index_in_page(e)];
            if (m_dense.size() > dense_index && m_dense[dense_index] == e) return;

            dense_index = m_dense.size();
            m_dense.push_back(e);
        }

        void erase(const entity &e)
        {
            auto page_i = page_index(e);
            if (page_i >= m_sparse.size()) return;

            auto &page = m_sparse[page_i];
            if (!page) return;

            auto dense_index = (*page)[index_in_page(e)];
            if (m_dense.size() <= dense_index || m_dense[dense_index] != e) return;

            auto swapped = m_dense.back();
            m_dense[dense_index] = swapped;
            m_dense.pop_back();

            if (swapped == e) return;
            
            auto &swapped_page = m_sparse[page_index(swapped)];
            (*swapped_page)[index_in_page(swapped)] = dense_index;
        }

        std::uint32_t index(const entity &e) const
        {
            auto page_i = page_index(e);
            assert((page_i < m_sparse.size()) && "sparse_set::index(): entity not found");

            const auto &page = m_sparse[page_i];
            assert(page && "sparse_set::index(): entity not found");

            auto dense_index = (*page)[index_in_page(e)];
            assert((dense_index < m_dense.size() && m_dense[dense_index] == e) && "sparse_set::index(): entity not found");

            return dense_index;
        }

        bool contains(const entity &e) const
        { 
            auto page_i = page_index(e);
            if (page_i >= m_sparse.size()) return false;

            const auto &page = m_sparse[page_i];
            if (!page) return false;

            auto dense_index = (*page)[index_in_page(e)];
            return dense_index < m_dense.size() && m_dense[dense_index] == e;
        }

        void clear() { m_sparse.clear(); m_dense.clear(); }
        std::size_t size() const { return m_dense.size(); }
        bool empty() const { return m_dense.empty(); }

        entity *begin() { return m_dense.data(); }
        entity *end() { return m_dense.data() + m_dense.size(); }
        const entity *begin() const { return m_dense.data(); }
        const entity *end() const { return m_dense.data() + m_dense.size(); }
        const entity *cbegin() const { return begin(); }
        const entity *cend() const { return end(); }
    };
} // namespace gamecoe
