function(copy_directory_to_build_root exe dir)
    add_custom_command(TARGET ${exe} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${GAMECOE_SOURCE_ROOT_DIR}/${dir} $<TARGET_FILE_DIR:${exe}>/${dir}
        COMMENT "[gamecoe] Copying ${dir} to build root"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction()