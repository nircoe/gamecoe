function(fetch_soundcoe)
    if(NOT DEFINED GAMECOE_USE_SOUNDCOE)
        set(GAMECOE_USE_SOUNDCOE OFF)
    endif()

    if(GAMECOE_USE_SOUNDCOE)
        message(STATUS "[gamecoe] Using soundcoe for sound management")
        message(STATUS "[gamecoe] To disable: \"set(GAMECOE_USE_SOUNDCOE OFF)\" before fetching gamecoe")
        set(GAMECOE_USE_SOUNDCOE 1 PARENT_SCOPE)

        message(STATUS "[gamecoe] Fetching soundcoe from source...")

        FetchContent_Declare(
            soundcoe
            GIT_REPOSITORY https://github.com/nircoe/soundcoe.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(soundcoe)
    else()
        message(STATUS "[gamecoe] soundcoe disabled")
        message(STATUS "[gamecoe] To enable: \"set(GAMECOE_USE_SOUNDCOE ON)\" before fetching gamecoe")
        set(GAMECOE_USE_SOUNDCOE 0 PARENT_SCOPE)
    endif()
    
endfunction()