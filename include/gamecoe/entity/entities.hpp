#pragma once

#include <cstddef>
#include <gamecoe/entity/entity.hpp>
#include <gamecoe/entity/component_pool.hpp>
#include <gamecoe/entity/extraction.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <gamecoe/component/transform.hpp>
#include <gamecoe/core/scene_id.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>
#include <cstdint>
#include <cassert>
#include <gamecoe/utils/error_handler.hpp>

namespace gamecoe
{
    namespace components
    {
        struct parent;
        struct children;
    } // namespace components

    // True for the hierarchy-managed relationship components - see entities::set_parent()/remove_parent()
    template <typename T>
    concept hierarchy_component = std::is_same_v<T, components::parent> || std::is_same_v<T, components::children>;

    class entities
    {
        // Plain counters, not atomics. Atomics would imply a thread-safety guarantee
        // this ECS doesn't have yet, revisit once the threading model is decided.
        static std::uint32_t s_component_id;

        std::vector<std::unique_ptr<basic_component_pool>> m_pools;
        std::vector<std::uint32_t> m_recycle_ids;
        std::vector<std::uint16_t> m_generations;
        // Per-entity activate()/deactivate() request, independent of any inherited parent state.
        std::vector<bool> m_self_active;
        // Scenes paused by game::deactivate_scene(). Kept apart from m_self_active, so resuming a scene
        // never undoes an entity's own deactivate().
        std::vector<scene_id> m_paused_scenes;
        // Scenes frozen by game::freeze_scene(). Frozen entities stay in the active partition, extract() and
        // for_each() skip them by their scene_tag.
        std::vector<scene_id> m_frozen_scenes;

        std::uint32_t m_current_entity_id{0};

        // Returns static and unique id for component T
        template <typename T>
        static std::uint32_t component_id()
        {
            static std::uint32_t s_componentT_id = s_component_id++;
            return s_componentT_id;
        }

        // Creates a new pool if not exists (lazy auto-registration).
        template <typename T>
        component_pool<T>* get_pool()
        {
            std::uint32_t comp_id = component_id<T>();

            if (comp_id >= m_pools.size())  m_pools.resize(comp_id + 1);
            if (!m_pools[comp_id])          m_pools[comp_id] = std::make_unique<component_pool<T>>();

            return static_cast<component_pool<T>*>(m_pools[comp_id].get());
        }

        // Non-creating lookup - nullptr if T's pool doesn't exist yet. The const-safe
        // counterpart to get_pool<T>() above, for callers that must not mutate m_pools.
        template <typename T>
        component_pool<T>* find_pool()
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return nullptr;
            return static_cast<component_pool<T>*>(m_pools[comp_id].get());
        }

        template <typename T>
        const component_pool<T>* find_pool() const
        {
            std::uint32_t comp_id = component_id<T>();
            if (comp_id >= m_pools.size() || !m_pools[comp_id]) return nullptr;
            return static_cast<const component_pool<T>*>(m_pools[comp_id].get());
        }

        bool in_frozen_scene(entity e, const component_pool<components::scene_tag> *tags) const
        {
            return detail::in_frozen_scene(e, tags, &m_frozen_scenes);
        }

        template <typename Pool, typename Func>
        void for_each_unfrozen(Pool *pool, Func &func) const
        {
            if (!pool) return;

            const auto *tags = find_pool<components::scene_tag>();
            pool->for_each([this, tags, &func](entity e, auto &component)
            {
                if (!in_frozen_scene(e, tags)) func(e, component);
            });
        }

        // Applies world_active to e and cascades to its subtree per each descendant's own self_active and scene pause.
        void set_active(entity e, bool world_active);

        // self_active AND scene not paused AND (no parent OR the parent's own world-active state) - the formula every
        // hierarchy-aware active-state recompute in this file is built on. Reads e's *current*
        // self_active and parent link, so callers update those first if this call means to reflect
        // a change (e.g. activate() sets m_self_active[e.id()] = true before calling this).
        bool compute_world_active(entity e);

        // True if the scene is in m_paused_scenes.
        bool is_scene_paused(scene_id id) const;

        // True if e carries a scene_tag whose scene is paused.
        bool in_paused_scene(entity e) const;

        // self_active AND e's own scene not paused. Ignores the parent chain.
        bool own_active(entity e) const;

