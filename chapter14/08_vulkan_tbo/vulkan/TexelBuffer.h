/* Vulkan texel buffer object */
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class TexelBuffer {
  public:
    static bool init(VkRenderData &renderData, VkTexelBufferData &TBOData,
      size_t bufferSize);
    static void uploadData(VkRenderData &renderData, VkTexelBufferData &TBOData,
      std::vector<glm::mat4> matricesToUpload);
    static void cleanup(VkRenderData &renderData, VkTexelBufferData &BOData);
};
