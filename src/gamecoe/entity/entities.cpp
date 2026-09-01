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

    entity entities::create(components::transform initial_transform)
    {
        std::uint32_t id;
        std::uint16_t generation;

        if (m_recycle_ids.empty())
        {
            id = m_current_entity_id;
            GAMECOE_ASSERT_LOG(id <= entity::MAX_ENTITIES, "entities::create(): entity limit reached");
            if (id > entity::MAX_ENTITIES) return entity::invalid();

            m_current_entity_id++;
            generation = 0;
            m_generations.push_back(generation);
            m_self_active.push_back(true);
        }
        else
        {
            id = m_recycle_ids.back();
            m_recycle_ids.pop_back();
            generation = m_generations[id];
            m_self_active[id] = true;
        }

        entity e = entity::create(id, generation);
        get_pool<components::transform>()->add(e, true, std::move(initial_transform));

        return e;
    }

    components::transform* entities::transform(entity e)
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::transform(): entity is not valid");
        if (!valid(e)) return nullptr;

        components::transform* t = get_component<components::transform>(e);
        GAMECOE_ASSERT_LOG(t != nullptr, "entities::transform(): transform missing (should be impossible - mandatory component)");
        return t;
    }

    const components::transform* entities::transform(entity e) const
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::transform(): entity is not valid");
        if (!valid(e)) return nullptr;

        const components::transform* t = get_component<components::transform>(e);
        GAMECOE_ASSERT_LOG(t != nullptr, "entities::transform(): transform missing (should be impossible - mandatory component)");
        return t;
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

            unlink_parent(current);

            if (auto* kids = get_pool<components::children>()->try_get(current))
                to_destroy.insert(to_destroy.end(), kids->handles.begin(), kids->handles.end());

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
        m_self_active.clear();
        m_current_entity_id = 0;
        logcoe::info("entities::clear(): cleared all entities");
    }

    bool entities::valid(entity e) const
    {
        return e.id() < m_generations.size() && m_generations[e.id()] == e.generation();
    }

    void entities::activate(entity e)
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::activate(): entity is not valid");
        if (!valid(e)) return;

        if (m_self_active[e.id()])
        {
            logcoe::debug("entities::activate(): entity already active, ignoring");
            return;
        }
        m_self_active[e.id()] = true;
        set_active(e, compute_world_active(e));
    }

    void entities::deactivate(entity e)
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::deactivate(): entity is not valid");
        if (!valid(e)) return;

        if (!m_self_active[e.id()])
        {
            logcoe::debug("entities::deactivate(): entity already inactive, ignoring");
            return;
        }
        m_self_active[e.id()] = false;
        set_active(e, false);
    }

    // Each node carries its own target: a self-inactive descendant stays inactive even when
    // an ancestor above it reactivates.
    void entities::set_active(entity e, bool world_active)
    {
        std::vector<std::pair<entity, bool>> worklist{ { e, world_active } };

        while (!worklist.empty())
        {
            auto [current, target] = worklist.back();
            worklist.pop_back();

            if (!valid(current) || is_active(current) == target) continue;

            for (auto &pool : m_pools) if (pool) (target ? pool->activate(current) : pool->deactivate(current));

            if (auto* kids = get_pool<components::children>()->try_get(current))
                for (entity child : kids->handles)
                    worklist.emplace_back(child, m_self_active[child.id()] && target);
        }
    }

    bool entities::compute_world_active(entity e)
    {
        if (!m_self_active[e.id()]) return false;
        auto* p = get_pool<components::parent>()->try_get(e);
        return !p || is_active(p->handle);
    }

    // Reads the transform pool's partition boundary directly - transform is mandatory, so
    // is_active() can never disagree with the pools.
    bool entities::is_active(entity e) const
    {
        GAMECOE_ASSERT_LOG(valid(e), "entities::is_active(): entity is not valid");
        if (!valid(e)) return false;   // release-mode only - debug already caught it via the assert above

        auto pool = find_pool<components::transform>();
        GAMECOE_ASSERT_LOG(pool != nullptr && pool->contains(e), "entities::is_active(): transform missing (should be impossible - mandatory component)");
        if (!pool || !pool->contains(e)) return false;   // release-mode only - debug already caught it via the assert above

        return pool->is_active(e);
    }

    void entities::reserve(std::size_t capacity)
    {
        m_generations.reserve(capacity);
        m_recycle_ids.reserve(capacity);
        m_self_active.reserve(capacity);

        for (auto &pool : m_pools) if (pool) pool->reserve(capacity);

        logcoe::debug("entities::reserve(): reserved capacity for " + std::to_string(capacity) + " entities");
    }

    std::size_t entities::size() const
    {
        const component_pool<components::transform>* pool = find_pool<components::transform>();
        return pool ? pool->size() : 0;
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

        unlink_parent(child);
        parent_pool->add(child, is_active(child), components::parent{ parent });

        if (children_pool->contains(parent)) children_pool->get(parent).handles.push_back(child);
        else children_pool->add(parent, is_active(parent), components::children{ { child } });

        set_active(child, compute_world_active(child));
    }

    bool entities::unlink_parent(entity child)
    {
        auto parent_pool = get_pool<components::parent>();
        auto* parent_comp = parent_pool->try_get(child);
        if (!parent_comp) return false;

        entity old_parent = parent_comp->handle;

        auto children_pool = get_pool<components::children>();
        if (auto* kids = children_pool->try_get(old_parent))
        {
            std::erase(kids->handles, child);
            // A childless entity has no children component at all, not one with an empty list -
            // so has_children() alone tells you whether an entity has any children.
            if (!kids->has_children()) children_pool->remove(old_parent);
        }

        parent_pool->remove(child);
        return true;
    }

    void entities::remove_parent(entity child)
    {
        if (!unlink_parent(child))
        {
            logcoe::debug("entities::remove_parent(): entity has no parent, ignoring");
            return;
        }

        set_active(child, compute_world_active(child));
    }

    void entities::remove_children(entity parent)
    {
        auto children_pool = get_pool<components::children>();
        auto* kids = children_pool->try_get(parent);
        if (!kids)
        {
            logcoe::debug("entities::remove_children(): entity has no children, ignoring");
            return;
        }

        // Copy protects iteration against set_active() swapping storage within children_pool
        std::vector<entity> handles = kids->handles;

        for (entity child : handles)
        {
            if (has_component<components::parent>(child))
            {
                get_pool<components::parent>()->remove(child);
                set_active(child, compute_world_active(child));
            }
        }

        children_pool->remove(parent);
    }

} // namespace gamecoe
