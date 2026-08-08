#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
#include <memory>
#include <array>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    class sparse_set
    {
        // PAGE_SIZE = 1024 matches 4KB OS page (1024 * sizeof(uint32_t) == 4096),
        // balances memory waste vs pointer-chasing overhead.
        static constexpr std::uint16_t PAGE_SIZE = 1024;
        // Mirrors entity::invalid()'s all-1s pattern,
        // safe for the same reason (create() can never produce this exact value).
        static constexpr std::uint32_t TOMBSTONE = std::numeric_limits<std::uint32_t>::max();

        using page_type = std::array<std::uint32_t, PAGE_SIZE>;
        using page_ptr_type = std::unique_ptr<page_type>;

        std::vector<page_ptr_type> m_sparse;
        std::vector<entity> m_dense;

        // Generation packed alongside dense index in the sparse array itself,
        // so contains() never touches the dense array, pure cache win.
        std::uint16_t index_in_page(entity e) const noexcept { return e.id() % PAGE_SIZE; }
        std::uint32_t page_index(entity e) const noexcept { return e.id() / PAGE_SIZE; }

        std::uint32_t unpack_dense_index(std::uint32_t packed) const noexcept { return packed & entity::ID_MASK; }
        std::uint16_t unpack_generation(std::uint32_t packed) const noexcept { return packed >> entity::ID_BITS; }
        std::uint32_t pack_dense_index(std::uint32_t dense_index, std::uint16_t generation) const noexcept
        {
            return (generation << entity::ID_BITS) | dense_index;
        }
        
    public:
        sparse_set() noexcept = default;
        sparse_set(const sparse_set&) = delete;
        sparse_set& operator=(const sparse_set&) = delete;
        sparse_set(sparse_set&&) noexcept = default;
        sparse_set& operator=(sparse_set&&) noexcept = default;

        ~sparse_set() = default;

        void reserve(std::size_t capacity) { m_dense.reserve(capacity); }

        void insert(entity e)
        {
            if (contains(e)) return;
            GAMECOE_ASSERT_LOG(m_dense.size() <= entity::MAX_ENTITIES, "sparse_set::insert(): max entities exceeded");

            auto page_i = page_index(e);
            if (page_i >= m_sparse.size()) 
                m_sparse.resize(page_i + 1);

            auto &page = m_sparse[page_i];
            if (!page) 
            {
                page = std::make_unique<page_type>();
                page->fill(TOMBSTONE);
            }
            
            (*page)[index_in_page(e)] = pack_dense_index(static_cast<std::uint32_t>(m_dense.size()), e.generation());
            m_dense.push_back(e);
        }

        void erase(entity e)
        {
            if (!contains(e)) return;

            auto &page = m_sparse[page_index(e)];
            auto dense_index = unpack_dense_index((*page)[index_in_page(e)]);

            auto swapped = m_dense.back();
            m_dense[dense_index] = swapped;
            m_dense.pop_back();

            (*page)[index_in_page(e)] = TOMBSTONE;

            // When erasing the last dense element, swapped == e, and the sparse-page slot was already tombstoned above.
            // Touching it again via swapped's page would write stale data.
            if (swapped == e) return;
            
            auto &swapped_page = m_sparse[page_index(swapped)];
            (*swapped_page)[index_in_page(swapped)] = pack_dense_index(dense_index, swapped.generation());
        }

        // Mirrors erase() but skips entity-to-index lookup, used by component_pool::remove()
        // which has already resolved the index and would otherwise pay for redundant lookup.
        void erase_at(std::uint32_t dense_index)
        {
            if (dense_index >= m_dense.size()) return;

            entity e = m_dense[dense_index];
            auto page_i = page_index(e);
            auto i_in_page = index_in_page(e);

            entity swapped = m_dense.back();
            m_dense[dense_index] = swapped;
            m_dense.pop_back();

            (*m_sparse[page_i])[i_in_page] = TOMBSTONE;

            if (swapped == e) return;

            auto &swapped_page = m_sparse[page_index(swapped)];
            (*swapped_page)[index_in_page(swapped)] = pack_dense_index(dense_index, swapped.generation());
        }

        std::optional<std::uint32_t> index(entity e) const
        {
            if (!contains(e)) return std::nullopt;

            return unpack_dense_index((*m_sparse[page_index(e)])[index_in_page(e)]);
        }

        bool contains(entity e) const noexcept
        { 
            auto page_i = page_index(e);

            return page_i < m_sparse.size() && m_sparse[page_i] && 
                   unpack_generation((*m_sparse[page_i])[index_in_page(e)]) == e.generation();
        }

        entity get_entity_at_index(std::size_t index) const noexcept
        {
            GAMECOE_ASSERT_LOG(index < m_dense.size(), "sparse_set::get_entity_at_index(): index out of bounds");
            return m_dense[index];
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
