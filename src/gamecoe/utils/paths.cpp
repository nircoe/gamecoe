#include <gamecoe/utils/paths.hpp>
#include <gamecoe/utils/error_handler.hpp>
#include <expected>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
    #include <limits.h>
    #include <cstdint>
#endif

namespace gamecoe
{
    namespace
    {
        std::expected<std::filesystem::path, error> getExecutablePath()
        {
            std::filesystem::path exe;
#if _WIN32
            char path[MAX_PATH];
            DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
            if (result == 0 || result == MAX_PATH)
                return std::unexpected(
                        detail::make_error(
                            error_code::path_resolution_failed,
                            "[gamecoe] Could not get the executable path on Windows"));
            exe = std::filesystem::path(path);
#elif __APPLE__
            char path[PATH_MAX];
            std::uint32_t size = PATH_MAX;
            if(_NSGetExecutablePath(path, &size))
               return std::unexpected(
                        detail::make_error(
                            error_code::path_resolution_failed,
                            "[gamecoe] Could not get the executable path on MacOS"));
            exe = std::filesystem::path(path);
#elif __linux__
            try { exe = std::filesystem::canonical("/proc/self/exe"); }
            catch(const std::filesystem::filesystem_error &e)
            {
                return std::unexpected(
                            detail::make_error(
                                error_code::path_resolution_failed,
                                "[gamecoe] Could not get the executable path on Linux: " + std::string(e.what())));
            }
#else
            return std::unexpected(
                    detail::make_error(
                        error_code::unsupported_platform,
                        "[gamecoe] Unsupported Operation System"));
#endif

            return exe;
        }
    }

    std::expected<std::string, error> getExecutableDirectory()
    {
        auto exe = getExecutablePath();
        if (!exe) return std::unexpected(exe.error());
        return exe->parent_path().string();
    }

    std::expected<std::string, error> resolvePath(const std::string &relativePath)
    {
        auto exe = getExecutablePath();
        if (!exe) return std::unexpected(exe.error());
        return (exe->parent_path() / relativePath).string();
    }
} // namespace gamecoe
