/* Vulkan uniform buffer object */
#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class UniformBuffer {
  public:
    static bool init(VkRenderData &renderData, VkUniformBufferData &UBOData,
      size_t bufferSize);
    static void uploadData(VkRenderData &renderData, VkUniformBufferData &UBOData,
      std::vector<glm::mat4> matricesToUpload);
    static void cleanup(VkRenderData &renderData, VkUniformBufferData &UBOData);
};
