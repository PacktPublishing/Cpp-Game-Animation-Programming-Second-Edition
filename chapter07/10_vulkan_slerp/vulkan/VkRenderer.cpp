#include <imgui_impl_glfw.h>

#include <glm/gtc/matrix_transform.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "VkRenderer.h"
#include "Logger.h"

VkRenderer::VkRenderer(GLFWwindow *window) {
  mRenderData.rdWindow = window;

  /* identity matrices */
  mMatrices.viewMatrix = glm::mat4(1.0f);
  mMatrices.projectionMatrix = glm::mat4(1.0f);
}

bool VkRenderer::init(unsigned int width, unsigned int height) {
  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  if (!mRenderData.rdWindow) {
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

  if (!createUBO(mRenderData)) {
      return false;
  }

  if (!createVBO(mRenderData)) {
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

  if (!createLinePipeline(mRenderData)) {
    return false;
  }

  if (!createFramebuffer(mRenderData)) {
    return false;
  }

  if (!createSyncObjects(mRenderData)) {
    return false;
  }

  if (!initUserInterface(mRenderData)) {
    return false;
  }

  mModel = std::make_unique<Model>();

  mModelMesh = std::make_unique<VkMesh>();
  Logger::log(1, "%s: model mesh storage initialized\n", __FUNCTION__);

  mAllMeshes = std::make_unique<VkMesh>();
  Logger::log(1, "%s: global mesh storage initialized\n", __FUNCTION__);

  mFrameTimer.start();

  Logger::log(1, "%s: Vulkan renderer initialized to %ix%i\n", __FUNCTION__, width, height);
  return true;
}

bool VkRenderer::deviceInit(VkRenderData &renderData) {
  /* instance and window - we need Vukan 1.1 for the "VK_KHR_maintenance1" extension */
  vkb::InstanceBuilder instBuild;
  auto instRet = instBuild.use_default_debug_messenger().request_validation_layers().require_api_version(1, 1, 0).build();
  if (!instRet) {
    Logger::log(1, "%s error: could not build vkb instance\n", __FUNCTION__);
    return false;
  }
  renderData.rdVkbInstance = instRet.value();

  VkResult result = VK_ERROR_UNKNOWN;
  result = glfwCreateWindowSurface(renderData.rdVkbInstance, mRenderData.rdWindow, nullptr, &mSurface);
  if (result != VK_SUCCESS) {
    Logger::log(1, "%s error: Could not create Vulkan surface\n", __FUNCTION__);
    return false;
  }

  /* just get the first available device */
  vkb::PhysicalDeviceSelector physicalDevSel{mRenderData.rdVkbInstance};
  auto firstPysicalDevSelRet = physicalDevSel.set_surface(mSurface).select();
  if (!firstPysicalDevSelRet) {
    Logger::log(1, "%s error: could not get physical devices\n", __FUNCTION__);
    return false;
  }

  /* a 2nd call is required to enable all the supported features, like wideLines */
  VkPhysicalDeviceFeatures physFeatures;
  vkGetPhysicalDeviceFeatures(firstPysicalDevSelRet.value(), &physFeatures);

  auto secondPhysicalDevSelRet = physicalDevSel.set_surface(mSurface).set_required_features(physFeatures).select();
  if (!secondPhysicalDevSelRet) {
    Logger::log(1, "%s error: could not get physical devices\n", __FUNCTION__);
    return false;
  }
  renderData.rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();

  Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__, renderData.rdVkbPhysicalDevice.name.c_str());

  mMinUniformBufferOffsetAlignment = renderData.rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
  Logger::log(1, "%s: the psyical device as a minimal unifom buffer offset of %i bytes\n", __FUNCTION__, mMinUniformBufferOffsetAlignment);

  vkb::DeviceBuilder devBuilder{renderData.rdVkbPhysicalDevice};
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
  auto  swapChainBuildRet = swapChainBuild.set_old_swapchain(renderData.rdVkbSwapchain).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).build();
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
  while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0) {
    glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
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

bool VkRenderer::createVBO(VkRenderData& renderData) {
  if (!VertexBuffer::init(renderData)) {
    Logger::log(1, "%s error: could not create vertex buffer\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createUBO(VkRenderData& renderData) {
  if (!UniformBuffer::init(renderData)) {
    Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
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
  if (!Pipeline::init(renderData, renderData.rdPipelineLayout, renderData.rdBasicPipeline, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init basic shader pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createLinePipeline(VkRenderData &renderData) {
  std::string vertexShaderFile = "shader/line.vert.spv";
  std::string fragmentShaderFile = "shader/line.frag.spv";
  if (!Pipeline::init(renderData, renderData.rdPipelineLayout, renderData.rdLinePipeline, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init line shader pipeline\n", __FUNCTION__);
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
  allocatorInfo.physicalDevice = mRenderData.rdVkbPhysicalDevice.physical_device;
  allocatorInfo.device = mRenderData.rdVkbDevice.device;
  allocatorInfo.instance = mRenderData.rdVkbInstance.instance;
  if (vmaCreateAllocator(&allocatorInfo, &mRenderData.rdAllocator) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not init VMA\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::initUserInterface(VkRenderData& renderData) {
  if (!mUserInterface.init(renderData)) {
    Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
    return false;
  }
  return true;
}

void VkRenderer::cleanup() {
  vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

  mUserInterface.cleanup(mRenderData);

  Texture::cleanup(mRenderData);
  SyncObjects::cleanup(mRenderData);
  CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
  CommandPool::cleanup(mRenderData);
  Framebuffer::cleanup(mRenderData);
  Pipeline::cleanup(mRenderData, mRenderData.rdLinePipeline);
  Pipeline::cleanup(mRenderData, mRenderData.rdBasicPipeline);
  PipelineLayout::cleanup(mRenderData, mRenderData.rdPipelineLayout);
  Renderpass::cleanup(mRenderData);
  UniformBuffer::cleanup(mRenderData);
  VertexBuffer::cleanup(mRenderData);

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
  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  /* Vulkan detects changes and recreates swapchain */
  Logger::log(1, "%s: resized window to %ix%i\n", __FUNCTION__, width, height);
}

void VkRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
}

void VkRenderer::handleMouseButtonEvents(int button, int action, int mods) {
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  if (button >= 0 && button < ImGuiMouseButton_COUNT) {
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
  }

  /* hide from application */
  if (io.WantCaptureMouse) {
    return;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    mMouseLock = !mMouseLock;
    if (mMouseLock) {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      /* enable raw mode if possible */
      if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(mRenderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
      }
    } else {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void VkRenderer::handleMousePositionEvents(double xPos, double yPos){
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent((float)xPos, (float)yPos);

  /* hide from application */
  if (io.WantCaptureMouse) {
    return;
  }

  /* calculate relative movement from last position */
  int mouseMoveRelX = static_cast<int>(xPos) - mMouseXPos;
  int mouseMoveRelY = static_cast<int>(yPos) - mMouseYPos;

  if (mMouseLock) {
    mRenderData.rdViewAzimuth += mouseMoveRelX / 10.0;
    /* keep between 0 and 360 degree */
    if (mRenderData.rdViewAzimuth < 0.0) {
      mRenderData.rdViewAzimuth += 360.0;
    }
    if (mRenderData.rdViewAzimuth >= 360.0) {
      mRenderData.rdViewAzimuth -= 360.0;
    }

    mRenderData.rdViewElevation -= mouseMoveRelY / 10.0;
    /* keep between -89 and +89 degree */
    if (mRenderData.rdViewElevation > 89.0) {
      mRenderData.rdViewElevation = 89.0;
    }
    if (mRenderData.rdViewElevation < -89.0) {
      mRenderData.rdViewElevation = -89.0;
    }
  }

  /* save old values*/
  mMouseXPos = static_cast<int>(xPos);
  mMouseYPos = static_cast<int>(yPos);
}

void VkRenderer::handleMovementKeys() {
  /* hide from application */
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }

  mRenderData.rdMoveForward = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_W) == GLFW_PRESS) {
    mRenderData.rdMoveForward += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_S) == GLFW_PRESS) {
    mRenderData.rdMoveForward -= 1;
  }

  mRenderData.rdMoveRight = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_A) == GLFW_PRESS) {
    mRenderData.rdMoveRight -= 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_D) == GLFW_PRESS) {
    mRenderData.rdMoveRight += 1;
  }

  /* viewport Y swap, same as OpenGL */
  mRenderData.rdMoveUp = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS) {
    mRenderData.rdMoveUp += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS) {
    mRenderData.rdMoveUp -= 1;
  }

  /* speed up movement with shift */
  if ((glfwGetKey(mRenderData.rdWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
      (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
    mRenderData.rdMoveForward *= 4;
    mRenderData.rdMoveRight *= 4;
    mRenderData.rdMoveUp *= 4;
  }
}

bool VkRenderer::draw() {
  /* get time difference for movement */
  double tickTime = glfwGetTime();
  mRenderData.rdTickDiff = tickTime - mLastTickTime;

  mRenderData.rdFrameTime = mFrameTimer.stop();
  mFrameTimer.start();

  handleMovementKeys();

  mAllMeshes->vertices.clear();

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
    mRenderData.rdFrameTime = mFrameTimer.stop();
    return recreateSwapchain(mRenderData);
  } else {
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      Logger::log(1, "%s error: failed to acquire swapchain image. Error is '%i'\n", __FUNCTION__, result);
      return false;
    }
  }

  VkClearValue colorClearValue;
  colorClearValue.color = { { 0.25f, 0.25f, 0.25f, 1.0f } };

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

  /* use inverted viewport to have same coordinates as OpenGL */
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
  viewport.width = static_cast<float>(mRenderData.rdVkbSwapchain.extent.width);
  viewport.height = -static_cast<float>(mRenderData.rdVkbSwapchain.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = mRenderData.rdVkbSwapchain.extent;

  mMatrixGenerateTimer.start();
  mMatrices.projectionMatrix = glm::perspective(
    glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
    static_cast<float>(mRenderData.rdVkbSwapchain.extent.width) /
    static_cast<float>(mRenderData.rdVkbSwapchain.extent.height), 0.01f, 50.0f);

  mMatrices.viewMatrix = mCamera.getViewMatrix(mRenderData);
  mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

  /* reset all values to zero when UI button is pressed */
  if (mRenderData.rdResetAnglesAndInterp) {
    mRenderData.rdResetAnglesAndInterp = false;

    mRenderData.rdRotXAngle = { 0, 0 };
    mRenderData.rdRotYAngle = { 0, 0 };
    mRenderData.rdRotZAngle = { 0, 0 };

    mRenderData.rdInterpValue = 0.0f;
  }

  /* create quaternion from angles  */
  for (int i = 0; i < 2; ++i) {
    mQuatModelOrientation[i] = glm::normalize(glm::quat(glm::vec3(
      glm::radians(static_cast<float>(mRenderData.rdRotXAngle[i])),
      glm::radians(static_cast<float>(mRenderData.rdRotYAngle[i])),
      glm::radians(static_cast<float>(mRenderData.rdRotZAngle[i]))
    )));
    mQuatModelOrientationConjugate[i] = glm::conjugate(mQuatModelOrientation[i]);
  }

  /* interpolate between the two quaternions */
  mQuatMix = glm::slerp(mQuatModelOrientation[0],mQuatModelOrientation[1],
    mRenderData.rdInterpValue);
  mQuatMixConjugate = glm::conjugate(mQuatMix);

  /* draw a static coordinate system */
  mCoordArrowsMesh.vertices.clear();
  if (mRenderData.rdDrawWorldCoordArrows) {
    mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
    std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
      [=](auto &n){
        n.color /= 2.0f;
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());
  }

  mStartPosArrowMesh.vertices.clear();
  mEndPosArrowMesh.vertices.clear();
  mQuatPosArrowMesh.vertices.clear();
  if (mRenderData.rdDrawModelCoordArrows) {
    /* start position arrow */
    mStartPosArrowMesh = mArrowModel.getVertexData();
    std::for_each(mStartPosArrowMesh.vertices.begin(), mStartPosArrowMesh.vertices.end(),
      [=](auto &n){
        glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
        glm::quat newPosition =
          mQuatModelOrientation[0] * position * mQuatModelOrientationConjugate[0];
        n.position.x = newPosition.x;
        n.position.y = newPosition.y;
        n.position.z = newPosition.z;
        n.position += mQuatModelDist;
        n.color = glm::vec3(0.0f, 0.8f, 0.8f);
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mStartPosArrowMesh.vertices.begin(), mStartPosArrowMesh.vertices.end());

    /* end position arrow */
    mEndPosArrowMesh = mArrowModel.getVertexData();
    std::for_each(mEndPosArrowMesh.vertices.begin(), mEndPosArrowMesh.vertices.end(),
      [=](auto &n){
        glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
        glm::quat newPosition =
          mQuatModelOrientation[1] * position * mQuatModelOrientationConjugate[1];
        n.position.x = newPosition.x;
        n.position.y = newPosition.y;
        n.position.z = newPosition.z;
        n.position += mQuatModelDist;
        n.color = glm::vec3(0.8f, 0.8f, 0.0f);
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mEndPosArrowMesh.vertices.begin(), mEndPosArrowMesh.vertices.end());

    /* draw an arrow to show quaternion orientation changes */
    mQuatPosArrowMesh = mArrowModel.getVertexData();
    std::for_each(mQuatPosArrowMesh.vertices.begin(), mQuatPosArrowMesh.vertices.end(),
      [=](auto &n){
        glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
        glm::quat newPosition =
          mQuatMix * position * mQuatMixConjugate;
        n.position.x = newPosition.x;
        n.position.y = newPosition.y;
        n.position.z = newPosition.z;
        n.position += mQuatModelDist;
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mQuatPosArrowMesh.vertices.begin(), mQuatPosArrowMesh.vertices.end());
  }

  *mModelMesh = mModel->getVertexData();
  mRenderData.rdTriangleCount = mModelMesh->vertices.size() / 3;
  std::for_each(mModelMesh->vertices.begin(), mModelMesh->vertices.end(),
    [=](auto &n){
      glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
      glm::quat newPosition =
        mQuatMix * position * mQuatMixConjugate;
      n.position.x = newPosition.x;
      n.position.y = newPosition.y;
      n.position.z = newPosition.z;
      n.position += mQuatModelDist;
  });
  mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
    mModelMesh->vertices.begin(), mModelMesh->vertices.end());

  mLineIndexCount =
    mStartPosArrowMesh.vertices.size() + mEndPosArrowMesh.vertices.size() +
    mQuatPosArrowMesh.vertices.size() + mCoordArrowsMesh.vertices.size();

  /* prepare command buffer */
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

  /* upload data to VBO */
  mUploadToVBOTimer.start();
  VertexBuffer::uploadData(mRenderData, *mAllMeshes);
  mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

  /* the rendering itself happens here */
  vkCmdBeginRenderPass(mRenderData.rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

  /* required for dynamic viewport */
  vkCmdSetViewport(mRenderData.rdCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(mRenderData.rdCommandBuffer, 0, 1, &scissor);

  /* the triangle drawing itself */
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1, &mRenderData.rdVertexBuffer, &offset);

  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout, 0, 1, &mRenderData.rdTextureDescriptorSet, 0, nullptr);
  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdPipelineLayout, 1, 1, &mRenderData.rdUBODescriptorSet, 0, nullptr);

  /* draw lines first */
  if (mLineIndexCount > 0) {
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdLinePipeline);
    vkCmdSetLineWidth(mRenderData.rdCommandBuffer, 3.0f);
    vkCmdDraw(mRenderData.rdCommandBuffer, mLineIndexCount, 1, 0, 0);
  }

  /* draw model last */
  vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderData.rdBasicPipeline);
  vkCmdDraw(mRenderData.rdCommandBuffer, mRenderData.rdTriangleCount * 3, 1, mLineIndexCount, 0);

  // imgui overlay
  mUIGenerateTimer.start();
  mUserInterface.createFrame(mRenderData);
  mRenderData.rdUIGenerateTime = mUIGenerateTimer.stop();

  mUIDrawTimer.start();
  mUserInterface.render(mRenderData);
  mRenderData.rdUIDrawTime = mUIDrawTimer.stop();

  vkCmdEndRenderPass(mRenderData.rdCommandBuffer);

  if (vkEndCommandBuffer(mRenderData.rdCommandBuffer) != VK_SUCCESS) {
    Logger::log(1, "%s error: failed to end command buffer\n", __FUNCTION__);
    return false;
  }

  /* upload UBO data after commands are created */
  mUploadToUBOTimer.start();
  void* data;
  vmaMapMemory(mRenderData.rdAllocator, mRenderData.rdUboBufferAlloc, &data);
  memcpy(data, &mMatrices, static_cast<uint32_t>(sizeof(VkUploadMatrices)));
  vmaUnmapMemory(mRenderData.rdAllocator, mRenderData.rdUboBufferAlloc);
  mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

  /* submit command buffer */
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
    mRenderData.rdFrameTime = mFrameTimer.stop();
    return recreateSwapchain(mRenderData);
  } else {
    if (result != VK_SUCCESS) {
      Logger::log(1, "%s error: failed to present swapchain image\n", __FUNCTION__);
      return false;
    }
  }
  mLastTickTime = tickTime;

  return true;
}
