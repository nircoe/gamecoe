function(generate_config_header)
    set(GAMECOE_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/config")
    set(GAMECOE_CONFIG_DIR ${GAMECOE_CONFIG_DIR} PARENT_SCOPE)

    file(MAKE_DIRECTORY ${GAMECOE_CONFIG_DIR})

    configure_file(
        ${GAMECOE_SOURCE_ROOT_DIR}/cmake/gamecoe_config.hpp.in
        ${GAMECOE_CONFIG_DIR}/gamecoe_config.hpp
        @ONLY
    )
endfunction()
