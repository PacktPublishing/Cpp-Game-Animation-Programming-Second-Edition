#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GltfModel.h"
#include "Logger.h"

bool GltfModel::loadModel(VkRenderData &renderData, VkGltfRenderData &gltfRenderData,
    std::string modelFilename, std::string textureFilename) {
  if (!Texture::loadTexture(renderData, gltfRenderData.rdGltfModelTexture, textureFilename)) {
    Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: glTF model texture '%s' successfully loaded\n", __FUNCTION__,
    modelFilename.c_str());

  mModel = std::make_shared<tinygltf::Model>();

  tinygltf::TinyGLTF gltfLoader;
  std::string loaderErrors;
  std::string loaderWarnings;
  bool result = false;

  result = gltfLoader.LoadASCIIFromFile(mModel.get(), &loaderErrors, &loaderWarnings,
    modelFilename);

  if (!loaderWarnings.empty()) {
    Logger::log(1, "%s: warnings while loading glTF model:\n%s\n", __FUNCTION__,
      loaderWarnings.c_str());
  }

  if (!loaderErrors.empty()) {
    Logger::log(1, "%s: errors while loading glTF model:\n%s\n", __FUNCTION__,
      loaderErrors.c_str());
  }

  if (!result) {
    Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__,
      modelFilename.c_str());
    return false;
  }

  /* extract position, normal, texture coords, and indices */
  createVertexBuffers(renderData, gltfRenderData);
  createIndexBuffer(renderData, gltfRenderData);

  renderData.rdGltfTriangleCount = getTriangleCount();;

  return true;
}

void GltfModel::createVertexBuffers(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  gltfRenderData.rdGltfVertexBufferData.resize(primitives.attributes.size());

  for (const auto& attrib : primitives.attributes) {
    const std::string attribType = attrib.first;
    const int accessorNum = attrib.second;

    const tinygltf::Accessor &accessor = mModel->accessors.at(accessorNum);
    const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

    if ((attribType.compare("POSITION") != 0) && (attribType.compare("NORMAL") != 0)
        && (attribType.compare("TEXCOORD_0") != 0)) {
      Logger::log(1, "%s: skipping attribute type %s\n", __FUNCTION__, attribType.c_str());
      continue;
    }

    /* buffers for position, normal and tex coordinates */
    VertexBuffer::init(renderData,
      gltfRenderData.rdGltfVertexBufferData.at(attributes.at(attribType)), bufferView.byteLength);
  }
}

void GltfModel::createIndexBuffer(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  /* buffer for vertex indices */
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView &indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer &indexBuffer = mModel->buffers.at(indexBufferView.buffer);

  IndexBuffer::init(renderData, gltfRenderData.rdGltfIndexBufferData, indexBufferView.byteLength);
}

void GltfModel::uploadVertexBuffers(VkRenderData& renderData, VkGltfRenderData& gltfRenderData) {
  for (int i = 0; i < 3; ++i) {
    const tinygltf::Accessor& accessor = mModel->accessors.at(i);
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    VertexBuffer::uploadData(renderData,
      gltfRenderData.rdGltfVertexBufferData.at(i), buffer, bufferView);
  }
}

void GltfModel::uploadIndexBuffer(VkRenderData& renderData, VkGltfRenderData& gltfRenderData) {
  /* buffer for vertex indices */
  const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

  IndexBuffer::uploadData(renderData, gltfRenderData.rdGltfIndexBufferData,
    indexBuffer, indexBufferView);
}

int GltfModel::getTriangleCount() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);

  unsigned int triangles = 0;
  switch (primitives.mode) {
    case TINYGLTF_MODE_TRIANGLES:
      triangles =  indexAccessor.count;
      break;
    default:
      Logger::log(1, "%s error: unknown draw mode %i\n", __FUNCTION__, primitives.mode);
      break;
  }
  return triangles;
}

void GltfModel::draw(VkRenderData &renderData, VkGltfRenderData& gltfRenderData) {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);

  /* texture */
  vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    renderData.rdGltfPipelineLayout, 0, 1,
    &gltfRenderData.rdGltfModelTexture.texTextureDescriptorSet, 0, nullptr);

  /* vertex buffer */
  VkDeviceSize offset = 0;
  for (int i = 0; i < 3; ++i) {
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, i, 1,
      &gltfRenderData.rdGltfVertexBufferData.at(i).rdVertexBuffer, &offset);
  }

  /* index buffer */
  vkCmdBindIndexBuffer(renderData.rdCommandBuffer,
    gltfRenderData.rdGltfIndexBufferData.rdIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

  vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    renderData.rdGltfPipeline);
  vkCmdDrawIndexed(renderData.rdCommandBuffer,
    static_cast<uint32_t>(renderData.rdGltfTriangleCount), 1, 0, 0, 0);
}

void GltfModel::cleanup(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  for (int i = 0; i < 3; ++i) {
    VertexBuffer::cleanup(renderData, gltfRenderData.rdGltfVertexBufferData.at(i));
  }

  IndexBuffer::cleanup(renderData, gltfRenderData.rdGltfIndexBufferData);

  Texture::cleanup(renderData, gltfRenderData.rdGltfModelTexture);
  mModel.reset();
}
