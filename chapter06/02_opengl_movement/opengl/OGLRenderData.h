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

  int rdFieldOfView = 90;

  bool rdUseChangedShader = false;

  float rdFrameTime = 0.0f;
  float rdMatrixGenerateTime = 0.0f;
  float rdUploadToUBOTime = 0.0f;
  float rdUIGenerateTime = 0.0f;
  float rdUIDrawTime = 0.0f;

  float rdCameraOrientation = 0.0f;
  float rdCameraHeadAngle = 0.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(0.0f);
};
