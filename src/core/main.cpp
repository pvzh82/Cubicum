#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <filesystem>
#include <iostream>


#include "../render/camera.h"
#include "path_manager.h"
#include "shader.h"




// define  funcs

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// screen settings
int width = 1920, height = 1080;

// comera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
  // initialize glfw
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Set OpenGL version to 3.3 Core Profile
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create window
  GLFWwindow* window = glfwCreateWindow(width, height, ":)", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create window :c" << std::endl;
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD :c" << std::endl;
    return -1;
  }

  // Set up callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // cube def
  // clang-format off
  float vertices[] = {
    // pos                   //texture
    // Front face
    0.0f, 0.0f, 0.0f,        0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,        1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,        1.0f, 1.0f,
    0.0f, 1.0f, 0.0f,        0.0f, 1.0f,

    // Back face
    0.0f, 0.0f, 1.0f,        0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,        1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,        1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,        0.0f, 1.0f,

    // Left face
    0.0f, 0.0f, 0.0f,        0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,        1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,        1.0f, 1.0f,
    0.0f, 1.0f, 0.0f,        0.0f, 1.0f,

    // Right face
    1.0f, 0.0f, 0.0f,        0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,        1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,        1.0f, 1.0f,
    1.0f, 1.0f, 0.0f,        0.0f, 1.0f,

    // Bottom face
    0.0f, 0.0f, 0.0f,        0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,        1.0f, 0.0f,
    1.0f, 0.0f, 1.0f,        1.0f, 1.0f,
    0.0f, 0.0f, 1.0f,        0.0f, 1.0f,

    // Top face
    0.0f, 1.0f, 0.0f,        0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,        1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,        1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,        0.0f, 1.0f,
  };

  unsigned int indices[] = {
    // Front face
    0, 1, 2, 2, 3, 0,

    // Back face
    4, 5, 6, 6, 7, 4,

    // Left face
    8, 9, 10, 10, 11, 8,

    // Right face
    12, 13, 14, 14, 15, 12,

    // Bottom face
    16, 17, 18, 18, 19, 16,

    // Top face
    20, 21, 22, 22, 23, 20
  };
  // clang-format on

  // shader getpath & compile
  std::string vertexShaderPath = PathManager::getShaderPath("vertex_shader.glsl");
  std::string fragmentShaderPath = PathManager::getShaderPath("fragment_shader.glsl");

  shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
  myShader.useShader();

  // load vertices data into gpu
  unsigned int VBO, VAO, EBO;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  // now vbo
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // texture time!
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glActiveTexture(GL_TEXTURE0);

  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  stbi_set_flip_vertically_on_load(true);

  // load and generate
  int texwidth, texheight, nrChannels;
  std::string grassBlockPath = PathManager::getTexturePath("grass.png");

  unsigned char* data = stbi_load(grassBlockPath.c_str(), &texwidth, &texheight, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "couldnt load texture :c" << std::endl;
  }

  stbi_image_free(data);

  glEnable(GL_DEPTH_TEST);

  // main loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;

    // input
    processInput(window);
    // rendering stuff
    glClearColor(0.455f, 0.701f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);
    // activate shader
    myShader.useShader();

    // projection
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    myShader.setMat4("projection", projection);

    // camera/view tranformation
    glm::mat4 view = camera.GetViewMatrix();
    myShader.setMat4("view", view);

    // render boxes
    glBindVertexArray(VAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));  // Move block in front of camera
    myShader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // cal events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

  glfwTerminate();
  return 0;
}

// input func
void processInput(GLFWwindow* window) {
  const float cameraSpeed = 0.05f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.ProcessKeyboard(camera.FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.ProcessKeyboard(camera.BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.ProcessKeyboard(camera.LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.ProcessKeyboard(camera.RIGHT, deltaTime);
  }
}

// window func
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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

  camera.ProcessMouseMovement(xoffset, yoffset, false);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
