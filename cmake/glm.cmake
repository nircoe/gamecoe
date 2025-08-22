function(fetch_glm)
    message(STATUS "[gamecoe] Fetching glm from source...")

    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
    set(GLM_QUIET ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)

    set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
endfunction()
