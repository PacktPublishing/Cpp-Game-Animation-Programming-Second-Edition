#include "TexelBuffer.h"
#include "Logger.h"

bool TexelBuffer::init(VkRenderData& renderData, VkTexelBufferData &TBOData,
    size_t bufferSize) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocInfo,
    &TBOData.rdTboBuffer, &TBOData.rdTboBufferAlloc, nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate texel buffer via VMA\n", __FUNCTION__);
    return false;
  }

  VkBufferViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  viewInfo.pNext = NULL;
  viewInfo.buffer = TBOData.rdTboBuffer;
  viewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  viewInfo.offset = 0;
  viewInfo.range = bufferSize;

  if (vkCreateBufferView(renderData.rdVkbDevice.device, &viewInfo, NULL,
       &TBOData.rdTboBufferView) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create TBO buffer view\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetLayoutBinding tboBind{};
  tboBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
  tboBind.binding = 0;
  tboBind.descriptorCount = 1;
  tboBind.pImmutableSamplers = nullptr;
  tboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo tboCreateInfo{};
  tboCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  tboCreateInfo.bindingCount = 1;
  tboCreateInfo.pBindings = &tboBind;

  if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &tboCreateInfo, nullptr,
    &TBOData.rdTBODescriptorLayout) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create TBO descriptor set layout\n", __FUNCTION__);
    return false;
  }

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptorPool{};
  descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPool.poolSizeCount = 1;
  descriptorPool.pPoolSizes = &poolSize;
  descriptorPool.maxSets = 1;

  if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &descriptorPool, nullptr,
      &TBOData.rdTBODescriptorPool) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create TBO descriptor pool\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
  descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorAllocateInfo.descriptorPool = TBOData.rdTBODescriptorPool;
  descriptorAllocateInfo.descriptorSetCount = 1;
  descriptorAllocateInfo.pSetLayouts = &TBOData.rdTBODescriptorLayout;

  if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo,
      &TBOData.rdTBODescriptorSet) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate TBO descriptor set\n", __FUNCTION__);
    return false;
  }

  VkDescriptorBufferInfo tboInfo{};
  tboInfo.buffer = TBOData.rdTboBuffer;
  tboInfo.offset = 0;
  tboInfo.range = bufferSize;

  VkWriteDescriptorSet writeDescriptorSet{};
  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
  writeDescriptorSet.dstSet = TBOData.rdTBODescriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.pBufferInfo = &tboInfo;
  writeDescriptorSet.pTexelBufferView = &TBOData.rdTboBufferView;

  vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

  Logger::log(1, "%s: created texel buffer of size %i\n", __FUNCTION__, tboInfo.range);
	return true;
}

void TexelBuffer::cleanup(VkRenderData& renderData, VkTexelBufferData &TBOData) {
  vkDestroyDescriptorPool(renderData.rdVkbDevice.device, TBOData.rdTBODescriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, TBOData.rdTBODescriptorLayout,
    nullptr);
  vkDestroyBufferView(renderData.rdVkbDevice.device, TBOData.rdTboBufferView,
    nullptr);
  vmaDestroyBuffer(renderData.rdAllocator, TBOData.rdTboBuffer, TBOData.rdTboBufferAlloc);
}
