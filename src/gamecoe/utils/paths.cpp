#include <gamecoe/utils/paths.hpp>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
    #include <limits.h>
    #include <cstdint>
#endif

namespace gamecoe
{
    std::filesystem::path getExecutablePath()
    {
        std::filesystem::path exe;
#if _WIN32
        char path[MAX_PATH];
        DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
        if (result == 0 || result == MAX_PATH)
            throw std::runtime_error("[gamecoe] Could not get the executable path on Windows");
        exe = std::filesystem::path(path);
#elif __APPLE__
        char path[PATH_MAX];
        std::uint32_t size = PATH_MAX;
        if(_NSGetExecutablePath(path, &size))
           throw std::runtime_error("[gamecoe] Could not get the executable path on MacOS");
        exe = std::filesystem::path(path);
#elif __linux__
        try { exe = std::filesystem::canonical("/proc/self/exe"); }
        catch(const std::filesystem::filesystem_error &e)
        {
            throw std::runtime_error("[gamecoe] Could not get the executable path on Linux: " + std::string(e.what()));
        }
#else
        throw std::runtime_error("[gamecoe] Unsupported Operation System");
#endif

        return exe;
    }

    std::string getExecutableDirectory()
    {
        return getExecutablePath().parent_path().string();
    }

    std::string resolvePath(const std::string &relativePath)
    {
        return (getExecutablePath().parent_path() / relativePath).string();
    }
} // namespace gamecoe
