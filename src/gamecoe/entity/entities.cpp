#include <gamecoe/entity/entities.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/component/parent_child.hpp>
#include <cstddef>
#include <string>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

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
            GAMECOE_ASSERT_LOG(id <= entity::MAX_ENTITIES, "entities::create(): entity limit reached");
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
        GAMECOE_ASSERT_LOG(valid(e), "entities::transform(): entity is not valid");
        components::transform* t = get_component<components::transform>(e);
        GAMECOE_ASSERT_LOG(t != nullptr, "entities::transform(): transform missing (should be impossible - mandatory component)");
        return *t;
    }

    const components::transform& entities::transform(entity e) const
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::transform(): entity is not valid");
        const components::transform* t = get_component<components::transform>(e);
        GAMECOE_ASSERT_LOG(t != nullptr, "entities::transform(): transform missing (should be impossible - mandatory component)");
        return *t;
    }

    void entities::destroy(entity e)
    {
        if (!valid(e))
        {
            logcoe::debug("entities::destroy(): entity already invalid, ignoring");
            return;
        }

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
        logcoe::info("entities::clear(): cleared all entities");
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

        logcoe::debug("entities::reserve(): reserved capacity for " + std::to_string(capacity) + " entities");
    }

    std::size_t entities::size() const
    {
        GAMECOE_ASSERT_LOG(static_cast<std::size_t>(m_current_entity_id) >= m_recycle_ids.size(), "entities::size(): recycle count exceeds allocated id count");
        return static_cast<std::size_t>(m_current_entity_id) - m_recycle_ids.size();
    }

    void entities::set_parent(entity child, entity parent)
    {
        GAMECOE_ASSERT_LOG(valid(child) && valid(parent), "entities::set_parent(): child/parent must be valid entities");
        GAMECOE_ASSERT_LOG(child != parent, "entities::set_parent(): entity cannot be its own parent");

        auto parent_pool = get_pool<components::parent>();
        auto children_pool = get_pool<components::children>();

        if (parent_pool->contains(child) && parent_pool->get(child).handle == parent) return;

        // Walk up from parent toward the root, checking whether child appears as its own ancestor.
        // guard bounds the walk to entities.size() so a corrupted parent chain
        // (shouldn't happen — set_parent blocks cycles at insertion) can't loop forever.
        entity ancestor = parent;
        std::size_t guard = 0;
        const std::size_t max_guard = size();
        while (guard < max_guard)
        {
            GAMECOE_ASSERT_LOG(ancestor != child, "entities::set_parent(): would create a parent/child cycle");
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
            // A childless entity has no children component at all, not one with an empty list -
            // so has_component<children>(e) alone tells you whether an entity has any children.
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
