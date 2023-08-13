#pragma once
struct ModelSettings {
  glm::vec2 msWorldPosition = glm::vec2(0.0f);
  glm::vec3 msWorldRotation = glm::vec3(0.0f);

  bool msDrawModel = true;
  bool msDrawSkeleton = false;
  skinningMode msVertexSkinningMode = skinningMode::linear;

  bool msPlayAnimation = true;
  replayDirection msAnimationPlayDirection = replayDirection::forward;
  int msAnimClip = 0;
  float msAnimSpeed = 1.0f;
  float msAnimTimePosition = 0.0f;
  float msAnimEndTime = 0.0f;

  blendMode msBlendingMode = blendMode::fadeinout;
  float msAnimBlendFactor = 1.0f;
  int msCrossBlendDestAnimClip = 0;
  float msAnimCrossBlendFactor = 0.0f;
  int msSkelSplitNode = 0;

  std::vector<std::string> msClipNames{};
  /* saved per instance as the node list is created here */
  std::vector<std::string> msSkelNodeNames{};

  ikMode msIkMode = ikMode::off;
  int msIkIterations = 10;
  glm::vec3 msIkTargetPos = glm::vec3(0.0f, 3.0f, 1.0f);
  glm::vec3 msIkTargetWorldPos = glm::vec3(0.0f, 0.0f,01.0f);
  int msIkEffectorNode = 0;
  int msIkRootNode = 0;
};

