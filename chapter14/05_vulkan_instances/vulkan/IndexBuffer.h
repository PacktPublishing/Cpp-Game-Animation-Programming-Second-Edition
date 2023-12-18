/* Vulkan uniform index buffer object */
#pragma once

#include <vulkan/vulkan.h>
#include <tiny_gltf.h>

#include "VkRenderData.h"

class IndexBuffer {
  public:
    static bool init(VkRenderData &renderData, VkIndexBufferData &indexBufferData,
      size_t bufferSize);
    static bool uploadData(VkRenderData &renderData, VkIndexBufferData &indexBufferData,
      const tinygltf::Buffer &buffer, const tinygltf::BufferView &bufferView);
    static void cleanup(VkRenderData &renderData, VkIndexBufferData &IndexBufferData);
};
