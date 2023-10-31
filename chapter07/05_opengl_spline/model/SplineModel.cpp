#include <glm/gtx/spline.hpp>

#include "SplineModel.h"
#include "Logger.h"

OGLMesh SplineModel::createVertexData(int numSplinePoints,
    glm::vec3 startVertex, glm::vec3 startTangent,
    glm::vec3 endVertex, glm::vec3 endTangent) {

  OGLMesh mVertexData;

  mVertexData.vertices.resize(numSplinePoints * 2 + 4);

  /* draw the tangents as lines */
  mVertexData.vertices[0].color = glm::vec3(0.0f, 0.0f, 0.0f);
  mVertexData.vertices[0].position = startVertex;
  mVertexData.vertices[1].color = glm::vec3(0.0f, 0.0, 0.0f);
  mVertexData.vertices[1].position = startVertex + startTangent;
  mVertexData.vertices[2].color = glm::vec3(0.8f, 0.8, 0.8f);
  mVertexData.vertices[2].position = endVertex;
  mVertexData.vertices[3].color = glm::vec3(0.8f, 0.8f,0.8f);
  mVertexData.vertices[3].position = endVertex + endTangent;

  /* draw tangent as line segments */
  float offset = 1.0f / static_cast<float>(numSplinePoints);
  float value = 0.0f;

  for (int i = 5; i < numSplinePoints * 2 + 4; i += 2) {
    mVertexData.vertices[i - 1].position = glm::hermite(
      startVertex, startTangent, endVertex,endTangent, value);
    mVertexData.vertices[i - 1].color = glm::vec3(value);

    /* keep color of line segment */
    mVertexData.vertices[i].color = glm::vec3(value);

    value += offset;
    mVertexData.vertices[i].position = glm::hermite(
      startVertex, startTangent, endVertex,endTangent, value);
  }
  mVertexData.vertices[numSplinePoints * 2 + 4 - 1].position = endVertex;
  mVertexData.vertices[numSplinePoints * 2 + 4 - 1].color = glm::vec3(value);

  return mVertexData;
}