        // Pool-unlink half of remove_parent(), with no active-state recompute - set_parent()
        // calls it directly so re-parenting recomputes once.
        // Returns true if child had a parent link that was removed, false if it had none.
        bool unlink_parent(entity child);

        // Re-tags root's subtree to target_tag, or clears the tag if target_tag is empty.
        // Stops at a node whose tag already matches, so it won't re-walk an already-correct subtree.
        void retag_subtree_scene(component_pool<components::scene_tag>& scene_tag_pool, entity root, const std::optional<components::scene_tag>& target_tag);

    public:
        entities() = default;
        entities(const entities&) = delete;
        entities(entities&& other) noexcept
            : m_pools(std::move(other.m_pools))
            , m_recycle_ids(std::move(other.m_recycle_ids))
            , m_generations(std::move(other.m_generations))
            , m_self_active(std::move(other.m_self_active))
            , m_paused_scenes(std::move(other.m_paused_scenes))
            , m_frozen_scenes(std::move(other.m_frozen_scenes))
            , m_current_entity_id(std::exchange(other.m_current_entity_id, 0))
        {}
        entities &operator=(const entities&) = delete;
        entities &operator=(entities&&) = delete;

        ~entities() = default;

        // May return a recycled id. Returns entity::invalid() in Release if the entity limit is reached.
        // scene_tag is stamped here and nowhere else, add/remove/set_component block it. No in_scene means
        // a global entity. An entity created into a paused scene starts inactive.
        entity create(components::transform initial_transform = components::transform{},
                      std::optional<scene_id> in_scene = std::nullopt);

        // No-op if e is already invalid.
        void destroy(entity e);

        // activate()/deactivate()/is_active() read and move the same active/inactive partition
        // that extract() and for_each() iterate over.
        void activate(entity e);
        void deactivate(entity e);
        bool is_active(entity e) const;

        // Scene-level pause, driven by game::deactivate_scene()/activate_scene(). Separate from
        // activate()/deactivate(), so resuming a scene never overrides an entity's own state.
        // Returns how many entities were re-evaluated, 0 if the scene was already in that state.
        std::size_t set_scene_paused(scene_id id, bool paused);

        // Scene-level freeze, driven by game::freeze_scene()/unfreeze_scene(). The scene's entities stay active,
        // extract() and for_each() skip them and extract_with_frozen() still returns them.
        // Returns true if the state changed, false if the scene was already in that state.
        bool set_scene_frozen(scene_id id, bool frozen);

        bool is_scene_frozen(scene_id id) const;

        // True if e carries a scene_tag whose scene is frozen. Says nothing about whether e is active.
        bool is_frozen(entity e) const;

        void clear();

        bool valid(entity e) const;

        // Also reserves capacity in every existing component pool, not just entity bookkeeping.
        void reserve(std::size_t capacity);

        std::size_t size() const;

        // Entity must already be valid, asserted. Returns nullptr in Release if e is invalid.
        template <typename T, typename... Args>
        T* add_component(entity e, Args&&... args)
        {
            static_assert(!std::is_same_v<T, components::transform>,
                "entities::add_component(): transform is mandatory, added automatically by create()");
            static_assert(!hierarchy_component<T>,
                "entities::add_component(): hierarchy components are managed - use entities::set_parent() instead");
            static_assert(!std::is_same_v<T, components::scene_tag>,
                "entities::add_component(): scene_tag is stamped by create(), pass the scene_id to create() instead");

            GAMECOE_ASSERT_GUARD(valid(e), "entities::add_component(): entity is not valid", nullptr);

            auto pool = get_pool<T>();
            return pool->add(e, is_active(e), std::forward<Args>(args)...);
        }

        // Safe on an invalid entity, returns false rather than asserting.
        template <typename T>
        bool has_component(entity e) const
        {
            if (!valid(e)) return false;

            auto pool = find_pool<T>();
            return pool && pool->contains(e);
        }

        // No-op if e doesn't have T, or e is invalid.
        template <typename T>
        void remove_component(entity e)
        {
            static_assert(!std::is_same_v<T, components::transform>,
                "entities::remove_component(): transform is mandatory and cannot be removed");
            static_assert(!hierarchy_component<T>,
                "entities::remove_component(): hierarchy components are managed - use "
                "entities::remove_parent() instead");
            static_assert(!std::is_same_v<T, components::scene_tag>,
                "entities::remove_component(): scene_tag is managed by set_parent() and destroy(), "
                "not removable directly");

            if (!has_component<T>(e)) return;

            m_pools[component_id<T>()]->remove(e);
        }

