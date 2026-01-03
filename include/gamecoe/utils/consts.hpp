#pragma once

#include <gamecoe_config.hpp>

// This header needs to be include after glfw (and glad before that)

#if GAMECOE_USE_OPENGL
    #ifdef _glfw3_h_
        #if GAMECOE_PROFILE_CORE
            #define GAMECOE_GRAPHICS_PROFILE GLFW_OPENGL_CORE_PROFILE
        #elif GAMECOE_PROFILE_COMPAT
            #define GAMECOE_GRAPHICS_PROFILE GLFW_OPENGL_COMPAT_PROFILE
        #else
            #define GAMECOE_GRAPHICS_PROFILE
        #endif
    #endif
#elif GAMECOE_USE_VULKAN // Not supported yet!
    #define GAMECOE_VULKAN_API_VERSION VK_MAKE_VERSION(GAMECOE_GRAPHICS_VERSION_MAJOR, GAMECOE_GRAPHICS_VERSION_MINOR, 0)
#endif