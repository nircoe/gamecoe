function(ensure_jinja2)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import jinja2"
        RESULT_VARIABLE JINJA2_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT JINJA2_RESULT EQUAL 0)
        message(STATUS "[gamecoe] Installing jinja2 for GLAD2 generator...")
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -m pip install --user jinja2
            RESULT_VARIABLE INSTALL_RESULT
        )
        
        if(NOT INSTALL_RESULT EQUAL 0)
            message(FATAL_ERROR "[gamecoe] Failed to install jinja2. Please run: pip install jinja2")
        endif()
    endif()
endfunction()

function(generate_glad)
    message(STATUS "[gamecoe] Fetching GLAD from source...")

    FetchContent_Declare(
        GLAD
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
    )
    FetchContent_MakeAvailable(glad)

    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    ensure_jinja2()

    # User-configurable options
    set(GAMECOE_GRAPHICS_API "gl" CACHE STRING "Graphics API (gl, vulkan)")
    set(GAMECOE_GRAPHICS_VERSION_MAJOR "3" CACHE STRING "Graphics API version major")
    set(GAMECOE_GRAPHICS_VERSION_MINOR "3" CACHE STRING "Graphics API version minor")
    set(GAMECOE_GRAPHICS_PROFILE "core" CACHE STRING "Graphics profile (core, compatibility)")

    if(GAMECOE_GRAPHICS_API STREQUAL "gl")
        set(GAMECOE_USE_OPENGL "1")
        set(GAMECOE_USE_VULKAN "0")
    elseif(GAMECOE_GRAPHICS_API STREQUAL "vulkan")
        set(GAMECOE_USE_OPENGL "0")
        set(GAMECOE_USE_VULKAN "1")
    endif()

    if(GAMECOE_GRAPHICS_PROFILE STREQUAL "core")
        set(GAMECOE_PROFILE_CORE "1")
        set(GAMECOE_PROFILE_COMPAT "0")
    elseif(GAMECOE_GRAPHICS_PROFILE STREQUAL "compatibility")
        set(GAMECOE_PROFILE_CORE "0")
        set(GAMECOE_PROFILE_COMPAT "1")
    endif()

    # GLAD paths
    set(GAMECOE_GLAD_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/glad")
    set(GAMECOE_GLAD_SOURCES "${GAMECOE_GLAD_OUTPUT_DIR}/src/gl.c")
    set(GAMECOE_GLAD_HEADERS "${GAMECOE_GLAD_OUTPUT_DIR}/include/glad/gl.h")

    # Export to parent scope
    set(GAMECOE_GLAD_OUTPUT_DIR "${GAMECOE_GLAD_OUTPUT_DIR}" PARENT_SCOPE)
    set(GAMECOE_GLAD_SOURCES "${GAMECOE_GLAD_SOURCES}" PARENT_SCOPE)
    set(GAMECOE_GLAD_HEADERS "${GAMECOE_GLAD_HEADERS}" PARENT_SCOPE)

    # config header path
    set(GAMECOE_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/config")
    set(GAMECOE_CONFIG_DIR "${GAMECOE_CONFIG_DIR}" PARENT_SCOPE)

    file(MAKE_DIRECTORY ${GAMECOE_GLAD_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${GAMECOE_CONFIG_DIR})

    # Generate config header
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/gamecoe_config.h.in
        ${GAMECOE_CONFIG_DIR}/gamecoe_config.h
        @ONLY
    )

    option(GAMECOE_GLAD_QUIET "Quiet GLAD2 generation output" OFF)
    set(GLAD_COMMAND_ARGS
        --api=${GAMECOE_GRAPHICS_API}:${GAMECOE_GRAPHICS_PROFILE}=${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR}
        --out-path=${GAMECOE_GLAD_OUTPUT_DIR}
        --reproducible
    )
    if(GAMECOE_GLAD_QUIET)
        list(APPEND GLAD_COMMAND_ARGS --quiet)
    endif()
    list(APPEND GLAD_COMMAND_ARGS c)

    set(GLAD_COMMENT "[gamecoe] Generating GLAD2 ")
    string(APPEND GLAD_COMMENT "${GAMECOE_GRAPHICS_API}:${GAMECOE_GRAPHICS_PROFILE}=")
    string(APPEND GLAD_COMMENT "${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR} loader")


    add_custom_command(
        OUTPUT ${GAMECOE_GLAD_SOURCES} ${GAMECOE_GLAD_HEADERS}
        COMMAND ${Python3_EXECUTABLE} -m glad ${GLAD_COMMAND_ARGS}
        WORKING_DIRECTORY ${glad_SOURCE_DIR}
        COMMENT ${GLAD_COMMENT}
        VERBATIM
    )

    add_custom_target(glad
        DEPENDS ${GAMECOE_GLAD_SOURCES} ${GAMECOE_GLAD_HEADERS}
        COMMENT "[gamecoe] GLAD2 generation complete"
    )
endfunction()