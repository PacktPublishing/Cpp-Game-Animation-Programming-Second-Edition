#include "UniformBuffer.h"
#include "Logger.h"

#include <VkBootstrap.h>

bool UniformBuffer::init(VkRenderData& renderData) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof(VkUploadMatrices);
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocInfo, &renderData.rdUboBuffer, &renderData.rdUboBufferAlloc, nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate uniform buffer via VMA\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetLayoutBinding uboBind{};
  uboBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboBind.binding = 0;
  uboBind.descriptorCount = 1;
  uboBind.pImmutableSamplers = nullptr;
  uboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo uboCreateInfo{};
  uboCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  uboCreateInfo.bindingCount = 1;
  uboCreateInfo.pBindings = &uboBind;

  if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &uboCreateInfo, nullptr, &renderData.rdUBODescriptorLayout) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create UBO descriptor set layout\n", __FUNCTION__);
    return false;
  }

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo descriptorPool{};
  descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPool.poolSizeCount = 1;
  descriptorPool.pPoolSizes = &poolSize;
  descriptorPool.maxSets = 1;

  if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &descriptorPool, nullptr, &renderData.rdUBODescriptorPool) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create UBO descriptor pool\n", __FUNCTION__);
    return false;
  }

  VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
  descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descriptorAllocateInfo.descriptorPool = renderData.rdUBODescriptorPool;
  descriptorAllocateInfo.descriptorSetCount = 1;
  descriptorAllocateInfo.pSetLayouts = &renderData.rdUBODescriptorLayout;

  if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo, &renderData.rdUBODescriptorSet) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate UBO descriptor set\n", __FUNCTION__);
    return false;
  }

  VkDescriptorBufferInfo uboInfo{};
  uboInfo.buffer = renderData.rdUboBuffer;
  uboInfo.offset = 0;
  uboInfo.range = sizeof(VkUploadMatrices);

  VkWriteDescriptorSet writeDescriptorSet{};
  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeDescriptorSet.dstSet = renderData.rdUBODescriptorSet;
  writeDescriptorSet.dstBinding = 0;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.pBufferInfo = &uboInfo;

  vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

	return true;
}

void UniformBuffer::uploadData(VkRenderData &renderData, VkUploadMatrices matrices) {
  void* data;
  vmaMapMemory(renderData.rdAllocator, renderData.rdUboBufferAlloc, &data);
  std::memcpy(data, &matrices, sizeof(VkUploadMatrices));
  vmaUnmapMemory(renderData.rdAllocator, renderData.rdUboBufferAlloc);
}

void UniformBuffer::cleanup(VkRenderData& renderData) {
  vkDestroyDescriptorPool(renderData.rdVkbDevice.device, renderData.rdUBODescriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdUBODescriptorLayout, nullptr);
  vmaDestroyBuffer(renderData.rdAllocator, renderData.rdUboBuffer, renderData.rdUboBufferAlloc);
}
