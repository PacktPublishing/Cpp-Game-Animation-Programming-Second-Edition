#include <glm/gtc/matrix_transform.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VkRenderer.h"
#include "Logger.h"

VkRenderer::VkRenderer(GLFWwindow *window) {
  mWindow = window;
}

bool VkRenderer::init(unsigned int width, unsigned int height) {
  if (!mWindow) {
    Logger::log(1, "%s error: invalid GLFWwindow handle\n", __FUNCTION__);
    return false;
  }

  if (!deviceInit(mRenderData)) {
    return false;
  }

  if (!initVma()) {
    return false;
  }

  if (!getQueue()) {
    return false;
  }

  if (!createSwapchain(mRenderData)) {
    return false;
  }

  /* must be done AFTER swapchain as we need data from it */
  if (!createDepthBuffer(mRenderData)) {
    return false;
  }

  if (!createCommandPool(mRenderData)) {
    return false;
  }

  if (!createCommandBuffer(mRenderData)) {
    return false;
  }

  /* we need the command pool */
  if (!loadTexture(mRenderData)) {
    return false;
  }


  if (!createRenderPass(mRenderData)) {
    return false;
  }

  if (!createPipelineLayout(mRenderData)) {
      return false;
  }

  if (!createBasicPipeline(mRenderData)) {
    return false;
  }

  if (!createChangedPipeline(mRenderData)) {
    return false;
  }

  if (!createFramebuffer(mRenderData)) {
    return false;
  }

  if (!createSyncObjects(mRenderData)) {
    return false;
  }

  Logger::log(1, "%s: Vulkan renderer initialized to %ix%i\n", __FUNCTION__, width, height);
  return true;
}

bool VkRenderer::deviceInit(VkRenderData &renderData) {
  /* instance and window */
  vkb::InstanceBuilder instBuild;
  auto instRet = instBuild.use_default_debug_messenger().request_validation_layers().build();
  // auto instRet = instBuild.build();
  if (!instRet) {
    Logger::log(1, "%s error: could not build vkb instance\n", __FUNCTION__);
    return false;
  }
  renderData.rdVkbInstance = instRet.value();

  VkResult result = VK_ERROR_UNKNOWN;
  result = glfwCreateWindowSurface(renderData.rdVkbInstance, mWindow, nullptr, &mSurface);
  if (result != VK_SUCCESS) {
    Logger::log(1, "%s error: Could not create Vulkan surface\n", __FUNCTION__);
    return false;
  }

  /* just get the first available device */
  vkb::PhysicalDeviceSelector physicalDevSel{mRenderData.rdVkbInstance};
  auto physicalDevSelRet = physicalDevSel.set_surface(mSurface).select();
  if (!physicalDevSelRet) {
    Logger::log(1, "%s error: could not get physical devices\n", __FUNCTION__);
    return false;
  }
  mPhysDevice = physicalDevSelRet.value();

  Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__, mPhysDevice.name.c_str());

  vkb::DeviceBuilder devBuilder{mPhysDevice};
  auto devBuilderRet = devBuilder.build();
  if (!devBuilderRet) {
    Logger::log(1, "%s error: could not get devices\n", __FUNCTION__);
    return false;
  }
  renderData.rdVkbDevice = devBuilderRet.value();

  return true;
}

bool VkRenderer::getQueue() {
  auto graphQueueRet = mRenderData.rdVkbDevice.get_queue(vkb::QueueType::graphics);
  if (!graphQueueRet.has_value()) {
    Logger::log(1, "%s error: could not get graphics queue\n", __FUNCTION__);
    return false;
  }
  mRenderData.rdGraphicsQueue = graphQueueRet.value();

  auto presentQueueRet = mRenderData.rdVkbDevice.get_queue(vkb::QueueType::present);
  if (!presentQueueRet.has_value()) {
    Logger::log(1, "%s error: could not get present queue\n", __FUNCTION__);
    return false;
  }
  mRenderData.rdPresentQueue = presentQueueRet.value();

  return true;
}

