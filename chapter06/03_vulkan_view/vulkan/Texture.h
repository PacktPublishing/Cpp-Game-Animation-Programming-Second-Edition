/* Vulkan texture */
#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"
#include "CommandBuffer.h"

class Texture {
  public:
    static bool loadTexture(VkRenderData &renderData, std::string textureFilename);
    static void cleanup(VkRenderData &renderData);
};
