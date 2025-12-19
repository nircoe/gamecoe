#include <gamecoe/utils/paths.hpp>

#ifdef _WIN32
    #include <windows.h>
#elif __linux__
    #include <unistd.h>
    #include <linux/limits.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
#endif

namespace gamecoe
{
    std::string getExecutableDirectory()
    {
        return "";
    }

    std::string resolvePath(const std::string &relativePath)
    {
        return "";
    }
} // namespace gamecoe
