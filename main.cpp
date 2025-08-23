#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <gamecoe_config.h>
#include <iostream>
#include <cmath>
#include <optional>
#include <gamecoe/shader.hpp>
#include <logcoe.hpp>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

struct garbage_collector
{
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    unsigned int m_texture1 = 0;
    unsigned int m_texture2 = 0;
    std::optional<gamecoe::shader> *m_shader = nullptr;
    ~garbage_collector()
    {
        glDeleteTextures(1, &m_texture1);
        glDeleteTextures(1, &m_texture2);
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        if (m_shader)
            m_shader->reset();
        logcoe::shutdown();
        glfwTerminate();
    }
};

int main()
{
    garbage_collector gc;
    logcoe::initialize(logcoe::LogLevel::INFO, "gamecoe");
    // glfw: initialize and configure
    if (!glfwInit())
    {
        logcoe::error("GLFW failed to initialize");
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
    if (!window)
    {
        logcoe::error("Failed to create GLFW window");
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#if GAMECOE_USE_OPENGL
    // glad: load all opengl function pointers
    if (!gladLoadGL(glfwGetProcAddress))
    {
        logcoe::error("Failed to initialize GLAD");
        return -1;
    }
#endif

    // build and compile our shader program
    // ------------------------------------

    std::optional<gamecoe::shader> shader;
    try
    {
        shader.emplace("shader.vert", "shader.frag");
    }
    catch (const std::runtime_error &e)
    {
        logcoe::error(std::string(e.what()));
        return -1;
    }

    gc.m_shader = &shader;

    // vertex data and buffers
    float vertices[] = {
        // positions     // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    gc.m_VAO = VAO;
    glGenBuffers(1, &VBO);
    gc.m_VBO = VBO;
    glGenBuffers(1, &EBO);
    gc.m_EBO = EBO;

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // first texture
    unsigned int texture1;
    glGenTextures(1, &texture1);
    gc.m_texture1 = texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/images/container.jpg", &width, &height, &nrChannels, 0);
    if (!data)
    {
        logcoe::error("Failed to load texture1");
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // second texture
    unsigned int texture2;
    glGenTextures(1, &texture2);
    gc.m_texture2 = texture2;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load("assets/images/awesomeface.png", &width, &height, &nrChannels, 0);
    if (!data)
    {
        logcoe::error("Failed to load texture2");
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    shader->use();
    shader->set("texture1", 0);
    shader->set("texture2", 1);

    // wire mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // transformation
        glm::mat4 trans(1.0f);
        float time = glfwGetTime();
        trans = glm::translate(trans, glm::vec3(glm::cos(time), glm::sin(time), 0.0f));
        trans = glm::rotate(trans, time, glm::vec3(0.0f, 0.0f, 1.0f));

        shader->use();
        shader->set("transform", trans);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}