#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/loader.h"
#include "../include/renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../include/imfilebrowser.h"


namespace fs = std::filesystem;

// Global mesh and camera
std::vector<Vertex> emptyVerts;
std::vector<unsigned int> emptyInds;
Renderer mesh(emptyVerts, emptyInds);
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), 0.0f, 0.0f, 3.0f, 0.5f, 90.0f);

// State
constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseControlEnabled = true;
bool meshLoaded = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
std::string file;
ImGui::FileBrowser fileDialog;

// Forward declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window);

// Mesh loader
Renderer load_shader_and_mesh(std::string filePath) {
    Loader loader;
    loader.GetVertices(filePath);
    std::cout << "Loaded mesh: " << loader.vertices.size() << " vertices, "
        << loader.indices.size() << " indices\n";
    return Renderer(loader.vertices, loader.indices);
}

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Mesh", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    Shader shader("assets/shaders/vertex.vert", "assets/shaders/fragment.frag");
    stbi_set_flip_vertically_on_load(true);

    // Default camera setup
    camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
    camera.pitch = 0.0f;
    camera.yaw = -90.0f;
    camera.updateCamera();

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Button("Open File")) {
            fileDialog.Open();
        }

        fileDialog.Display();

        if (fileDialog.HasSelected()) {
            file = fileDialog.GetSelected().string();
            mesh = load_shader_and_mesh(file);
            meshLoaded = true;
            fileDialog.ClearSelected();

            // Load texture from same directory
            fs::path filePath(file);
            fs::path texturePath = filePath.parent_path() / "diffuse.jpg";
            std::string textureFile = texturePath.string();

            int width, height, nrChannels;
            unsigned char* data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glUseProgram(shader.ID);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                glUniform1i(glGetUniformLocation(shader.ID, "materialDiffuse"), 0);

                stbi_image_free(data);
            }
            else {
                std::cerr << "Failed to load texture: " << textureFile << "\n";
            }

            // Reset camera to focus on origin
            camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
            camera.pitch = 0.0f;
            camera.yaw = -90.0f;
            camera.updateCamera();
        }

        ImGui::Begin("File Info");
        ImGui::TextWrapped("Loaded file: %s", file.c_str());
        ImGui::End();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
        modelMat = glm::scale(modelMat, glm::vec3(0.01f, 0.01f, 0.01f));

        glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f),
            static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT),
            0.1f, 100.0f);

        glm::mat4 viewMat = camera.lookAtMatrix();

        shader.use();
        shader.setMat4("model", modelMat);
        shader.setMat4("projection", projectionMat);
        shader.setMat4("view", viewMat);
        shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
        shader.setVec3("viewPos", camera.position);

        if (meshLoaded && !mesh.vertices.empty() && !mesh.indices.empty()) {
            mesh.DrawMesh();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseControlEnabled = false;
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseControlEnabled = true;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!mouseControlEnabled) return;

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
