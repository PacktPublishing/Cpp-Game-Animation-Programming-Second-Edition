/* OpenGL */
#pragma once
#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GltfNode.h"

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

enum class ikMode {
  off = 0,
  ccd
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

  float rdViewAzimuth = 0.0f;
  float rdViewElevation = 0.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(-0.5f, 2.5f, 6.0f);

  bool rdDrawGltfModel = true;
  bool rdDrawSkeleton = true;
  skinningMode rdGPUDualQuatVertexSkinning = skinningMode::linear;

  bool rdPlayAnimation = true;
  std::vector<std::string> rdClipNames{};
  int rdAnimClip = 0;
  int rdAnimClipSize = 0;
  float rdAnimSpeed = 1.0f;
  float rdAnimTimePosition = 0.0f;
  float rdAnimEndTime = 0.0f;
  int rdModelNodeCount = 0;

  replayDirection rdAnimationPlayDirection = replayDirection::forward;

  float rdAnimBlendFactor = 1.0f;

  blendMode rdBlendingMode = blendMode::fadeinout;
  int rdCrossBlendDestAnimClip = 0;
  float rdAnimCrossBlendFactor = 0.0f;

  int rdSkelSplitNode = 0;
  std::vector<std::string> rdSkelNodeNames{};

  ikMode rdIkMode = ikMode::off;
  int rdIkIterations = 10;
  glm::vec3 rdIkTargetPos = glm::vec3(0.0f, 3.0f, 1.0f);
  int rdIkEffectorNode = 0;
  int rdIkRootNode = 0;
};
