/* coordinate arrows */
#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "OGLRenderData.h"

class CoordArrowsModel {
  public:
    OGLMesh getVertexData();

  private:
    void init();
    OGLMesh mVertexData;
};
