#include <gamecoe/entity/command_buffer.hpp>
#include <gamecoe/component/scene_tag.hpp>
#include <cassert>

namespace gamecoe
{
    command_buffer::placeholder command_buffer::spawn(components::transform t)
    {
        m_spawn_transforms.push_back(t);
        return placeholder(static_cast<std::uint32_t>(m_spawn_transforms.size() - 1));
    }

    entity command_buffer::resolver::resolve(placeholder p) const
    {
        assert(p.m_index < m_created.size() && "command_buffer::resolver::resolve(): placeholder out of range");
        return m_created[p.m_index];
    }

    void command_buffer::flush(entities& ents, std::optional<scene_id> scene)
    {
        std::vector<entity> created;
        created.reserve(m_spawn_transforms.size());

        for (const components::transform& t : m_spawn_transforms)
        {
            entity e = ents.create();
            ents.transform(e) = t;
            if (scene) ents.add_component<components::scene_tag>(e, components::scene_tag{ *scene });
            created.push_back(e);
        }

        resolver r(created);
        for (auto& cmd : m_commands)
            cmd(ents, r);

        clear();
    }

    void command_buffer::clear()
    {
        m_spawn_transforms.clear();
        m_commands.clear();
    }
} // namespace gamecoe
