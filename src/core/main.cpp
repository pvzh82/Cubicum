#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "../render/camera.h"
#include "../render/world.h"
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
Camera camera(glm::vec3(0.0f, 50.0f, 50.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
  // init glfw
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // set opengl version :D
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
  glfwMakeContextCurrent(window);

  // init GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD :c" << std::endl;
    return -1;
  }

  // setup callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // shader getpath & compile
  std::string vertexShaderPath =
      PathManager::getShaderPath("vertex_shader.glsl");
  std::string fragmentShaderPath =
      PathManager::getShaderPath("fragment_shader.glsl");

  Shader shader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
  shader.useShader();

  // init world
  World world;

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
  std::string grassBlockPath = PathManager::getTexturePath("dirt.png");

  unsigned char* data =
      stbi_load(grassBlockPath.c_str(), &texwidth, &texheight, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "couldnt load texture :c" << std::endl;
  }

  stbi_image_free(data);

  glEnable(GL_DEPTH_TEST);

  // fps
  double lastTime = glfwGetTime();
  int frameCount = 0;

  // main loop
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;

    // checkfps
    double currentTime = glfwGetTime();
    frameCount++;
    if (currentTime - lastTime >= 1.0f) {
      std::cout << "FPS: " << frameCount << "\n";
      frameCount = 0;
      lastTime = currentTime;
    }

    // input
    processInput(window);
    // rendering stuff
    glClearColor(0.455f, 0.701f, 1.0f, 0.8f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);
    // activate shader
    shader.useShader();
    shader.setInt("ourTexture", 0);

    // projection
    glm::mat4 projection = glm::perspective(
        glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("projection", projection);

    // camera/view tranformation
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", view);

    // update world
    world.Update(camera.Position.x, camera.Position.y, camera.Position.z, 0);

    // render world
    world.Render(shader);

    // call events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // cleanup
  system("clear");
  glfwTerminate();
  return 0;
}

// input func
void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.FORWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.BACKWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.LEFT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.RIGHT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.UP, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camera.ProcessKeyboard(camera.DOWN, deltaTime);
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
