#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "OGLRenderData.h"

class Model {
  public:
    void init();

    OGLMesh getVertexData();

  private:
    OGLMesh mVertexData;
};
