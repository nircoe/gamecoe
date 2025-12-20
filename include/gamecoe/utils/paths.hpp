#pragma once

#include <string>

namespace gamecoe {
    // Gets the executable directory (build root) full path
    std::string getExecutableDirectory();
    // Gets relative path to build root, and returns the full path
    std::string resolvePath(const std::string &relativePath);
} // namespace gamecoe
