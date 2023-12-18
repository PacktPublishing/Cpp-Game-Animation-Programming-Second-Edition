/* Vulkan texture */
#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class Texture {
  public:
    static bool loadTexture(VkRenderData &renderData, VkTextureData &textureData, std::string textureFilename);
    static void cleanup(VkRenderData &renderData, VkTextureData &textureData);
};