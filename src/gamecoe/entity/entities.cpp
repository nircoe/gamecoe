#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
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

        std::vector<entity> to_destroy{ e };
        while (!to_destroy.empty())
        {
            entity current = to_destroy.back();
            to_destroy.pop_back();

            if (!valid(current)) continue;

            remove_parent(current);

            if (has_component<components::children>(current))
            {
                const auto& handles = get_pool<components::children>()->get(current).handles;
                to_destroy.insert(to_destroy.end(), handles.begin(), handles.end());
            }

            m_generations[current.id()]++;
            m_recycle_ids.push_back(current.id());

            for (auto &pool : m_pools) if (pool) pool->remove(current);
        }
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

    void entities::set_parent(entity child, entity parent)
    {
        assert(valid(child) && valid(parent) && "entities::set_parent(): child/parent must be valid entities");
        assert(child != parent && "entities::set_parent(): entity cannot be its own parent");

        auto parent_pool = get_pool<components::parent>();
        auto children_pool = get_pool<components::children>();

        if (parent_pool->contains(child) && parent_pool->get(child).handle == parent) return;

        entity ancestor = parent;
        std::size_t guard = 0;
        while (guard < size())
        {
            assert(ancestor != child && "entities::set_parent(): would create a parent/child cycle");
            // Only relevant in release mode - we should have already caught cycles via the assert above in debug builds
            if (ancestor == child) return;
            if (!parent_pool->contains(ancestor)) break;
            ancestor = parent_pool->get(ancestor).handle;
            ++guard;
        }

        remove_parent(child);
        parent_pool->add(child, components::parent{ parent });

        if (children_pool->contains(parent)) children_pool->get(parent).handles.push_back(child);
        else children_pool->add(parent, components::children{ { child } });
    }

    void entities::remove_parent(entity child)
    {
        if (!has_component<components::parent>(child)) return;

        auto parent_pool = get_pool<components::parent>();
        entity old_parent = parent_pool->get(child).handle;

        if (has_component<components::children>(old_parent))
        {
            auto children_pool = get_pool<components::children>();
            std::erase(children_pool->get(old_parent).handles, child);
            if (!children_pool->get(old_parent).has_children()) children_pool->remove(old_parent);
        }

        parent_pool->remove(child);
    }

    void entities::remove_children(entity parent)
    {
        if (!has_component<components::children>(parent)) return;

        auto children_pool = get_pool<components::children>();
        const auto& handles = children_pool->get(parent).handles;

        for (entity child : handles)
            if (has_component<components::parent>(child)) get_pool<components::parent>()->remove(child);

        children_pool->remove(parent);
    }

} // namespace gamecoe
