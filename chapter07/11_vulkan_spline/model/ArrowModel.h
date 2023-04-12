/* a simple, line based arrow */
#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "VkRenderData.h"

class ArrowModel {
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};
