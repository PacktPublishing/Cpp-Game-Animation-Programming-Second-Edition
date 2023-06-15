/* Renderpass as separate object */
#pragma once

/* Vulkan also before GLFW */
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "VkRenderData.h"

class Renderpass {
  public:
    static bool init(VkRenderData &renderData);
    static void cleanup(VkRenderData &renderData);
};
