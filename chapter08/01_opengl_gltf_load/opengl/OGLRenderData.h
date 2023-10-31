/* OpenGL */
#pragma once
#include <vector>

#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct OGLVertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 uv;
};

struct OGLMesh {
  std::vector<OGLVertex> vertices;
};

struct OGLRenderData {
  GLFWwindow *rdWindow = nullptr;

  int rdWidth = 0;
  int rdHeight = 0;

  unsigned int rdTriangleCount = 0;
  unsigned int rdGltfTriangleCount = 0;

  int rdFieldOfView = 60;

  float rdFrameTime = 0.0f;
  float rdMatrixGenerateTime = 0.0f;
  float rdUploadToVBOTime = 0.0f;
  float rdUploadToUBOTime = 0.0f;
  float rdUIGenerateTime = 0.0f;
  float rdUIDrawTime = 0.0f;

  int rdMoveForward = 0;
  int rdMoveRight = 0;
  int rdMoveUp = 0;

  float rdTickDiff = 0.0f;

  float rdViewAzimuth = 230.0f;
  float rdViewElevation = -15.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(6.0f, 4.0f, -4.0f);

  bool rdDrawWorldCoordArrows = true;
  bool rdDrawModelCoordArrows = true;
  bool rdResetAngles = false;

  int rdRotXAngle = 0;
  int rdRotYAngle = 0;
  int rdRotZAngle = 0;
};
