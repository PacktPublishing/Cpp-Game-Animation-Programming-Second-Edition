/* Vulkan uniform buffer object */
#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <tiny_gltf.h>

#include "VkRenderData.h"

class VertexBuffer {
  public:
    static bool init(VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
      unsigned int bufferSize);
    static bool uploadData(VkRenderData& renderData, VkVertexBufferData &vertexBufferData,
      VkMesh vertexData);
    static bool uploadData(VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
      std::vector<glm::vec3> vetrexData);
    static bool uploadData(VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
      const tinygltf::Buffer &buffer, const tinygltf::BufferView &bufferView);
    static void cleanup(VkRenderData &renderData, VkVertexBufferData &vertexBufferData);
};
