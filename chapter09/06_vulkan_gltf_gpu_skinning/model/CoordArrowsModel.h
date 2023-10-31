/* coordinate arrows */
#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "VkRenderData.h"

class CoordArrowsModel {
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};
