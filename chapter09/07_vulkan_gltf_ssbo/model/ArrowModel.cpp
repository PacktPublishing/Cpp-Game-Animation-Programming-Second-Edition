#include "ArrowModel.h"
#include "Logger.h"

VkMesh ArrowModel::getVertexData() {
  if (mVertexData.vertices.size() == 0) {
    init();
  }
  return mVertexData;
}

void ArrowModel::init() {
  mVertexData.vertices.resize(6);

  /* simple arrow, pointing to X axis */
  mVertexData.vertices[0].position = glm::vec3(0.0f, 0.0f,  0.0f);
  mVertexData.vertices[1].position = glm::vec3(1.0f, 0.0f,  0.0f);
  mVertexData.vertices[2].position = glm::vec3(1.0f, 0.0f,  0.0f);
  mVertexData.vertices[3].position = glm::vec3(0.8f, 0.0f,  0.075f);
  mVertexData.vertices[4].position = glm::vec3(1.0f, 0.0f,  0.0f);
  mVertexData.vertices[5].position = glm::vec3(0.8f, 0.0f, -0.075f);

  mVertexData.vertices[0].color = glm::vec3(0.8f, 0.0f, 0.0f);
  mVertexData.vertices[1].color = glm::vec3(0.8f, 0.0f, 0.0f);
  mVertexData.vertices[2].color = glm::vec3(0.8f, 0.0f, 0.0f);
  mVertexData.vertices[3].color = glm::vec3(0.8f, 0.0f, 0.0f);
  mVertexData.vertices[4].color = glm::vec3(0.8f, 0.0f, 0.0f);
  mVertexData.vertices[5].color = glm::vec3(0.8f, 0.0f, 0.0f);

  Logger::log(1, "%s: ArrowModel - loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}
