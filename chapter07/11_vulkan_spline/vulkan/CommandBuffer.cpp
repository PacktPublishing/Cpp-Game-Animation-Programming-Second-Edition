#include "CommandBuffer.h"
#include "Logger.h"

bool CommandBuffer::init(VkRenderData &renderData, VkCommandBuffer &commandBuffer) {
  VkCommandBufferAllocateInfo bufferAllocInfo{};
  bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  bufferAllocInfo.commandPool = renderData.rdCommandPool;
  bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  bufferAllocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(renderData.rdVkbDevice.device, &bufferAllocInfo, &commandBuffer) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate command buffers\n", __FUNCTION__);
    return false;
  }

  return true;
}

void CommandBuffer::cleanup(VkRenderData &renderData, VkCommandBuffer &commandBuffer) {
  vkFreeCommandBuffers(renderData.rdVkbDevice.device, renderData.rdCommandPool, 1, &commandBuffer);
}
