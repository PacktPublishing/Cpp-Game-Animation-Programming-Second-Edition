/* Vulkan command pool */
#pragma once

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include "VkRenderData.h"

class CommandPool {
  public:
    static bool init(VkRenderData &renderData);
    static void cleanup(VkRenderData &renderData);
};
