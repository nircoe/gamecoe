add_executable(gamecoe_tests
    main.cpp
    support/scene_id.cpp
    colorcoe_tests.cpp
    utils/paths_tests.cpp
    core/window_tests.cpp
    core/game_tests.cpp
    graphics/buffer_tests.cpp
    graphics/vertex_array_tests.cpp
    graphics/shader_tests.cpp
    graphics/texture_tests.cpp
    entity/entity_tests.cpp
    entity/sparse_set_tests.cpp
    entity/component_pool_tests.cpp
    entity/entities_tests.cpp
    entity/extraction_tests.cpp
    entity/command_buffer_tests.cpp
    component/transform_tests.cpp
    component/parent_child_tests.cpp
    component/scene_tag_tests.cpp
    component/shape_renderer_tests.cpp
    component/shape_collider_tests.cpp
)

target_link_libraries(gamecoe_tests
    PRIVATE
        gamecoe
        testcoe
        glfw
)

target_include_directories(gamecoe_tests PRIVATE ${GAMECOE_GLAD_OUTPUT_DIR}/include)
target_include_directories(gamecoe_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})