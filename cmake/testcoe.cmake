function(gamecoe_fetch_testcoe)
    if(TARGET testcoe)
        message(STATUS "[gamecoe] testcoe already available, skipping fetch")
        return()
    endif()

    message(STATUS "[gamecoe] Fetching testcoe from source...")
    message(STATUS "[gamecoe] To change: \"set(GAMECOE_BUILD_TESTS OFF)\" before fetching gamecoe")

    FetchContent_Declare(
        testcoe
        GIT_REPOSITORY https://github.com/nircoe/testcoe.git
        GIT_TAG v0.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(testcoe)
endfunction()
