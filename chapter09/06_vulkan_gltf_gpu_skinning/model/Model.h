#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "VkRenderData.h"

class Model {
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};
