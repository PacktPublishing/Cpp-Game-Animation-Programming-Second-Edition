/* generate spline plus */
#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "VkRenderData.h"

class SplineModel {
  public:
  VkMesh createVertexData(int numSplinePoints,
    glm::vec3 startVertex, glm::vec3 startTangent,
    glm::vec3 endVertex, glm::vec3 endTangent);
};
