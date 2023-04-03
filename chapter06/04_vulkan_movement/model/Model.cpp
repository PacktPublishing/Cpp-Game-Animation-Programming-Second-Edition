#include "Model.h"
#include "Logger.h"

void Model::init() {
  mVertexData.vertices.resize(36);

  /* front */
  mVertexData.vertices[0].position = glm::vec3(-0.5f, -0.5f,  0.5f);
  mVertexData.vertices[1].position = glm::vec3( 0.5f,  0.5f,  0.5f);
  mVertexData.vertices[2].position = glm::vec3(-0.5f,  0.5f,  0.5f);
  mVertexData.vertices[3].position = glm::vec3(-0.5f, -0.5f,  0.5f);
  mVertexData.vertices[4].position = glm::vec3( 0.5f, -0.5f,  0.5f);
  mVertexData.vertices[5].position = glm::vec3( 0.5f,  0.5f,  0.5f);

  mVertexData.vertices[0].color = glm::vec3(1.0f, 0.5f, 0.5f);
  mVertexData.vertices[1].color = glm::vec3(1.0f, 0.5f, 0.5f);
  mVertexData.vertices[2].color = glm::vec3(1.0f, 0.5f, 0.5f);
  mVertexData.vertices[3].color = glm::vec3(1.0f, 0.5f, 0.5f);
  mVertexData.vertices[4].color = glm::vec3(1.0f, 0.5f, 0.5f);
  mVertexData.vertices[5].color = glm::vec3(1.0f, 0.5f, 0.5f);

  mVertexData.vertices[0].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[1].uv = glm::vec2(1.0, 1.0);
  mVertexData.vertices[2].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[3].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[4].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[5].uv = glm::vec2(1.0, 1.0);

  /* back */
  mVertexData.vertices[6].position = glm::vec3(-0.5f, -0.5f,  -0.5f);
  mVertexData.vertices[7].position = glm::vec3(-0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[8].position = glm::vec3( 0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[9].position = glm::vec3(-0.5f, -0.5f,  -0.5f);
  mVertexData.vertices[10].position = glm::vec3( 0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[11].position = glm::vec3( 0.5f, -0.5f,  -0.5f);

  mVertexData.vertices[6].color = glm::vec3(0.5f, 1.0f, 0.5f);
  mVertexData.vertices[7].color = glm::vec3(0.5f, 1.0f, 0.5f);
  mVertexData.vertices[8].color = glm::vec3(0.5f, 1.0f, 0.5f);
  mVertexData.vertices[9].color = glm::vec3(0.5f, 1.0f, 0.5f);
  mVertexData.vertices[10].color = glm::vec3(0.5f, 1.0f, 0.5f);
  mVertexData.vertices[11].color = glm::vec3(0.5f, 1.0f, 0.5f);

  mVertexData.vertices[6].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[7].uv = glm::vec2(1.0, 1.0);
  mVertexData.vertices[8].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[9].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[10].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[11].uv = glm::vec2(0.0, 0.0);

  /* left */
  mVertexData.vertices[12].position = glm::vec3(-0.5f, -0.5f,  0.5f);
  mVertexData.vertices[13].position = glm::vec3(-0.5f,  0.5f,  0.5f);
  mVertexData.vertices[14].position = glm::vec3(-0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[15].position = glm::vec3(-0.5f, -0.5f,  0.5f);
  mVertexData.vertices[16].position = glm::vec3(-0.5f,  0.5f, -0.5f);
  mVertexData.vertices[17].position = glm::vec3(-0.5f, -0.5f, -0.5f);

  mVertexData.vertices[12].color = glm::vec3(0.5f, 0.5f, 1.0f);
  mVertexData.vertices[13].color = glm::vec3(0.5f, 0.5f, 1.0f);
  mVertexData.vertices[14].color = glm::vec3(0.5f, 0.5f, 1.0f);
  mVertexData.vertices[15].color = glm::vec3(0.5f, 0.5f, 1.0f);
  mVertexData.vertices[16].color = glm::vec3(0.5f, 0.5f, 1.0f);
  mVertexData.vertices[17].color = glm::vec3(0.5f, 0.5f, 1.0f);

  mVertexData.vertices[12].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[13].uv = glm::vec2(1.0, 1.0);
  mVertexData.vertices[14].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[15].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[16].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[17].uv = glm::vec2(0.0, 0.0);

  /* right */
  mVertexData.vertices[18].position = glm::vec3(0.5f, -0.5f,  0.5f);
  mVertexData.vertices[19].position = glm::vec3(0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[20].position = glm::vec3(0.5f,  0.5f,  0.5f);
  mVertexData.vertices[21].position = glm::vec3(0.5f, -0.5f,  0.5f);
  mVertexData.vertices[22].position = glm::vec3(0.5f, -0.5f, -0.5f);
  mVertexData.vertices[23].position = glm::vec3(0.5f,  0.5f, -0.5f);

  mVertexData.vertices[18].color = glm::vec3(0.0f, 0.5f, 0.5f);
  mVertexData.vertices[19].color = glm::vec3(0.0f, 0.5f, 0.5f);
  mVertexData.vertices[20].color = glm::vec3(0.0f, 0.5f, 0.5f);
  mVertexData.vertices[21].color = glm::vec3(0.0f, 0.5f, 0.5f);
  mVertexData.vertices[22].color = glm::vec3(0.0f, 0.5f, 0.5f);
  mVertexData.vertices[23].color = glm::vec3(0.0f, 0.5f, 0.5f);

  mVertexData.vertices[18].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[19].uv = glm::vec2(1.0, 1.0);
  mVertexData.vertices[20].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[21].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[22].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[23].uv = glm::vec2(1.0, 1.0);

  /* top */
  mVertexData.vertices[24].position = glm::vec3( 0.5f,  0.5f,  0.5f);
  mVertexData.vertices[25].position = glm::vec3(-0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[26].position = glm::vec3(-0.5f,  0.5f,  0.5f);
  mVertexData.vertices[27].position = glm::vec3( 0.5f,  0.5f,  0.5f);
  mVertexData.vertices[28].position = glm::vec3( 0.5f,  0.5f,  -0.5f);
  mVertexData.vertices[29].position = glm::vec3(-0.5f,  0.5f,  -0.5f);

  mVertexData.vertices[24].color = glm::vec3(0.5f, 0.0f, 0.5f);
  mVertexData.vertices[25].color = glm::vec3(0.5f, 0.0f, 0.5f);
  mVertexData.vertices[26].color = glm::vec3(0.5f, 0.0f, 0.5f);
  mVertexData.vertices[27].color = glm::vec3(0.5f, 0.0f, 0.5f);
  mVertexData.vertices[28].color = glm::vec3(0.5f, 0.0f, 0.5f);
  mVertexData.vertices[29].color = glm::vec3(0.5f, 0.0f, 0.5f);

  mVertexData.vertices[24].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[25].uv = glm::vec2(1.0, 1.0);
  mVertexData.vertices[26].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[27].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[28].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[29].uv = glm::vec2(1.0, 1.0);

  /* bottom */
  mVertexData.vertices[30].position = glm::vec3( 0.5f,  -0.5f,  0.5f);
  mVertexData.vertices[31].position = glm::vec3(-0.5f,  -0.5f,  0.5f);
  mVertexData.vertices[32].position = glm::vec3(-0.5f,  -0.5f,  -0.5f);
  mVertexData.vertices[33].position = glm::vec3( 0.5f,  -0.5f,  0.5f);
  mVertexData.vertices[34].position = glm::vec3(-0.5f,  -0.5f,  -0.5f);
  mVertexData.vertices[35].position = glm::vec3( 0.5f,  -0.5f,  -0.5f);

  mVertexData.vertices[30].color = glm::vec3(0.5f, 0.5f, 0.0f);
  mVertexData.vertices[31].color = glm::vec3(0.5f, 0.5f, 0.0f);
  mVertexData.vertices[32].color = glm::vec3(0.5f, 0.5f, 0.0f);
  mVertexData.vertices[33].color = glm::vec3(0.5f, 0.5f, 0.0f);
  mVertexData.vertices[34].color = glm::vec3(0.5f, 0.5f, 0.0f);
  mVertexData.vertices[35].color = glm::vec3(0.5f, 0.5f, 0.0f);

  mVertexData.vertices[30].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[31].uv = glm::vec2(0.0, 0.0);
  mVertexData.vertices[32].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[33].uv = glm::vec2(0.0, 1.0);
  mVertexData.vertices[34].uv = glm::vec2(1.0, 0.0);
  mVertexData.vertices[35].uv = glm::vec2(1.0, 1.0);

  Logger::log(1, "%s: loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}

VkMesh Model::getVertexData() {
  return mVertexData;
}