bool VkRenderer::createDepthBuffer(VkRenderData &renderData) {
  VkExtent3D depthImageExtent = {
        renderData.rdVkbSwapchain.extent.width,
        renderData.rdVkbSwapchain.extent.height,
        1
  };

  renderData.rdDepthFormat = VK_FORMAT_D32_SFLOAT;

  VkImageCreateInfo depthImageInfo{};
  depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  depthImageInfo.format = renderData.rdDepthFormat;
  depthImageInfo.extent = depthImageExtent;
  depthImageInfo.mipLevels = 1;
  depthImageInfo.arrayLayers = 1;
  depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VmaAllocationCreateInfo depthAllocInfo{};
  depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vmaCreateImage(renderData.rdAllocator, &depthImageInfo, &depthAllocInfo, &renderData.rdDepthImage, &renderData.rdDepthImageAlloc, nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
    return false;
  }

  VkImageViewCreateInfo depthImageViewinfo{};
  depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depthImageViewinfo.image = renderData.rdDepthImage;
  depthImageViewinfo.format = renderData.rdDepthFormat;
  depthImageViewinfo.subresourceRange.baseMipLevel = 0;
  depthImageViewinfo.subresourceRange.levelCount = 1;
  depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
  depthImageViewinfo.subresourceRange.layerCount = 1;
  depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  if (vkCreateImageView(renderData.rdVkbDevice.device, &depthImageViewinfo, nullptr, &renderData.rdDepthImageView) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createSwapchain(VkRenderData &renderData) {
  vkb::SwapchainBuilder swapChainBuild{renderData.rdVkbDevice};

  /* VK_PRESENT_MODE_FIFO_KHR enables vsync */
  auto swapChainBuildRet = swapChainBuild.set_old_swapchain(renderData.rdVkbSwapchain).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).build();
  if (!swapChainBuildRet) {
    Logger::log(1, "%s error: could not init swapchain\n", __FUNCTION__);
    return false;
  }

  vkb::destroy_swapchain(renderData.rdVkbSwapchain);
  renderData.rdVkbSwapchain = swapChainBuildRet.value();

  return true;
}

bool VkRenderer::recreateSwapchain(VkRenderData &renderData) {
  /* handle minimize */
  int width = 0, height = 0;
  glfwGetFramebufferSize(mWindow, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(mWindow, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(renderData.rdVkbDevice.device);

  /* cleanup */
  CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
  CommandPool::cleanup(renderData);
  Framebuffer::cleanup(renderData);
  vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
  vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);

  renderData.rdVkbSwapchain.destroy_image_views(renderData.rdSwapchainImageViews);

  /* and recreate */
  if (!createSwapchain(renderData)) {
    Logger::log(1, "%s error: could not recreate swapchain\n", __FUNCTION__);
    return false;
  }

  if (!createDepthBuffer(mRenderData)) {
    Logger::log(1, "%s error: could not recreate depth buffer\n", __FUNCTION__);
    return false;
  }

  if (!createFramebuffer(renderData)) {
    Logger::log(1, "%s error: could not recreate framebuffers\n", __FUNCTION__);
    return false;
  }

  if (!createCommandPool(renderData)) {
    Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
    return false;
  }

  if (!createCommandBuffer(renderData)) {
    Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
    return false;
  }

  return true;
}

bool VkRenderer::createRenderPass(VkRenderData &renderData) {
  if (!Renderpass::init(renderData)) {
    Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createPipelineLayout(VkRenderData &renderData) {
  if (!PipelineLayout::init(renderData, renderData.rdPipelineLayout)) {
    Logger::log(1, "%s error: could not init pipeline layout\n", __FUNCTION__);
    return false;
  }
  return true;
}
bool VkRenderer::createBasicPipeline(VkRenderData &renderData) {
  std::string vertexShaderFile = "shader/basic.vert.spv";
  std::string fragmentShaderFile = "shader/basic.frag.spv";
  if (!Pipeline::init(renderData, renderData.rdPipelineLayout, renderData.rdBasicPipeline, vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createChangedPipeline(VkRenderData& renderData) {
  std::string vertexShaderFile = "shader/changed.vert.spv";
  std::string fragmentShaderFile = "shader/changed.frag.spv";
  if (!Pipeline::init(renderData, renderData.rdPipelineLayout, renderData.rdChangedPipeline, vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createFramebuffer(VkRenderData &renderData) {
  if (!Framebuffer::init(renderData)) {
    Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createCommandPool(VkRenderData &renderData) {
  if (!CommandPool::init(renderData)) {
    Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createCommandBuffer(VkRenderData &renderData) {
  if (!CommandBuffer::init(renderData, renderData.rdCommandBuffer)) {
    Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createSyncObjects(VkRenderData& renderData) {
  if (!SyncObjects::init(renderData)) {
    Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::loadTexture(VkRenderData& renderData) {
  std::string textureFileName = "textures/crate.png";
  if (!Texture::loadTexture(renderData, textureFileName)) {
    Logger::log(1, "%s error: could not load texture\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::initVma() {
  VmaAllocatorCreateInfo allocatorInfo{};
  allocatorInfo.physicalDevice = mPhysDevice.physical_device;
  allocatorInfo.device = mRenderData.rdVkbDevice.device;
  allocatorInfo.instance = mRenderData.rdVkbInstance.instance;
  if (vmaCreateAllocator(&allocatorInfo, &mRenderData.rdAllocator) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not init VMA\n", __FUNCTION__);
    return false;
  }

  return true;
}

void VkRenderer::cleanup() {
  vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

  Texture::cleanup(mRenderData);
  vmaDestroyBuffer(mRenderData.rdAllocator, mVertexBuffer, mVertexBufferAlloc);

  SyncObjects::cleanup(mRenderData);
  CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
  CommandPool::cleanup(mRenderData);
  Framebuffer::cleanup(mRenderData);
  Pipeline::cleanup(mRenderData, mRenderData.rdBasicPipeline);
  Pipeline::cleanup(mRenderData, mRenderData.rdChangedPipeline);
  PipelineLayout::cleanup(mRenderData, mRenderData.rdPipelineLayout);
  Renderpass::cleanup(mRenderData);

  vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
  vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);
  vmaDestroyAllocator(mRenderData.rdAllocator);

  mRenderData.rdVkbSwapchain.destroy_image_views(mRenderData.rdSwapchainImageViews);
  vkb::destroy_swapchain(mRenderData.rdVkbSwapchain);

  vkb::destroy_device(mRenderData.rdVkbDevice);
  vkb::destroy_surface(mRenderData.rdVkbInstance.instance, mSurface);
  vkb::destroy_instance(mRenderData.rdVkbInstance);

  Logger::log(1, "%s: Vulkan renderer destroyed\n", __FUNCTION__);
}

void VkRenderer::setSize(unsigned int width, unsigned int height) {
  /* Vulkan detects changes and recreates swapchain */
  Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

bool VkRenderer::uploadData(VkMesh vertexData) {
  /* needs to be split in buffer alloc and upload */

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = vertexData.vertices.size() * sizeof(VkVertex);
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  if (vmaCreateBuffer(mRenderData.rdAllocator, &bufferInfo, &vmaAllocInfo, &mVertexBuffer, &mVertexBufferAlloc, nullptr) != VK_SUCCESS) {
      Logger::log(1, "%s error: could not allocate vertex buffer via VMA\n", __FUNCTION__);
      return false;
  }

  void* data;
  vmaMapMemory(mRenderData.rdAllocator, mVertexBufferAlloc, &data);
  memcpy(data, vertexData.vertices.data(), vertexData.vertices.size() * sizeof(VkVertex));
  vmaUnmapMemory(mRenderData.rdAllocator, mVertexBufferAlloc);

  mTriangleCount = vertexData.vertices.size() / 3;

  return true;
}

void VkRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
    toggleShader();
  }
}

void VkRenderer::toggleShader() {
  mUseChangedShader = !mUseChangedShader;
}

bool VkRenderer::draw() {
  if (vkWaitForFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    Logger::log(1, "%s error: waiting for fence failed", __FUNCTION__);
    return false;
  }

  if (vkResetFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence) != VK_SUCCESS) {
    Logger::log(1, "%s error:  fence reset failed", __FUNCTION__);
    return false;
  }

  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(mRenderData.rdVkbDevice.device,
      mRenderData.rdVkbSwapchain.swapchain,
      UINT64_MAX,
      mRenderData.rdPresentSemaphore,
      VK_NULL_HANDLE,
      &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return recreateSwapchain(mRenderData);
  } else {
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      Logger::log(1, "%s error: failed to acquire swapchain image. Error is '%i'\n", __FUNCTION__, result);
      return false;
    }
  }

  if (vkResetCommandBuffer(mRenderData.rdCommandBuffer, 0) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to reset command buffer\n", __FUNCTION__);
    return false;
  }

  VkCommandBufferBeginInfo cmdBeginInfo{};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if(vkBeginCommandBuffer(mRenderData.rdCommandBuffer, &cmdBeginInfo) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to begin command buffer\n", __FUNCTION__);
    return false;
  }

  VkClearValue colorClearValue;
  colorClearValue.color = { { 0.1f, 0.1f, 0.1f, 1.0f } };

  VkClearValue depthValue;
  depthValue.depthStencil.depth = 1.0f;

  VkClearValue clearValues[] = { colorClearValue, depthValue };

  VkRenderPassBeginInfo rpInfo{};
  rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpInfo.renderPass = mRenderData.rdRenderpass;

  rpInfo.renderArea.offset.x = 0;
  rpInfo.renderArea.offset.y = 0;
  rpInfo.renderArea.extent = mRenderData.rdVkbSwapchain.extent;
  rpInfo.framebuffer = mRenderData.rdFramebuffers[imageIndex];

  rpInfo.clearValueCount = 2;
  rpInfo.pClearValues = clearValues;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(mRenderData.rdVkbSwapchain.extent.width);
  viewport.height = static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = mRenderData.rdVkbSwapchain.extent;

  vkCmdBeginRenderPass(mRenderData.rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

  /* the rendering itself happens here */
  if (!mUseChangedShader) {
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdBasicPipeline);
  } else {
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdChangedPipeline);
  }

  /* required for dynamic viewport */
  vkCmdSetViewport(mRenderData.rdCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(mRenderData.rdCommandBuffer, 0, 1, &scissor);

  /* the triangle drawing itself */
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1, &mVertexBuffer, &offset);

  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout, 0, 1, &mRenderData.rdDescriptorSet, 0, nullptr);

  glm::vec3 cameraPosition = glm::vec3(0.4f, 0.3f, 1.0f);
  glm::vec3 cameraLookAtPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);

  VkPushConstants pushMatrices;
  pushMatrices.pkProjectionMatrix = glm::perspective(glm::radians(90.0f), static_cast<float>(mRenderData.rdVkbSwapchain.extent.width) / static_cast<float>(mRenderData.rdVkbSwapchain.extent.height), 0.1f, 100.f);

  float t = glfwGetTime();
  glm::mat4 model;

  //upload the matrix to the GPU via push constants
  if (!mUseChangedShader) {
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), -t, glm::vec3(0.0f, 0.0f, 1.0f));
    pushMatrices.pkViewMatrix = glm::lookAt(cameraPosition, cameraLookAtPosition, cameraUpVector) * model;
  } else {
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0.0f, 0.0f, 1.0f));
    pushMatrices.pkViewMatrix = glm::lookAt(cameraPosition, cameraLookAtPosition, cameraUpVector) * model;
  }

  vkCmdPushConstants(mRenderData.rdCommandBuffer, mRenderData.rdPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkPushConstants), &pushMatrices);

  vkCmdDraw(mRenderData.rdCommandBuffer, mTriangleCount * 3, 1, 0, 0);

  vkCmdEndRenderPass(mRenderData.rdCommandBuffer);

  if (vkEndCommandBuffer(mRenderData.rdCommandBuffer) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to end command buffer\n", __FUNCTION__);
    return false;
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.pWaitDstStageMask = &waitStage;

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &mRenderData.rdPresentSemaphore;

  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &mRenderData.rdRenderSemaphore;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &mRenderData.rdCommandBuffer;

  if (vkQueueSubmit(mRenderData.rdGraphicsQueue, 1, &submitInfo, mRenderData.rdRenderFence) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to submit draw command buffer\n", __FUNCTION__);
    return false;
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &mRenderData.rdRenderSemaphore;

  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &mRenderData.rdVkbSwapchain.swapchain;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(mRenderData.rdPresentQueue, &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    return recreateSwapchain(mRenderData);
  } else {
    if (result != VK_SUCCESS) {
      Logger::log(1, "%s error: failed to present swapchain image\n", __FUNCTION__);
      return false;
    }
  }

  return true;
}
