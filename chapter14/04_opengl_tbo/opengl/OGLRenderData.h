/* OpenGL */
#pragma once
#include <vector>
#include <string>

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

enum class skinningMode {
  linear = 0,
  dualQuat
};

enum class replayDirection {
  forward = 0,
  backward
};

enum class blendMode {
  fadeinout = 0,
  crossfade,
  additive
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

  float rdViewAzimuth = 15.0f;
  float rdViewElevation = -25.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(-10.0f, 16.0f, 35.0f);

  int rdNumberOfInstances = 0;
  int rdCurrentSelectedInstance = 0;
};
