#include "PipelineLayout.h"
#include "Logger.h"

#include <VkBootstrap.h>

bool PipelineLayout::init(VkRenderData &renderData, VkTextureData &textureData, VkPipelineLayout &pipelineLayout) {

  VkPushConstantRange pushConstants{};
  pushConstants.offset = 0;
  pushConstants.size = sizeof(VkPushConstants);
  pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayout layouts [] = { textureData.texTextureDescriptorLayout,
    renderData.rdPerspViewMatrixUBO.rdUBODescriptorLayout,
    renderData.rdJointMatrixSSBO.rdSSBODescriptorLayout,
    renderData.rdJointDualQuatSSBO.rdSSBODescriptorLayout };

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 4;
  pipelineLayoutInfo.pSetLayouts = layouts;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstants;

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
