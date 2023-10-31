#include "CoordArrowsModel.h"
#include "Logger.h"

OGLMesh CoordArrowsModel::getVertexData() {
  if (mVertexData.vertices.size() == 0) {
    init();
  }
  return mVertexData;
}

void CoordArrowsModel::init() {
  mVertexData.vertices.resize(18);

  /*  X axis - red */
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

  /*  Y axis - green */
  mVertexData.vertices[6].position = glm::vec3(0.0f, 0.0f,  0.0f);
  mVertexData.vertices[7].position = glm::vec3(0.0f, 1.0f,  0.0f);
  mVertexData.vertices[8].position = glm::vec3(0.0f, 1.0f,  0.0f);
  mVertexData.vertices[9].position = glm::vec3(0.0f, 0.8f,  0.075f);
  mVertexData.vertices[10].position = glm::vec3(0.0f, 1.0f,  0.0f);
  mVertexData.vertices[11].position = glm::vec3(0.0f, 0.8f, -0.075f);

  mVertexData.vertices[6].color = glm::vec3(0.0f, 0.8f, 0.0f);
  mVertexData.vertices[7].color = glm::vec3(0.0f, 0.8f, 0.0f);
  mVertexData.vertices[8].color = glm::vec3(0.0f, 0.8f, 0.0f);
  mVertexData.vertices[9].color = glm::vec3(0.0f, 0.8f, 0.0f);
  mVertexData.vertices[10].color = glm::vec3(0.0f, 0.8f, 0.0f);
  mVertexData.vertices[11].color = glm::vec3(0.0f, 0.8f, 0.0f);

  /*  Z axis - blue */
  mVertexData.vertices[12].position = glm::vec3( 0.0f,   0.0f, 0.0f);
  mVertexData.vertices[13].position = glm::vec3( 0.0f,   0.0f, 1.0f);
  mVertexData.vertices[14].position = glm::vec3( 0.0f,   0.0f, 1.0f);
  mVertexData.vertices[15].position = glm::vec3( 0.075f, 0.0f, 0.8f);
  mVertexData.vertices[16].position = glm::vec3( 0.0f,   0.0f, 1.0f);
  mVertexData.vertices[17].position = glm::vec3(-0.075f, 0.0f, 0.8f);

  mVertexData.vertices[12].color = glm::vec3(0.0f, 0.0f, 0.8f);
  mVertexData.vertices[13].color = glm::vec3(0.0f, 0.0f, 0.8f);
  mVertexData.vertices[14].color = glm::vec3(0.0f, 0.0f, 0.8f);
  mVertexData.vertices[15].color = glm::vec3(0.0f, 0.0f, 0.8f);
  mVertexData.vertices[16].color = glm::vec3(0.0f, 0.0f, 0.8f);
  mVertexData.vertices[17].color = glm::vec3(0.0f, 0.0f, 0.8f);

  Logger::log(1, "%s: CoordArrowsModel - loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}

