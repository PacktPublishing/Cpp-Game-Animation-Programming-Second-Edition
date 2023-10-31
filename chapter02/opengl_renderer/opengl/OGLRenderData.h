/* OpenGL */
#pragma once
#include <vector>

#include <glm/glm.hpp>

struct OGLVertex {
  glm::vec3 position;
  glm::vec2 uv;
};

struct OGLMesh {
  std::vector<OGLVertex> vertices;
};

