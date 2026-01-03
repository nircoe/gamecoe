function(fetch_glfw)
    message(STATUS "[gamecoe] Fetching GLFW from source...")

    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    if(UNIX AND NOT APPLE)
        option(GAMECOE_GLFW_WAYLAND "Enable Wayland support" ON)
        option(GAMECOE_GLFW_X11 "Enable X11 support" ON)
        
        message(STATUS "[gamecoe] GLFW window system (Linux):")
        if(GAMECOE_GLFW_WAYLAND)
            message(STATUS "[gamecoe] Wayland: ENABLED")
        else()
            message(STATUS "[gamecoe] Wayland: DISABLED")
        endif()
        if(GAMECOE_GLFW_X11)
            message(STATUS "[gamecoe] X11: ENABLED")
        else()
            message(STATUS "[gamecoe] X11: DISABLED")
        endif()
        message(STATUS "[gamecoe] To change: \"set(GAMECOE_GLFW_WAYLAND OFF)\" or \"set(GAMECOE_GLFW_X11 OFF)\" before fetching gamecoe")

        if(NOT GAMECOE_GLFW_WAYLAND AND NOT GAMECOE_GLFW_X11)
            message(FATAL_ERROR "[gamecoe] Linux requires at least X11 or Wayland")
        endif()
        set(GLFW_BUILD_WAYLAND ${GAMECOE_GLFW_WAYLAND} CACHE BOOL "" FORCE)
        set(GLFW_BUILD_X11 ${GAMECOE_GLFW_X11} CACHE BOOL "" FORCE)
    endif()

    FetchContent_Declare(
        GLFW
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
endfunction()