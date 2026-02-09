function(fetch_testcoe)
    if(NOT DEFINED GAMECOE_USE_TESTCOE)
        set(GAMECOE_USE_TESTCOE OFF)
    endif()

    if(GAMECOE_USE_TESTCOE)
        message(STATUS "[gamecoe] Using testcoe for testing")
        message(STATUS "[gamecoe] To disable: \"set(GAMECOE_USE_TESTCOE OFF)\" before fetching gamecoe")
        set(GAMECOE_USE_TESTCOE 1 PARENT_SCOPE)

        message(STATUS "[gamecoe] Fetching testcoe from source...")

        FetchContent_Declare(
            testcoe
            GIT_REPOSITORY https://github.com/nircoe/testcoe.git
            GIT_TAG v0.1.1
        )
        FetchContent_MakeAvailable(testcoe)
    else()
        message(STATUS "[gamecoe] testcoe disabled")
        message(STATUS "[gamecoe] To enable: \"set(GAMECOE_USE_TESTCOE ON)\" before fetching gamecoe")
        set(GAMECOE_USE_TESTCOE 0 PARENT_SCOPE)
    endif()
endfunction()
