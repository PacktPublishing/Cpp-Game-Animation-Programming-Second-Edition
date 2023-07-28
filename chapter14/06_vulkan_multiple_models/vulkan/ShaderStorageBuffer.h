/* Vulkan shader storage buffer object */
#pragma once

#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class ShaderStorageBuffer {
  public:
    static bool init(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData,
      size_t bufferSize);
    static void cleanup(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData);
};
