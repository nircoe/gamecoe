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
            message(FATAL_ERROR 
                "[gamecoe] Failed to install jinja2. Please run: pip install --user jinja2\n"
                "  For Arch-based distros (externally-managed Python): sudo pacman -S python-jinja\n"
                "  Or use a virtual environment: python -m venv venv && source venv/bin/activate && pip install jinja2\n"
            )
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
    if(NOT DEFINED GAMECOE_GRAPHICS_API)
        set(GAMECOE_GRAPHICS_API "OpenGL")
        message(STATUS "[gamecoe] Using default graphics API - ${GAMECOE_GRAPHICS_API}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_API <API>)\"")
        message(STATUS "[gamecoe] Options are \"OpenGL\" (\"Vulkan\" not supported yet)")
    endif()
    
    set(DEFAULT_VERSION_MAJOR OFF)
    if(NOT DEFINED GAMECOE_GRAPHICS_VERSION_MAJOR)
        set(DEFAULT_VERSION_MAJOR ON)
        set(GAMECOE_GRAPHICS_VERSION_MAJOR 3)
        set(GAMECOE_GRAPHICS_VERSION_MAJOR 3 PARENT_SCOPE)
    endif()

    set(DEFAULT_VERSION_MINOR OFF)
    if(NOT DEFINED GAMECOE_GRAPHICS_VERSION_MINOR)
        set(DEFAULT_VERSION_MINOR ON)
        set(GAMECOE_GRAPHICS_VERSION_MINOR 3)
        set(GAMECOE_GRAPHICS_VERSION_MINOR 3 PARENT_SCOPE)
    endif()
    
    if(DEFAULT_VERSION_MAJOR OR DEFAULT_VERSION_MINOR)
        message(STATUS "[gamecoe] Using default version ${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_VERSION_MAJOR <major>)\" and \"set(GAMECOE_GRAPHICS_VERSION_MINOR <minor>)\"")
        message(STATUS "[gamecoe] Currently supported: OpenGL 3.3+") # TODO: Add more in the future
    endif()

    if(NOT DEFINED GAMECOE_GRAPHICS_PROFILE)
        set(GAMECOE_GRAPHICS_PROFILE "core")
        message(STATUS "[gamecoe] Using default profile: ${GAMECOE_GRAPHICS_PROFILE}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_PROFILE <profile>)\"")
        message(STATUS "[gamecoe] Options are \"core\" and \"compatibility\"")
    endif()

    if(GAMECOE_GRAPHICS_API STREQUAL "OpenGL")
        set(GAMECOE_GRAPHICS_API "gl") # for GLAD generator
        set(GAMECOE_USE_OPENGL 1 PARENT_SCOPE)
        set(GAMECOE_USE_VULKAN 0 PARENT_SCOPE)
    elseif(GAMECOE_GRAPHICS_API STREQUAL "Vulkan")
        message(FATAL_ERROR "[gamecoe] Vulkan is not supported yet")
        # Uncomment when Vulkan will be supported
        # set(GAMECOE_GRAPHICS_API "vulkan") # for GLAD generator
        # set(GAMECOE_USE_OPENGL 0 PARENT_SCOPE)
        # set(GAMECOE_USE_VULKAN 1 PARENT_SCOPE)
    else()
        message(FATAL_ERROR "[gamecoe] ${GAMECOE_GRAPHICS_API} is not a supported graphics API")
    endif()

    if(GAMECOE_GRAPHICS_PROFILE STREQUAL "core")
        set(GAMECOE_PROFILE_CORE 1 PARENT_SCOPE)
        set(GAMECOE_PROFILE_COMPAT 0 PARENT_SCOPE)
    elseif(GAMECOE_GRAPHICS_PROFILE STREQUAL "compatibility")
        set(GAMECOE_PROFILE_CORE 0 PARENT_SCOPE)
        set(GAMECOE_PROFILE_COMPAT 1 PARENT_SCOPE)
    endif()

    # GLAD paths
    set(GAMECOE_GLAD_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/glad")
    set(GAMECOE_GLAD_SOURCES "${GAMECOE_GLAD_OUTPUT_DIR}/src/gl.c")
    set(GAMECOE_GLAD_HEADERS "${GAMECOE_GLAD_OUTPUT_DIR}/include/glad/gl.h")

    # Export to parent scope
    set(GAMECOE_GLAD_OUTPUT_DIR "${GAMECOE_GLAD_OUTPUT_DIR}" PARENT_SCOPE)
    set(GAMECOE_GLAD_SOURCES "${GAMECOE_GLAD_SOURCES}" PARENT_SCOPE)
    set(GAMECOE_GLAD_HEADERS "${GAMECOE_GLAD_HEADERS}" PARENT_SCOPE)

    file(MAKE_DIRECTORY ${GAMECOE_GLAD_OUTPUT_DIR})

    option(GAMECOE_GLAD_QUIET "Quiet GLAD2 generation output" ON)
    if(GAMECOE_GLAD_QUIET)
        message(STATUS "[gamecoe] GLAD2 generation output: QUIET")
        message(STATUS "[gamecoe] To show verbose output: \"set(GAMECOE_GLAD_QUIET OFF)\" before fetching gamecoe")
    else()
        message(STATUS "[gamecoe] GLAD2 generation output: VERBOSE")
    endif()

    set(GLAD_COMMAND_ARGS
        --api=${GAMECOE_GRAPHICS_API}:${GAMECOE_GRAPHICS_PROFILE}=${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR}
        --out-path=${GAMECOE_GLAD_OUTPUT_DIR}
        --reproducible
    )
    if(GAMECOE_GLAD_QUIET)
        list(APPEND GLAD_COMMAND_ARGS --quiet)
    endif()
    list(APPEND GLAD_COMMAND_ARGS c)

    set(GLAD_COMMENT "[gamecoe] Generating GLAD2")
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
    )
endfunction()