#include "PipelineLayout.h"
#include "Logger.h"

#include <VkBootstrap.h>

bool PipelineLayout::init(VkRenderData &renderData, VkTextureData &textureData, VkPipelineLayout &pipelineLayout) {

  VkDescriptorSetLayout layouts [] = { textureData.texTextureDescriptorLayout,
    renderData.rdPerspViewMatrixUBO.rdUBODescriptorLayout,
    renderData.rdJointMatrixSSBO.rdSSBODescriptorLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 3;
  pipelineLayoutInfo.pSetLayouts = layouts;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr,
      &pipelineLayout) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create pipeline layout\n", __FUNCTION__);
    return false;
  }
  return true;
}

void PipelineLayout::cleanup(VkRenderData &renderData, VkPipelineLayout &pipelineLayout) {
  vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
