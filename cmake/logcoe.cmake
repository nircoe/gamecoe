function(fetch_logcoe)
    message(STATUS "[gamecoe] Fetching logcoe from source...")

    FetchContent_Declare(
        logcoe
        GIT_REPOSITORY https://github.com/nircoe/logcoe.git
        GIT_TAG v0.1.0
    )
    FetchContent_MakeAvailable(logcoe)
endfunction()