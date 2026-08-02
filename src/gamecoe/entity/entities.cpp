#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <cassert>
#include <cstddef>

namespace gamecoe
{
    std::uint32_t entities::s_component_id{0};

    entity entities::create()
    {
        std::uint32_t id;
        std::uint16_t generation;

        if (m_recycle_ids.empty())
        {
            id = m_current_entity_id;
            assert(id <= entity::MAX_ENTITIES && "entities::create(): entity limit reached");
            m_current_entity_id++;
            generation = 0;
            m_generations.push_back(generation);
        }
        else
        {
            id = m_recycle_ids.back();
            m_recycle_ids.pop_back();
            generation = m_generations[id];
        }

        entity e = entity::create(id, generation);
        get_pool<components::transform>()->add(e);

        return e;
    }

    components::transform& entities::transform(entity e)
    {
        assert(valid(e) && "entities::transform(): entity is not valid");
        components::transform* t = get_component<components::transform>(e);
        assert(t && "entities::transform(): transform missing (should be impossible - mandatory component)");
        return *t;
    }

    const components::transform& entities::transform(entity e) const
    {
        assert(valid(e) && "entities::transform(): entity is not valid");
        const components::transform* t = get_component<components::transform>(e);
        assert(t && "entities::transform(): transform missing (should be impossible - mandatory component)");
        return *t;
    }

    void entities::destroy(entity e)
    {
        if (!valid(e)) return;

        for (auto &pool : m_pools) if (pool) pool->remove(e);

        m_generations[e.id()]++;
        m_recycle_ids.push_back(e.id());
    }

    void entities::clear()
    {
        m_pools.clear();
        m_recycle_ids.clear();
        m_generations.clear();
        m_current_entity_id = 0;
    }

    bool entities::valid(entity e) const
    {
        return e.id() < m_generations.size() && m_generations[e.id()] == e.generation();
    }

    void entities::reserve(std::size_t capacity)
    {
        m_generations.reserve(capacity);
        m_recycle_ids.reserve(capacity);

        for (auto &pool : m_pools) if (pool) pool->reserve(capacity);
    }

    std::size_t entities::size() const
    {
        assert(static_cast<std::size_t>(m_current_entity_id) >= m_recycle_ids.size());
        return static_cast<std::size_t>(m_current_entity_id) - m_recycle_ids.size();
    }

} // namespace gamecoe
