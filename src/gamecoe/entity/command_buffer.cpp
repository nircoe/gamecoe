#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <string>
#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe_config.hpp>

#if GAMECOE_USE_LOGCOE
    #include <logcoe.hpp>
#endif

namespace gamecoe
{
    command_buffer::placeholder command_buffer::spawn(components::transform t)
    {
        m_spawn_transforms.push_back(t);
        return placeholder(static_cast<std::uint32_t>(m_spawn_transforms.size() - 1));
    }

    entity command_buffer::resolver::resolve(placeholder p) const
    {
        GAMECOE_ASSERT_LOG(p.m_index < m_created.size(), "command_buffer::resolver::resolve(): placeholder out of range");
        return m_created[p.m_index];
    }

    void command_buffer::set_parent(placeholder child, placeholder parent)
    {
        m_commands.emplace_back([child, parent](entities& ents, const resolver& r)
        {
            ents.set_parent(r.resolve(child), r.resolve(parent));
        });
    }

    void command_buffer::flush(entities& ents, std::optional<scene_id> scene)
    {
        std::vector<entity> created;
        created.reserve(m_spawn_transforms.size());

        for (const components::transform& t : m_spawn_transforms)
        {
            entity e = ents.create(t);
            if (scene) ents.add_component<components::scene_tag>(e, components::scene_tag{ *scene });
            created.push_back(e);
        }

        resolver r(created);
        while (!m_commands.empty())
        {
            auto batch = std::move(m_commands);
            for (auto& cmd : batch)
                cmd(ents, r);
        }

        logcoe::info("command_buffer::flush(): flushed " + std::to_string(created.size()) + " entities");

        clear();
    }

    void command_buffer::clear()
    {
        m_last_spawn_count = m_spawn_transforms.size();
        m_spawn_transforms.clear();
        m_spawn_transforms.shrink_to_fit();
        m_last_command_count = m_commands.size();
        m_commands.clear();
        m_commands.shrink_to_fit();
    }

    void command_buffer::reserve_from_last_build()
    {
        m_spawn_transforms.reserve(m_last_spawn_count);
        m_commands.reserve(m_last_command_count);
    }
} // namespace gamecoe
