#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>
#include <memory>
#include <array>
#include <utility>
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
        std::uint32_t m_active_count{0};

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

        // Swaps two dense slots and repoints both sparse entries. 
        // Safe when a == b (writes the same packed value twice). 
        // Pages are guaranteed live: both slots hold entities currently in m_dense, 
        // so their pages were allocated by insert().
        void swap_dense(std::uint32_t a, std::uint32_t b) noexcept
        {
            std::swap(m_dense[a], m_dense[b]);
            entity ea = m_dense[a];
            entity eb = m_dense[b];
            (*m_sparse[page_index(ea)])[index_in_page(ea)] = pack_dense_index(a, ea.generation());
            (*m_sparse[page_index(eb)])[index_in_page(eb)] = pack_dense_index(b, eb.generation());
        }

    public:
        sparse_set() noexcept = default;
        sparse_set(const sparse_set&) = delete;
        sparse_set& operator=(const sparse_set&) = delete;
        sparse_set(sparse_set&& other) noexcept
            : m_sparse(std::move(other.m_sparse)), m_dense(std::move(other.m_dense)),
              m_active_count(std::exchange(other.m_active_count, 0)) {}

        sparse_set& operator=(sparse_set&& other) noexcept
        {
            m_sparse = std::move(other.m_sparse);
            m_dense = std::move(other.m_dense);
            m_active_count = std::exchange(other.m_active_count, 0);
            return *this;
        }

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

            // Entities are active by default when inserted.
            activate_at(static_cast<std::uint32_t>(m_dense.size() - 1));
        }

        // Mirrors erase() but skips entity-to-index lookup, used by component_pool::remove()
        // which has already resolved the index and would otherwise pay for redundant lookup.
        void erase_at(std::uint32_t dense_index)
        {
            if (dense_index >= m_dense.size()) return;

            // Move the entity out of the active partition first (the same swap deactivate does), so the
            // swap-with-back below can never drop an inactive entity into an active slot.
            if (dense_index < m_active_count)
            {
                deactivate_at(dense_index);
                dense_index = m_active_count;   // deactivate_at() left it in the first inactive slot
            }

            // Both endpoints are now inactive, so the swap-with-back below can never disturb the active partition.
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

        void erase(entity e) { if (auto i = index(e)) erase_at(i.value()); }

        std::size_t active_size() const noexcept { return m_active_count; }
        bool is_active(std::uint32_t dense_index) const noexcept { return dense_index < m_active_count; }

        void deactivate(entity e) { if (auto i = index(e)) deactivate_at(i.value()); }
        void activate(entity e)   { if (auto i = index(e)) activate_at(i.value()); }

        // Index-taking forms mirror erase_at(): used by component_pool, which has already
        // resolved the index and would otherwise pay for a redundant lookup.
        void deactivate_at(std::uint32_t dense_index)
        {
            GAMECOE_ASSERT_LOG(dense_index < m_dense.size(), "sparse_set::deactivate_at(): index out of bounds");
            if (dense_index >= m_active_count) return;   // already inactive
            --m_active_count;
            swap_dense(dense_index, m_active_count);     // m_active_count now names the old last-active slot
        }

        void activate_at(std::uint32_t dense_index)
        {
            GAMECOE_ASSERT_LOG(dense_index < m_dense.size(), "sparse_set::activate_at(): index out of bounds");
            if (dense_index < m_active_count) return;    // already active
            swap_dense(dense_index, m_active_count);     // m_active_count names the first inactive slot
            ++m_active_count;
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

        void clear() { m_sparse.clear(); m_dense.clear(); m_active_count = 0; }
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
