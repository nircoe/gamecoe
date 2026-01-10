function(ensure_jinja2 OUTPUT_PYTHON_VAR)
    # Check if the user has jinja2 installed
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import jinja2"
        RESULT_VARIABLE JINJA2_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    if(JINJA2_RESULT EQUAL 0)
        message(STATUS "[gamecoe] Using jinja2 to generate GLAD")
        set(${OUTPUT_PYTHON_VAR} ${Python3_EXECUTABLE} PARENT_SCOPE)
        return()
    endif()

    # Trying to install jinja2 for the user
    message(STATUS "[gamecoe] Trying to install jinja2")
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install --user jinja2
        RESULT_VARIABLE JINJA2_INSTALL_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    if(JINJA2_INSTALL_RESULT EQUAL 0)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import jinja2"
            RESULT_VARIABLE JINJA2_VERIFY
            OUTPUT_QUIET ERROR_QUIET
        )

        if(JINJA2_VERIFY EQUAL 0)
            set(${OUTPUT_PYTHON_VAR} ${Python3_EXECUTABLE} PARENT_SCOPE)
            return()
        endif()
    endif()

    # Couldn't use jinja2 in user environment, trying inside a venv
    message(STATUS "[gamecoe] Could not install jinja2, using virtual environment")
    set(VENV_DIR ${CMAKE_CURRENT_BINARY_DIR}/.venv_glad)

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}
        RESULT_VARIABLE VENV_RESULT
    )

    if(NOT VENV_RESULT EQUAL 0)
        message(FATAL_ERROR "[gamecoe] Failed to create virtual environment, please install jinja2 manually")
    endif()

    if(WIN32)
        set(VENV_PYTHON ${VENV_DIR}/Scripts/python.exe)
    else()
        set(VENV_PYTHON ${VENV_DIR}/bin/python)
    endif()

    execute_process(
        COMMAND ${VENV_PYTHON} -m pip install jinja2
        RESULT_VARIABLE VENV_INSTALL_RESULT
    )

    if(NOT VENV_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "[gamecoe] Failed to install jinja2 in virtual environment, please install jinja2 manually")
    endif()

    set(${OUTPUT_PYTHON_VAR} ${VENV_PYTHON} PARENT_SCOPE)
    message(STATUS "[gamecoe] Using virtual environment at: ${VENV_DIR}")
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
    ensure_jinja2(PYTHON_EXE)

    # User-configurable options
    if(NOT DEFINED GAMECOE_GRAPHICS_API)
        set(GAMECOE_GRAPHICS_API "OpenGL")
        message(STATUS "[gamecoe] Using default graphics API - ${GAMECOE_GRAPHICS_API}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_API <API>)\"")
        message(STATUS "[gamecoe] Options are \"OpenGL\" (\"Vulkan\" not supported yet)")
    elseif(GAMECOE_GRAPHICS_API NOT STREQUAL "OpenGL") # TODO: Change when adding Vulkan support
        message(FATAL_ERROR "[gamecoe] ${GAMECOE_GRAPHICS_API} is not a supported graphics API")
    endif()
    
    set(DEFAULT_VERSION_MAJOR OFF)
    if(NOT DEFINED GAMECOE_GRAPHICS_VERSION_MAJOR)
        set(DEFAULT_VERSION_MAJOR ON)
    endif()

    set(DEFAULT_VERSION_MINOR OFF)
    if(NOT DEFINED GAMECOE_GRAPHICS_VERSION_MINOR)
        set(DEFAULT_VERSION_MINOR ON)
    endif()

    if(DEFAULT_VERSION_MAJOR OR DEFAULT_VERSION_MINOR)
        set(GAMECOE_GRAPHICS_VERSION_MAJOR 4)
        set(GAMECOE_GRAPHICS_VERSION_MAJOR 4 PARENT_SCOPE)
        set(GAMECOE_GRAPHICS_VERSION_MINOR 6)
        set(GAMECOE_GRAPHICS_VERSION_MINOR 6 PARENT_SCOPE)
        message(STATUS "[gamecoe] Using default version ${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_VERSION_MAJOR <major>)\" and \"set(GAMECOE_GRAPHICS_VERSION_MINOR <minor>)\"")
        message(STATUS "[gamecoe] Currently supported versions: OpenGL 3.0+") # TODO: Add more in the future
    endif()

    set(CURRENT_GRAPHICS_VERSION "${GAMECOE_GRAPHICS_VERSION_MAJOR}.${GAMECOE_GRAPHICS_VERSION_MINOR}")
    if(CURRENT_GRAPHICS_VERSION VERSION_LESS "3.0")
        message(FATAL_ERROR "[gamecoe] Currently supported versions: OpenGL 3.0+")
    endif()

    if(NOT DEFINED GAMECOE_GRAPHICS_PROFILE)
        if(CURRENT_GRAPHICS_VERSION VERSION_GREATER_EQUAL "3.2")
            set(GAMECOE_GRAPHICS_PROFILE "core")
        else()
            set(GAMECOE_GRAPHICS_PROFILE "compat")
        endif()
        message(STATUS "[gamecoe] Using default profile: ${GAMECOE_GRAPHICS_PROFILE}")
        message(STATUS "[gamecoe] Can be changed with \"set(GAMECOE_GRAPHICS_PROFILE <profile>)\"")
        message(STATUS "[gamecoe] Options are \"core\" (for OpenGL3.2+) and \"compat\"")
    elseif((GAMECOE_GRAPHICS_PROFILE NOT STREQUAL "core") AND (GAMECOE_GRAPHICS_PROFILE NOT STREQUAL "compat"))
        message(FATAL_ERROR "[gamecoe] Only OpenGL \"core\" (version 3.2+) or \"compat\" profiles are supported")
    endif()

    message(STATUS "\n\n\n   [gamecoe] Using ${GAMECOE_GRAPHICS_API} ${CURRENT_GRAPHICS_VERSION} ${GAMECOE_GRAPHICS_PROFILE}\n\n")

    if(GAMECOE_GRAPHICS_API STREQUAL "OpenGL")
        set(GAMECOE_GRAPHICS_API "gl") # for GLAD generator
        set(GAMECOE_USE_OPENGL 1 PARENT_SCOPE)
        set(GAMECOE_USE_VULKAN 0 PARENT_SCOPE)
    # elseif(GAMECOE_GRAPHICS_API STREQUAL "Vulkan")
        # Uncomment when Vulkan will be supported
        # set(GAMECOE_GRAPHICS_API "vulkan") # for GLAD generator
        # set(GAMECOE_USE_OPENGL 0 PARENT_SCOPE)
        # set(GAMECOE_USE_VULKAN 1 PARENT_SCOPE)
    endif()

    if(GAMECOE_GRAPHICS_PROFILE STREQUAL "compat")
        set(GAMECOE_GRAPHICS_PROFILE "compatibility")
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
        COMMAND ${PYTHON_EXE} -m glad ${GLAD_COMMAND_ARGS}
        WORKING_DIRECTORY ${glad_SOURCE_DIR}
        COMMENT ${GLAD_COMMENT}
        VERBATIM
    )

    add_custom_target(glad
        DEPENDS ${GAMECOE_GLAD_SOURCES} ${GAMECOE_GLAD_HEADERS}
    )
endfunction()