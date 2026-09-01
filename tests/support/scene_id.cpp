#include <support/scene_id.hpp>

namespace gamecoe
{
    std::string to_string(scene_id id)
    {
        return "TestScene" + std::to_string(static_cast<std::uint16_t>(id));
    }
} // namespace gamecoe
