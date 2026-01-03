function(fetch_logcoe)
    if(NOT DEFINED GAMECOE_USE_LOGCOE)
        set(GAMECOE_USE_LOGCOE OFF)
    endif()

    if(GAMECOE_USE_LOGCOE)
        message(STATUS "[gamecoe] Using logcoe logging library")
        message(STATUS "[gamecoe] To disable: \"set(GAMECOE_USE_LOGCOE OFF)\" before fetching gamecoe")
        set(GAMECOE_USE_LOGCOE 1 CACHE INTERNAL "" FORCE)

        message(STATUS "[gamecoe] Fetching logcoe from source...")

        FetchContent_Declare(
            logcoe
            GIT_REPOSITORY https://github.com/nircoe/logcoe.git
            GIT_TAG v0.1.1
        )
        FetchContent_MakeAvailable(logcoe)
    else()
        message(STATUS "[gamecoe] logcoe logging disabled")
        message(STATUS "[gamecoe] To enable: \"set(GAMECOE_USE_LOGCOE ON)\" before fetching gamecoe")
        set(GAMECOE_USE_LOGCOE 0 CACHE INTERNAL "" FORCE)
    endif()
    
endfunction()