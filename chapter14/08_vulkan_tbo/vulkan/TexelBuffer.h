/* Vulkan texel buffer object */
#pragma once

#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class TexelBuffer {
  public:
    static bool init(VkRenderData &renderData, VkTexelBufferData &TBOData,
      size_t bufferSize);
    static void cleanup(VkRenderData &renderData, VkTexelBufferData &BOData);
};
