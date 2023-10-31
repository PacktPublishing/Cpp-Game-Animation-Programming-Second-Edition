#include "ShaderStorageBuffer.h"
#include "Logger.h"

bool ShaderStorageBuffer::init(VkRenderData& renderData, VkShaderStorageBufferData &SSBOData,
    size_t bufferSize) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocInfo,
    &SSBOData.rdSsboBuffer, &SSBOData.rdSsboBufferAlloc, nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate shader storage buffer via VMA\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetLayoutBinding ssboBind{};
  ssboBind.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  ssboBind.binding = 0;
  ssboBind.descriptorCount = 1;
  ssboBind.pImmutableSamplers = nullptr;
  ssboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo ssboCreateInfo{};
  ssboCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  ssboCreateInfo.bindingCount = 1;
  ssboCreateInfo.pBindings = &ssboBind;

  if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &ssboCreateInfo, nullptr,
    &SSBOData.rdSSBODescriptorLayout) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create SSBO descriptor set layout\n", __FUNCTION__);
    return false;
  }

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptorPool{};
  descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPool.poolSizeCount = 1;
  descriptorPool.pPoolSizes = &poolSize;
  descriptorPool.maxSets = 1;

  if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &descriptorPool, nullptr,
      &SSBOData.rdSSBODescriptorPool) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create SSBO descriptor pool\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
  descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorAllocateInfo.descriptorPool = SSBOData.rdSSBODescriptorPool;
  descriptorAllocateInfo.descriptorSetCount = 1;
  descriptorAllocateInfo.pSetLayouts = &SSBOData.rdSSBODescriptorLayout;

  if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo,
      &SSBOData.rdSSBODescriptorSet) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate SSBO descriptor set\n", __FUNCTION__);
    return false;
  }

  VkDescriptorBufferInfo ssboInfo{};
  ssboInfo.buffer = SSBOData.rdSsboBuffer;
  ssboInfo.offset = 0;
  ssboInfo.range = bufferSize;

  VkWriteDescriptorSet writeDescriptorSet{};
  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  writeDescriptorSet.dstSet = SSBOData.rdSSBODescriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.pBufferInfo = &ssboInfo;

  vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

  Logger::log(1, "%s: created shader storage buffer of size %i\n", __FUNCTION__, ssboInfo.range);
	return true;
}

void ShaderStorageBuffer::cleanup(VkRenderData& renderData, VkShaderStorageBufferData &SSBOData) {
  vkDestroyDescriptorPool(renderData.rdVkbDevice.device, SSBOData.rdSSBODescriptorPool,
    nullptr);
  vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, SSBOData.rdSSBODescriptorLayout,
    nullptr);
  vmaDestroyBuffer(renderData.rdAllocator, SSBOData.rdSsboBuffer, SSBOData.rdSsboBufferAlloc);
}