        // Pointer may be invalidated by any add_component call (pool reallocation).
        template <typename T>
        T* get_component(entity e)
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<component_pool<T>*>(m_pools[component_id<T>()].get());
            return pool->try_get(e);
        }

        // Pointer may be invalidated by any add_component call (pool reallocation).
        template <typename T>
        const T* get_component(entity e) const
        {
            if (!has_component<T>(e)) return nullptr;

            auto pool = static_cast<const component_pool<T>*>(m_pools[component_id<T>()].get());
            return pool->try_get(e);
        }

        // Add-or-assign: sets T's value if e already has it, otherwise adds it fresh.
        template <typename T, typename V>
        void set_component(entity e, V&& value)
        {
            static_assert(!hierarchy_component<T>,
                "entities::set_component(): hierarchy components are managed - use entities::set_parent() instead");
            static_assert(!std::is_same_v<std::decay_t<T>, components::scene_tag>,
                "entities::set_component(): scene_tag is stamped by create(), not settable afterward");
            static_assert(!std::is_same_v<std::decay_t<T>, components::transform>,
                "entities::set_component(): transform is a built-in component, use entities::transform(e) directly");

            if (T* c = get_component<T>(e)) *c = std::forward<V>(value);
            else                             add_component<T>(e, std::forward<V>(value));
        }

        // Transform always exists for a valid entity. Returns nullptr in Release if e is invalid.
        components::transform* transform(entity e);

        // Transform always exists for a valid entity. Returns nullptr in Release if e is invalid.
        const components::transform* transform(entity e) const;

        // nullptr if e is global (no scene_tag). Returns nullptr in Release if e is invalid.
        // Pointer may be invalidated by any add_component call, create() with a scene, or set_parent()
        // (pool reallocation).
        const components::scene_tag* scene(entity e) const;

        // Snapshot of every entity tagged with the scene (active + inactive).
        std::vector<entity> scene_entities(scene_id id) const;

        // Updates both sides. Parenting implies scene ownership: also re-tags child's whole subtree into
        // parent's scene (or clears it if parent is global), so the subtree follows that scene's pause.
        // For cosmetic cross-scene following, copy the transform in a system instead of parenting.
        void set_parent(entity child, entity parent);

        // Updates both sides.
        void remove_parent(entity child);

        // Updates both sides.
        void remove_children(entity parent);

        // Active entities only, and not the ones in a frozen scene. for_each_all() visits everything.
        template <typename T, typename Func>
        void for_each(Func &&func)
        {
            for_each_unfrozen(find_pool<T>(), func);
        }

        template <typename T, typename Func>
        void for_each(Func &&func) const
        {
            for_each_unfrozen(find_pool<T>(), func);
        }

        template <typename T, typename Func>
        void for_each_all(Func &&func)
        {
            auto pool = find_pool<T>();
            if (pool) pool->for_each_all(std::forward<Func>(func));
        }

        template <typename T, typename Func>
        void for_each_all(Func &&func) const
        {
            auto pool = find_pool<T>();
            if (pool) pool->for_each_all(std::forward<Func>(func));
        }

        // Active entities only, and not the ones in a frozen scene.
        template <typename... Components>
        extraction<Components...> extract()
        {
            return extraction<Components...>(find_pool<std::remove_const_t<Components>>()...,
                                             find_pool<components::scene_tag>(), &m_frozen_scenes);
        }

        template <typename... Components>
        extraction<std::add_const_t<Components>...> extract() const
        {
            return extraction<std::add_const_t<Components>...>(const_cast<entities*>(this)->find_pool<Components>()...,
                                                               find_pool<components::scene_tag>(), &m_frozen_scenes);
        }

        // Like extract(), but also returns entities in a frozen scene. For systems that must keep running over a
        // frozen scene, e.g. rendering.
        template <typename... Components>
        extraction<Components...> extract_with_frozen()
        {
            return extraction<Components...>(find_pool<std::remove_const_t<Components>>()..., nullptr, nullptr);
        }

        template <typename... Components>
        extraction<std::add_const_t<Components>...> extract_with_frozen() const
        {
            return extraction<std::add_const_t<Components>...>(const_cast<entities*>(this)->find_pool<Components>()...,
                                                               nullptr, nullptr);
        }
    };
} // namespace gamecoe
