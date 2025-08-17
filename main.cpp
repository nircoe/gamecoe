#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <gamecoe_config.h>
#include <iostream>
#include <cmath>
#include <optional>
#include <gamecoe/shader.hpp>
#include <logcoe.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int shutdown()
{
    logcoe::shutdown();
    glfwTerminate();
    return -1;
}

int main()
{
    logcoe::initialize(logcoe::LogLevel::INFO, "gamecoe");
    // glfw: initialize and configure
    if(!glfwInit()) 
    {
        logcoe::error("GLFW failed to initialize");
        logcoe::shutdown();
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GAMECOE_GRAPHICS_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GAMECOE_GRAPHICS_VERSION_MINOR);
    
#if GAMECOE_USE_OPENGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GAMECOE_GRAPHICS_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
    if(!window)
    {
        logcoe::error("Failed to create GLFW window");
        return shutdown();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#if GAMECOE_USE_OPENGL
    // glad: load all opengl function pointers
    if(!gladLoadGL(glfwGetProcAddress))
    {
        logcoe::error("Failed to initialize GLAD");
        return shutdown();
    }
#endif
    
    // build and compile our shader program
    // ------------------------------------

    std::optional<gamecoe::shader> shader;
    try
    {
        shader.emplace("shader.vert", "shader.frag");
    }
    catch(const std::runtime_error &e) 
    {
        logcoe::error(std::string(e.what()));
        return shutdown();
    }

    // vertex data and buffers
    float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f    // top
    };
    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 2, // first triangle
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
    glBindVertexArray(0);

    // wire mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if(shader)
            shader->use();

        // float timeValue = glfwGetTime();
        // float redValue = (std::sinf(timeValue) / 2.0f) + 0.5f;
        // int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        // glUniform4f(vertexColorLocation, redValue, 0.0f, 0.0f, 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    if(shader)
        shader.reset();
    shutdown();
    return 0;
}