#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>

class Camera {
 public:
  // main enum
  enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

  // const declaration
  static const float YAW;
  static const float PITCH;
  static const float SPEED;
  static const float SENSITIVITY;
  static const float ZOOM;

  // camera attributes variables
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  // declare big construct
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
         float yaw = YAW,
         float pitch = PITCH);

  Camera(float posX,
         float posY,
         float posZ,
         float upX,
         float upY,
         float upZ,
         float yaw,
         float pitch);

  glm::mat4 GetViewMatrix();

  void ProcessKeyboard(Camera_Movement direction, float deltaTime);
  void ProcessMouseMovement(float xoffset,
                            float yoffset,
                            GLboolean constrainPitch);
  void ProcessMouseScroll(float yoffset);

 private:
  void updateCameraVectors();
};

#endif
