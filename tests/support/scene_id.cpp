#include <support/scene_id.hpp>
#include <utility>

namespace gamecoe
{
    std::string to_string(scene_id id) { return "TestScene" + std::to_string(std::to_underlying(id)); }
} // namespace gamecoe
