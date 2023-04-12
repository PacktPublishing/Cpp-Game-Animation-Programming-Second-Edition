#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Model {
  public:
    OGLMesh getVertexData();

  private:
    void init();
    OGLMesh mVertexData;
};
