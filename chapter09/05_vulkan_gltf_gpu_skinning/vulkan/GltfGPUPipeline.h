/* Vulkan graphics pipeline with shaders, GPU skinning */
#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

class GltfGPUPipeline {
  public:
    static bool init(VkRenderData &renderData, VkPipelineLayout& pipelineLayout,
      VkPipeline& pipeline, VkPrimitiveTopology topology,
      std::string vertexShaderFilename, std::string fragmentShaderFilename);
    static void cleanup(VkRenderData &renderData, VkPipeline &pipeline);
};
