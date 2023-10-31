/* Vulkan sync objects */
#pragma once

#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class SyncObjects {
  public:
    static bool init(VkRenderData &renderData);
    static void cleanup(VkRenderData &renderData);
};
