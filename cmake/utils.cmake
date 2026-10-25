function(gamecoe_copy_assets_to_build_root exe)
    if(DEFINED GAMECOE_SOURCE_ROOT_DIR)
        set(GAMECOE_ASSETS_PATH ${GAMECOE_SOURCE_ROOT_DIR}/assets)
    elseif(DEFINED gamecoe_SOURCE_DIR)
        set(GAMECOE_ASSETS_PATH ${gamecoe_SOURCE_DIR}/assets)
    else()
        message(FATAL_ERROR "[gamecoe] Cannot determine gamecoe source directory")
    endif()

    add_custom_command(TARGET ${exe} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${GAMECOE_ASSETS_PATH} $<TARGET_FILE_DIR:${exe}>/gamecoe
        COMMENT "[gamecoe] Copying gamecoe assets to build root"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()

function(gamecoe_enable_strict_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra -Wpedantic)
    endif()
endfunction()
