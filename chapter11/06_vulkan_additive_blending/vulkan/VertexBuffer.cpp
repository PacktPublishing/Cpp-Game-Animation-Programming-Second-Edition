#include <cstring>

#include "VertexBuffer.h"
#include "CommandBuffer.h"
#include "Logger.h"

bool VertexBuffer::init(VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
    unsigned int bufferSize) {
  /* vertex buffer */
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = bufferSize;
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo bufferAllocInfo{};
  bufferAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &bufferAllocInfo,
      &vertexBufferData.rdVertexBuffer, &vertexBufferData.rdVertexBufferAlloc,
      nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate vertex buffer via VMA\n", __FUNCTION__);
    return false;
  }

  /* staging buffer for copy */
  VkBufferCreateInfo stagingBufferInfo{};
  stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  stagingBufferInfo.size = bufferSize;;
  stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  VmaAllocationCreateInfo stagingAllocInfo{};
  stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

  if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo,
      &vertexBufferData.rdStagingBuffer, &vertexBufferData.rdStagingBufferAlloc,
      nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate vertex staging buffer via VMA\n",
      __FUNCTION__);
    return false;
  }
  vertexBufferData.rdVertexBufferSize = bufferSize;
  return true;
}

bool VertexBuffer::uploadData(VkRenderData& renderData, VkVertexBufferData &vertexBufferData,
    VkMesh vertexData) {
  unsigned int vertexDataSize = vertexData.vertices.size() * sizeof(VkVertex);

  /* buffer too small, resize */
  if (vertexBufferData.rdVertexBufferSize < vertexDataSize) {
    cleanup(renderData, vertexBufferData);

    if (!init(renderData, vertexBufferData, vertexDataSize)) {
      Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n",
        __FUNCTION__, vertexDataSize);
      return false;
    }
    Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
    vertexBufferData.rdVertexBufferSize = vertexDataSize;
  }

  /* copy data to staging buffer*/
  void* data;
  vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
  std::memcpy(data, vertexData.vertices.data(), vertexDataSize);
  vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

  VkBufferMemoryBarrier vertexBufferBarrier{};
  vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
  vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
  vertexBufferBarrier.offset = 0;
  vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

  VkBufferCopy stagingBufferCopy{};
  stagingBufferCopy.srcOffset = 0;
  stagingBufferCopy.dstOffset = 0;
  stagingBufferCopy.size = vertexDataSize;

  vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer,
   vertexBufferData.rdVertexBuffer, 1, &stagingBufferCopy);
  vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

  return true;
}

bool VertexBuffer::uploadData(VkRenderData& renderData, VkVertexBufferData &vertexBufferData,
    std::vector<glm::vec3> vertexData) {
  unsigned int vertexDataSize = vertexData.size() * sizeof(glm::vec3);

  /* buffer too small, resize */
  if (vertexBufferData.rdVertexBufferSize < vertexDataSize) {
    cleanup(renderData, vertexBufferData);

    if (!init(renderData, vertexBufferData, vertexDataSize)) {
      Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n",
        __FUNCTION__, vertexDataSize);
      return false;
    }
    Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
    vertexBufferData.rdVertexBufferSize = vertexDataSize;
  }

  /* copy data to staging buffer*/
  void* data;
  vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
  std::memcpy(data, vertexData.data(), vertexDataSize);
  vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

  VkBufferMemoryBarrier vertexBufferBarrier{};
  vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
  vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
  vertexBufferBarrier.offset = 0;
  vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

  VkBufferCopy stagingBufferCopy{};
  stagingBufferCopy.srcOffset = 0;
  stagingBufferCopy.dstOffset = 0;
  stagingBufferCopy.size = vertexDataSize;

  vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer,
   vertexBufferData.rdVertexBuffer, 1, &stagingBufferCopy);
  vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

  return true;
}

bool VertexBuffer::uploadData(VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
    const tinygltf::Buffer &buffer, const tinygltf::BufferView &bufferView) {
  /* buffer too small, resize */
  if (vertexBufferData.rdVertexBufferSize < bufferView.byteLength) {
    cleanup(renderData, vertexBufferData);

    if (!init(renderData, vertexBufferData, bufferView.byteLength)) {
      Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n",
        __FUNCTION__, bufferView.byteLength);
      return false;
    }
    Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, bufferView.byteLength);
    vertexBufferData.rdVertexBufferSize = bufferView.byteLength;
  }

  /* copy data to staging buffer*/
  void* data;
  vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
  std::memcpy(data, &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);
  vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

  VkBufferMemoryBarrier vertexBufferBarrier{};
  vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
  vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
  vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
  vertexBufferBarrier.offset = 0;
  vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

  VkBufferCopy stagingBufferCopy{};
  stagingBufferCopy.srcOffset = 0;
  stagingBufferCopy.dstOffset = 0;
  stagingBufferCopy.size = vertexBufferData.rdVertexBufferSize;

  vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer,
   vertexBufferData.rdVertexBuffer, 1, &stagingBufferCopy);
  vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

  return true;
}

void VertexBuffer::cleanup(VkRenderData &renderData, VkVertexBufferData &vertexBufferData) {
  vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdStagingBuffer, vertexBufferData.rdStagingBufferAlloc);
  vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdVertexBuffer, vertexBufferData.rdVertexBufferAlloc);
}
