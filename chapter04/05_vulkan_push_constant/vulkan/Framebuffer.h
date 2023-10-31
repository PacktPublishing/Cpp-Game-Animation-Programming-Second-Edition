/* Vulkan framebuffer class */
#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class Framebuffer {
  public:
    static bool init(VkRenderData &renderData);
    static void cleanup(VkRenderData &renderData);
};
