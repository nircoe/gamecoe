#include <gamecoe/utils/error_handler.hpp>
#include <gamecoe/utils/paths.hpp>
#include <expected>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <limits.h>
    #include <mach-o/dyld.h>
    #include <cstdint>
#endif

namespace gamecoe
{
    namespace
    {
        [[nodiscard]] std::expected<std::filesystem::path, error> get_executable_path()
        {
            std::filesystem::path exe;
#if _WIN32
            char path[MAX_PATH];
            DWORD result = GetModuleFileNameA(nullptr, path, MAX_PATH);
            if (result == 0 || result == MAX_PATH)
                return std::unexpected(detail::make_error(error_code::path_resolution_failure,
                                                          "[gamecoe] Could not get the executable path on Windows"));
            exe = std::filesystem::path(path);
#elif __APPLE__
            char path[PATH_MAX];
            std::uint32_t size = PATH_MAX;
            if (_NSGetExecutablePath(path, &size))
                return std::unexpected(detail::make_error(error_code::path_resolution_failure,
                                                          "[gamecoe] Could not get the executable path on MacOS"));
            exe = std::filesystem::path(path);
#elif __linux__
            std::error_code ec;
            exe = std::filesystem::canonical("/proc/self/exe", ec);
            if (ec)
                return std::unexpected(
                    detail::make_error(error_code::path_resolution_failure,
                                       "[gamecoe] Could not get the executable path on Linux: " + ec.message()));
#else
            return std::unexpected(
                detail::make_error(error_code::unsupported_platform, "[gamecoe] Unsupported Operation System"));
#endif

            return exe;
        }
    } // namespace

    std::expected<std::string, error> executable_directory()
    {
        auto exe = get_executable_path();
        if (!exe) return std::unexpected(exe.error());
        return exe->parent_path().string();
    }

    std::expected<std::string, error> resolve_path(const std::string &relative_path)
    {
        auto exe = get_executable_path();
        if (!exe) return std::unexpected(exe.error());
        return (exe->parent_path() / relative_path).string();
    }
} // namespace gamecoe
