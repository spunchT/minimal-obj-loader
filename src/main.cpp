#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "include/shader.h"
#include "include/camera.h"
#include "include/loader.h"
#include "include/renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
    // screen dimensions
    constexpr unsigned int SCR_WIDTH = 800;
    constexpr unsigned int SCR_HEIGHT = 600;

    // camera and input state
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, 0.0f, 3.0f, 0.5f, 90.0f);
    float lastX = SCR_WIDTH / 2.0f;
    float lastY = SCR_HEIGHT / 2.0f;
    bool firstMouse = true;

    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
}

// forward declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

int main()
{
    // initialize glfw and create window
    if (!glfwInit()) {
        std::cout << "failed to initialize glfw\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Mesh", nullptr, nullptr);
    if (!window) {
        std::cout << "failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "failed to initialize glad\n";
        return -1;
    }

    // opengl state
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // load shader and mesh
    Shader shader("assets/shaders/vertex.vert", "assets/shaders/fragment.frag");
    Loader loader;
    loader.GetVertices("assets/models/human.obj");
    Renderer mesh(loader.vertices, loader.indices);

    // model and projection setup
    glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
    modelMat = glm::scale(modelMat, glm::vec3(0.1f));

    glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f),
                                               static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
                                               0.1f, 100.0f);

    shader.use();
    shader.setMat4("model", modelMat);
    shader.setMat4("projection", projectionMat);

    // initial camera pose
    camera.position = glm::vec3(1.5f, 1.5f, 3.0f);
    camera.yaw = -135.0f;
    camera.pitch = -20.0f;
    camera.updateCamera();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMat = camera.lookAtMatrix();
        shader.use();
        shader.setMat4("view", viewMat);
        shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
        shader.setVec3("viewPos", camera.position);

        mesh.DrawMesh();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// update viewport on resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// handle keyboard and cursor mode toggles and movement
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}

// update camera orientation from mouse movement
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// update camera zoom from scroll wheel
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
