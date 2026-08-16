#pragma once

#include <gamecoe/utils/error.hpp>
#include <expected>
#include <string>

namespace gamecoe {
    // Gets the executable directory (build root) full path
    [[nodiscard]] std::expected<std::string, error> executable_directory();
    // Gets relative path to build root, and returns the full path
    [[nodiscard]] std::expected<std::string, error> resolve_path(const std::string &relativePath);
} // namespace gamecoe
