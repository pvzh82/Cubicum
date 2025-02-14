#include <cmath>
#include <filesystem>
#include <iostream>

#include "path_manager.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// define  funcs
int width = 1920, height = 1080;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

// comera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
  // initialize glfºw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create window and if statement
  GLFWwindow *window = glfwCreateWindow(width, height, ":)", NULL, NULL);
  if (window == NULL)

  {
    std::cout << "Failed to create window :c" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // initialize glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD :c" << std::endl;
    return -1;
  }
  // size of rendering window
  glViewport(0, 0, width, height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glClearColor(0.455f, 0.701f, 1.0f, 0.8f);

  // triangle test
  float vertices[] = {

    // pos                   //texture
    0.0f,  0.0f, 0.0f,        0.0f, 0.0f,
    1.0f,  0.0f, 0.0f,        1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,        1.0f, 1.0f,
    0.0f,  1.0f, 0.0f,        0.0f, 1.0f,

    0.0f,  0.0f, 1.0f,        0.0f, 0.0f,
    1.0f,  0.0f, 1.0f,        1.0f, 0.0f,
    1.0f,  1.0f, 1.0f,        1.0f, 1.0f,
    0.0f,  1.0f, 1.0f,        0.0f, 1.0f,

    0.0f,  0.0f, 0.0f,        0.0f, 0.0f,
    0.0f,  0.0f, 1.0f,        1.0f, 0.0f,
    0.0f,  1.0f, 1.0f,        1.0f, 1.0f,
    0.0f,  1.0f, 0.0f,        0.0f, 1.0f,

    1.0f,  0.0f, 1.0f,        0.0f, 0.0f,
    1.0f,  0.0f, 1.0f,        1.0f, 0.0f,
    1.0f,  1.0f, 1.0f,        1.0f, 1.0f,
    1.0f,  1.0f, 0.0f,        0.0f, 1.0f,

    0.0f,  0.0f, 0.0f,        0.0f, 0.0f,
    1.0f,  0.0f, 0.0f,        1.0f, 0.0f,
    1.0f,  0.0f, 1.0f,        1.0f, 1.0f,
    0.0f,  0.0f, 1.0f,        0.0f, 1.0f,

    0.0f,  1.0f, 0.0f,        0.0f, 0.0f,
    1.0f,  1.0f, 0.0f,        1.0f, 0.0f,
    1.0f,  1.0f, 1.0f,        1.0f, 1.0f,
    0.0f,  1.0f, 1.0f,        0.0f, 1.0f,
    };
    unsigned int indices[] = {
        0, 1, 2, 0, 2, 3,

        4, 5, 6, 4, 6, 7,

        8, 9, 10, 8, 9, 11,

        12, 13, 14, 12, 14, 15,

        16, 17, 18, 16, 18, 19,

        20, 21, 22, 20, 22, 23
    };

  // shader getpath & compile
  // rootPath = "../";
  std::string vertexShaderPath =
      PathManager::getShaderPath("vertex_shader.glsl");
  std::string fragmentShaderPath =
      PathManager::getShaderPath("fragment_shader.glsl");

  shader myShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
  myShader.useShader();

  // load vertices data into gpu
  // vao

  unsigned int VBO, VAO, EBO;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  // now vbo
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
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

  unsigned char *data =
      stbi_load(grassBlockPath.c_str(), &texwidth, &texheight, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "couldnt load texture :c" << std::endl;
  }

  glm::mat4 projection = glm::perspective(
      glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
  myShader.setMat4("projection", projection);

  stbi_image_free(data);

  glEnable(GL_DEPTH_TEST);

  // main loop: call funcs defined before with parameter "window" to create the
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

    // camera/view tranformation
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    myShader.setMat4("view", view);

    // render boxes
    glBindVertexArray(VAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f)); // Move block in front of camera
    myShader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // cal events and swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // at end remove to free memories
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
  return 0;
}

// input func
void processInput(GLFWwindow *window) {
  const float cameraSpeed = 0.05f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cameraPos += cameraSpeed * cameraFront;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cameraPos -= cameraSpeed * cameraFront;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  }
}
// window func
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}