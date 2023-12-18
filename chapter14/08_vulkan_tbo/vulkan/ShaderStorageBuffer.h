/* Vulkan shader storage buffer object */
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class ShaderStorageBuffer {
  public:
    static bool init(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData,
      size_t bufferSize);
    static void uploadData(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData,
      std::vector<glm::mat4> matricesToUpload);
    static void uploadData(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData,
      std::vector<glm::mat2x4> matricesToUpload);
    static void cleanup(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData);
};
