/* Vulkan uniform buffer object */
#pragma once

#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class UniformBuffer {
  public:
    static bool init(VkRenderData &renderData);
    static void uploadData(VkRenderData &renderData, VkUploadMatrices matrices);
    static void cleanup(VkRenderData &renderData);
};
