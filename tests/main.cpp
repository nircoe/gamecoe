#include <testcoe.hpp>
#include <iostream>
#include <string>

int print_help()
{
    std::cout << "Usage: ./gamecoe_tests [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --help           Display this help message" << std::endl;
    std::cout << "  --all            Run all tests (default)" << std::endl;
    std::cout << "  --suite=NAME     Run only the specified test suite" << std::endl;
    std::cout << "  --test=SUITE.TEST Run only the specified test" << std::endl;
    std::cout << std::endl;
    std::cout << "Available test suites:" << std::endl;
    std::cout << "  ColorcoeTests            - Color hex decoding tests" << std::endl;
    std::cout << "  PathsTests               - Executable path resolution tests" << std::endl;
    std::cout << "  WindowTests              - Window class tests" << std::endl;
    std::cout << "  GameTests                - Game class tests" << std::endl;
    std::cout << "  GameMoveTests            - Game move constructor test" << std::endl;
    std::cout << "  GraphicsBufferTests      - Graphics buffer class tests" << std::endl;
    std::cout << "  VertexArrayTests         - Vertex array class tests" << std::endl;
    std::cout << "  ShaderTests              - Shader class tests" << std::endl;
    std::cout << "  TextureTests             - Texture class tests" << std::endl;
    std::cout << "  EntityTests              - Entity handle tests" << std::endl;
    std::cout << "  SparseSetTests           - Sparse set data structure tests" << std::endl;
    std::cout << "  ComponentPoolTests       - Component pool wrapper tests" << std::endl;
    std::cout << "  EntitiesTests            - Entities manager tests" << std::endl;
    std::cout << "  ExtractionTests          - Extraction system tests" << std::endl;
    std::cout << "  CommandBufferTests       - Command buffer tests" << std::endl;
    std::cout << "  TransformTests           - Transform component tests" << std::endl;
    std::cout << "  ParentTests              - Parent component tests" << std::endl;
    std::cout << "  ChildrenTests            - Children component tests" << std::endl;
    std::cout << "  SceneTagTests            - Scene tag component tests" << std::endl;
    std::cout << "  ShapeRendererTests       - Shape renderer component tests" << std::endl;
    std::cout << "  ShapeColliderTests       - Shape collider component tests" << std::endl;
    std::cout << "  SystemTests              - System entry and access set tests" << std::endl;
    std::cout << "  SchedulerTests           - System scheduler ordering tests" << std::endl;
    std::cout << std::endl;
    std::cout << "Example usage:" << std::endl;
    std::cout << "  ./gamecoe_tests --suite=EntityTests" << std::endl;
    std::cout << "  ./gamecoe_tests --test=EntityTests.CreateValidEntity" << std::endl;
    return 0;
}

int main(int argc, char **argv)
{
    std::cout << "====================================================" << std::endl;
    std::cout << "                 gamecoe Test Suite                 " << std::endl;
    std::cout << "====================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Comprehensive testing for gamecoe." << std::endl;
    std::cout << "Testing Core Module: window, game" << std::endl;
    std::cout << "Testing Graphics Module: buffer, vertex_array, shader, texture" << std::endl;
    std::cout << "Testing Entity Module: entity, sparse_set, component_pool, entities, extraction, command_buffer" << std::endl;
    std::cout << "Testing Component Module: transform, parent, children, scene_tag, shape_renderer, shape_collider" << std::endl;
    std::cout << "Testing System Module: systems::entry, systems::create, systems::scheduler" << std::endl;
    std::cout << std::endl;

    bool ask_for_all = false;
    std::string suite_name;
    std::string test_name;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return print_help();
        else if (arg == "--all")
            ask_for_all = true;
        else if (!ask_for_all && arg.substr(0, 8) == "--suite=")
        {
            suite_name = arg.substr(8);
            if (suite_name.empty())
            {
                std::cerr << "Error: --suite= requires a suite name" << std::endl;
                return 1;
            }
        }
        else if (!ask_for_all && arg.substr(0, 7) == "--test=")
        {
            std::string full_test = arg.substr(7);
            size_t dot_pos = full_test.find('.');
            if (dot_pos == std::string::npos)
            {
                std::cerr << "Error: --test= requires SUITE.TEST" << std::endl;
                return 1;
            }
            suite_name = full_test.substr(0, dot_pos);
            test_name = full_test.substr(dot_pos + 1);
        }
    }

    testcoe::init(&argc, argv);

    // Default fork-based death tests inherit any thread state a test already spun up (e.g.
    // GLFW/logcoe via game::create()) - a held lock at fork time can deadlock the child.
    // threadsafe re-execs fresh instead, so it's set globally rather than per-test.
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    if (ask_for_all || (test_name.empty() && suite_name.empty()))
    {
        std::cout << "Running all gamecoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!test_name.empty())
    {
        std::cout << "Running test: " << suite_name << "." << test_name << std::endl;
        return testcoe::run_test(suite_name, test_name);
    }

    if (!suite_name.empty())
    {
        std::cout << "Running suite: " << suite_name << std::endl;
        return testcoe::run_suite(suite_name);
    }

    return 0;
}
