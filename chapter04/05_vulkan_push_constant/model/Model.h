#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "VkRenderData.h"

class Model {
  public:
    void init();

    VkMesh getVertexData();

  private:
    VkMesh mVertexData;
};
