#include <testcoe.hpp>
#include <iostream>
#include <string>

int printHelp()
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
    std::cout << "  GraphicsBufferTests      - Graphics buffer class tests" << std::endl;
    std::cout << "  VertexArrayTests         - Vertex array class tests" << std::endl;
    std::cout << "  ShaderTests              - Shader class tests" << std::endl;
    std::cout << "  TextureTests             - Texture class tests" << std::endl;
    std::cout << "  EntityTests              - Entity handle tests" << std::endl;
    std::cout << "  SparseSetTests           - Sparse set data structure tests" << std::endl;
    std::cout << "  ComponentPoolTests       - Component pool wrapper tests" << std::endl;
    std::cout << "  EntitiesTests            - Entities manager tests" << std::endl;
    std::cout << "  CommandBufferTests       - Command buffer tests" << std::endl;
    std::cout << "  TransformTests           - Transform component tests" << std::endl;
    std::cout << "  ParentTests              - Parent component tests" << std::endl;
    std::cout << "  ChildrenTests            - Children component tests" << std::endl;
    std::cout << "  SceneTagTests            - Scene tag component tests" << std::endl;
    std::cout << "  ShapeRendererTests       - Shape renderer component tests" << std::endl;
    std::cout << "  ShapeColliderTests       - Shape collider component tests" << std::endl;
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
    std::cout << "Testing Entity Module: entity, sparse_set, component_pool, entities, command_buffer" << std::endl;
    std::cout << "Testing Component Module: transform, parent, children, scene_tag, shape_renderer, shape_collider" << std::endl;
    std::cout << std::endl;

    testcoe::init(&argc, argv);

    bool askForAll = false;
    std::string suiteName;
    std::string testName;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
            return printHelp();
        else if (arg == "--all")
            askForAll = true;
        else if (!askForAll && arg.substr(0, 8) == "--suite=")
            suiteName = arg.substr(8);
        else if (!askForAll && arg.substr(0, 7) == "--test=")
        {
            std::string fullTest = arg.substr(7);
            size_t dotPos = fullTest.find('.');
            if (dotPos != std::string::npos)
            {
                suiteName = fullTest.substr(0, dotPos);
                testName = fullTest.substr(dotPos + 1);
            }
        }
    }

    if (askForAll || (testName.empty() && suiteName.empty()))
    {
        std::cout << "Running all gamecoe tests..." << std::endl;
        return testcoe::run();
    }

    if (!testName.empty())
    {
        std::cout << "Running test: " << suiteName << "." << testName << std::endl;
        return testcoe::run_test(suiteName, testName);
    }

    if (!suiteName.empty())
    {
        std::cout << "Running suite: " << suiteName << std::endl;
        return testcoe::run_suite(suiteName);
    }

    return 0;
}