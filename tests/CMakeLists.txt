add_executable(gamecoe_tests
    main.cpp
    entity/entity_tests.cpp
    entity/sparse_set_tests.cpp
    entity/component_pool_tests.cpp
    entity/entities_tests.cpp
    entity/extraction_tests.cpp
    component/transform_tests.cpp
    component/parent_child_tests.cpp
    component/shape_renderer_tests.cpp
)

target_link_libraries(gamecoe_tests
    PRIVATE
        gamecoe
        testcoe
)