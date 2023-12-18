#include <imgui_impl_glfw.h>

#include <glm/gtc/matrix_transform.hpp>

#include <ctime>
#include <cstdlib>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "VkRenderer.h"
#include "ModelSettings.h"
#include "Logger.h"

VkRenderer::VkRenderer(GLFWwindow *window) {
  mRenderData.rdWindow = window;

  mPerspViewMatrices.emplace_back(glm::mat4(1.0f)); // view matrix
  mPerspViewMatrices.emplace_back(glm::mat4(1.0f)); // perspective matrix
}

bool VkRenderer::init(unsigned int width, unsigned int height) {
  /* randomize rand() */
  std::srand(static_cast<int>(time(NULL)));

  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  if (!mRenderData.rdWindow) {
    Logger::log(1, "%s error: invalid GLFWwindow handle\n", __FUNCTION__);
    return false;
  }

  if (!deviceInit()) {
    return false;
  }

  if (!initVma()) {
    return false;
  }

  if (!getQueue()) {
    return false;
  }

  if (!createSwapchain()) {
    return false;
  }

  /* must be done AFTER swapchain as we need data from it */
  if (!createDepthBuffer()) {
    return false;
  }

  if (!createCommandPool()) {
    return false;
  }

  if (!createCommandBuffer()) {
    return false;
  }

  if (!createUBO()) {
    return false;
  }
  /* before pipeline layout and pipeline */

  if (!loadGltfModel()) {
    return false;
  }

  if (!createInstances()) {
    return false;
  }

  if (!createMatrixSSBO()) {
    return false;
  }

  if (!createDQSSBO()) {
    return false;
  }

  if (!createVBO()) {
    return false;
  }

  if (!createRenderPass()) {
    return false;
  }

  if (!createGltfPipelineLayout()) {
      return false;
  }

  if (!createLinePipeline()) {
      return false;
  }

  if (!createGltfSkeletonPipeline()) {
      return false;
  }

  if (!createGltfGPUPipeline()) {
      return false;
  }

  if (!createGltfGPUDQPipeline()) {
      return false;
  }

  if (!createFramebuffer()) {
    return false;
  }

  if (!createSyncObjects()) {
    return false;
  }

  if (!initUserInterface()) {
    return false;
  }

  /* valid, but emtpy */
  mLineMesh = std::make_shared<VkMesh>();
  Logger::log(1, "%s: line mesh storage initialized\n", __FUNCTION__);

  mFrameTimer.start();

  Logger::log(1, "%s: Vulkan renderer initialized to %ix%i\n", __FUNCTION__, width, height);
  return true;
}

bool VkRenderer::deviceInit() {
  /* instance and window - we need Vukan 1.1 for the "VK_KHR_maintenance1" extension */
  vkb::InstanceBuilder instBuild;
  auto instRet = instBuild.use_default_debug_messenger().request_validation_layers().require_api_version(1, 1, 0).build();
  if (!instRet) {
    Logger::log(1, "%s error: could not build vkb instance\n", __FUNCTION__);
    return false;
  }
  mRenderData.rdVkbInstance = instRet.value();

  VkResult result = VK_ERROR_UNKNOWN;
  result = glfwCreateWindowSurface(mRenderData.rdVkbInstance, mRenderData.rdWindow, nullptr, &mSurface);
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
  mRenderData.rdVkbPhysicalDevice = secondPhysicalDevSelRet.value();

  Logger::log(1, "%s: found physical device '%s'\n", __FUNCTION__, mRenderData.rdVkbPhysicalDevice.name.c_str());

  mMinUniformBufferOffsetAlignment = mRenderData.rdVkbPhysicalDevice.properties.limits.minUniformBufferOffsetAlignment;
  Logger::log(1, "%s: the psyical device as a minimal unifom buffer offset of %i bytes\n", __FUNCTION__, mMinUniformBufferOffsetAlignment);

  vkb::DeviceBuilder devBuilder{mRenderData.rdVkbPhysicalDevice};
  auto devBuilderRet = devBuilder.build();
  if (!devBuilderRet) {
    Logger::log(1, "%s error: could not get devices\n", __FUNCTION__);
    return false;
  }
  mRenderData.rdVkbDevice = devBuilderRet.value();

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

bool VkRenderer::createDepthBuffer() {
  VkExtent3D depthImageExtent = {
        mRenderData.rdVkbSwapchain.extent.width,
        mRenderData.rdVkbSwapchain.extent.height,
        1
  };

  mRenderData.rdDepthFormat = VK_FORMAT_D32_SFLOAT;

  VkImageCreateInfo depthImageInfo{};
  depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
  depthImageInfo.format = mRenderData.rdDepthFormat;
  depthImageInfo.extent = depthImageExtent;
  depthImageInfo.mipLevels = 1;
  depthImageInfo.arrayLayers = 1;
  depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VmaAllocationCreateInfo depthAllocInfo{};
  depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  depthAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vmaCreateImage(mRenderData.rdAllocator, &depthImageInfo, &depthAllocInfo, &mRenderData.rdDepthImage, &mRenderData.rdDepthImageAlloc, nullptr) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not allocate depth buffer memory\n", __FUNCTION__);
    return false;
  }

  VkImageViewCreateInfo depthImageViewinfo{};
  depthImageViewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depthImageViewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depthImageViewinfo.image = mRenderData.rdDepthImage;
  depthImageViewinfo.format = mRenderData.rdDepthFormat;
  depthImageViewinfo.subresourceRange.baseMipLevel = 0;
  depthImageViewinfo.subresourceRange.levelCount = 1;
  depthImageViewinfo.subresourceRange.baseArrayLayer = 0;
  depthImageViewinfo.subresourceRange.layerCount = 1;
  depthImageViewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

  if (vkCreateImageView(mRenderData.rdVkbDevice.device, &depthImageViewinfo, nullptr, &mRenderData.rdDepthImageView) != VK_SUCCESS) {
    Logger::log(1, "%s error: could not create depth buffer image view\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createSwapchain() {
  vkb::SwapchainBuilder swapChainBuild{mRenderData.rdVkbDevice};

  /* VK_PRESENT_MODE_FIFO_KHR enables vsync */
  auto  swapChainBuildRet = swapChainBuild.set_old_swapchain(mRenderData.rdVkbSwapchain).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).build();
  if (!swapChainBuildRet) {
    Logger::log(1, "%s error: could not init swapchain\n", __FUNCTION__);
    return false;
  }

  vkb::destroy_swapchain(mRenderData.rdVkbSwapchain);
  mRenderData.rdVkbSwapchain = swapChainBuildRet.value();

  return true;
}

bool VkRenderer::recreateSwapchain() {
  /* handle minimize */
  while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0) {
    glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

  /* cleanup */
  Framebuffer::cleanup(mRenderData);
  vkDestroyImageView(mRenderData.rdVkbDevice.device, mRenderData.rdDepthImageView, nullptr);
  vmaDestroyImage(mRenderData.rdAllocator, mRenderData.rdDepthImage, mRenderData.rdDepthImageAlloc);

  mRenderData.rdVkbSwapchain.destroy_image_views(mRenderData.rdSwapchainImageViews);

  /* and recreate */
  if (!createSwapchain()) {
    Logger::log(1, "%s error: could not recreate swapchain\n", __FUNCTION__);
    return false;
  }

  if (!createDepthBuffer()) {
    Logger::log(1, "%s error: could not recreate depth buffer\n", __FUNCTION__);
    return false;
  }

  if (!createFramebuffer()) {
    Logger::log(1, "%s error: could not recreate framebuffers\n", __FUNCTION__);
    return false;
  }

  return true;
}

bool VkRenderer::createVBO() {
  /* init with arbitrary size here */
  if (!VertexBuffer::init(mRenderData, mRenderData.rdVertexBufferData, 2000)) {
    Logger::log(1, "%s error: could not create vertex buffer\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createUBO() {
  size_t matrixSize = mPerspViewMatrices.size() * sizeof(glm::mat4);
  if (!UniformBuffer::init(mRenderData, mRenderData.rdPerspViewMatrixUBO, matrixSize)) {
    Logger::log(1, "%s error: could not create uniform buffers\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createMatrixSSBO() {
  size_t modelJointMatrixBufferSize = 0;
  int jointMatrixSize = 0;

  for (const auto &instance : mGltfInstances) {
    jointMatrixSize += instance->getJointMatrixSize();
    modelJointMatrixBufferSize += instance->getJointMatrixSize() * sizeof(glm::mat4);
  }

  if (!ShaderStorageBuffer::init(mRenderData, mRenderData.rdJointMatrixSSBO, modelJointMatrixBufferSize)) {
    Logger::log(1, "%s error: could not create shader storage buffers\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createDQSSBO() {
  size_t modelJointDualQuatBufferSize = 0;
  int jointQuatSize = 0;

  for (const auto &instance : mGltfInstances) {
    jointQuatSize += instance->getJointDualQuatsSize();
    modelJointDualQuatBufferSize += instance->getJointDualQuatsSize() *
      sizeof(glm::mat2x4);
  }

  if (!ShaderStorageBuffer::init(mRenderData, mRenderData.rdJointDualQuatSSBO, modelJointDualQuatBufferSize)) {
    Logger::log(1, "%s error: could not create shader storage buffers\n", __FUNCTION__);
    return false;
  }

  return true;
}

bool VkRenderer::createRenderPass() {
  if (!Renderpass::init(mRenderData)) {
    Logger::log(1, "%s error: could not init renderpass\n", __FUNCTION__);
    return false;
  }
  return true;
}
bool VkRenderer::createGltfPipelineLayout() {
  VkTextureData texData = mGltfModels.at(0)->getVkTextureData();
  if (!PipelineLayout::init(mRenderData, texData, mRenderData.rdGltfPipelineLayout)) {
      Logger::log(1, "%s error: could not init pipeline layout\n", __FUNCTION__);
      return false;
  }
  return true;
}

bool VkRenderer::createLinePipeline() {
  std::string vertexShaderFile = "shader/line.vert.spv";
  std::string fragmentShaderFile = "shader/line.frag.spv";
  if (!Pipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout, mRenderData.rdLinePipeline,
      VK_PRIMITIVE_TOPOLOGY_LINE_LIST, vertexShaderFile, fragmentShaderFile)) {
      Logger::log(1, "%s error: could not init line shader pipeline\n", __FUNCTION__);
      return false;
  }
  return true;
}

bool VkRenderer::createGltfSkeletonPipeline() {
  std::string vertexShaderFile = "shader/line.vert.spv";
  std::string fragmentShaderFile = "shader/line.frag.spv";
  if (!GltfSkeletonPipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout,
      mRenderData.rdGltfSkeletonPipeline, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
      vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init gltf skeleton shader pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createGltfGPUPipeline() {
  std::string vertexShaderFile = "shader/gltf_gpu.vert.spv";
  std::string fragmentShaderFile = "shader/gltf_gpu.frag.spv";
  if (!GltfGPUPipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout,
      mRenderData.rdGltfGPUPipeline, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init gltf GPU shader pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createGltfGPUDQPipeline() {
  std::string vertexShaderFile = "shader/gltf_gpu_dquat.vert.spv";
  std::string fragmentShaderFile = "shader/gltf_gpu_dquat.frag.spv";
  if (!GltfGPUPipeline::init(mRenderData, mRenderData.rdGltfPipelineLayout,
      mRenderData.rdGltfGPUDQPipeline, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      vertexShaderFile, fragmentShaderFile)) {
    Logger::log(1, "%s error: could not init gltf GPU dual quat shader pipeline\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createFramebuffer() {
  if (!Framebuffer::init(mRenderData)) {
    Logger::log(1, "%s error: could not init framebuffer\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createCommandPool() {
  if (!CommandPool::init(mRenderData)) {
    Logger::log(1, "%s error: could not create command pool\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createCommandBuffer() {
  if (!CommandBuffer::init(mRenderData, mRenderData.rdCommandBuffer)) {
    Logger::log(1, "%s error: could not create command buffers\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::createSyncObjects() {
  if (!SyncObjects::init(mRenderData)) {
    Logger::log(1, "%s error: could not create sync objects\n", __FUNCTION__);
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

bool VkRenderer::initUserInterface() {
  if (!mUserInterface.init(mRenderData)) {
    Logger::log(1, "%s error: could not init ImGui\n", __FUNCTION__);
    return false;
  }
  return true;
}

bool VkRenderer::loadGltfModel() {
  /* load 2x the woman and the sq model */
  mGltfModels.resize(3);

  mGltfModels.at(0) = std::make_shared<GltfModel>();
  std::string modelFilename = "assets/Woman.gltf";
  std::string modelTexFilename = "textures/Woman.png";
  if (!mGltfModels.at(0)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__,
      modelFilename.c_str());
    return false;
  }

  mGltfModels.at(1) = std::make_shared<GltfModel>();
  modelTexFilename = "textures/Woman2.png";
  if (!mGltfModels.at(1)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__,
      modelFilename.c_str());
    return false;
  }

  mGltfModels.at(2) = std::make_shared<GltfModel>();
  modelFilename = "assets/dq.gltf";
  modelTexFilename = "textures/dq.png";
  if (!mGltfModels.at(2)->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__,
      modelFilename.c_str());
    return false;
  }

  return true;
}

bool VkRenderer::createInstances() {
  int numTriangles = 0;

  /* create glTF instances from the model */
  for (int i = 0; i < 200; ++i) {
    int xPos = std::rand() % 40 - 20;
    int zPos = std::rand() % 40 - 20;
    int modelNo = std::rand() % 2;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(modelNo),
      glm::vec2(static_cast<float>(xPos), static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(modelNo)->getTriangleCount();
  }

  for (int i = 0; i < 25; ++i) {
    int xPos = std::rand() % 50 - 25;
    int zPos = std::rand() % 20 - 50;
    mGltfInstances.emplace_back(std::make_shared<GltfInstance>(mGltfModels.at(2),
      glm::vec2(static_cast<float>(xPos), static_cast<float>(zPos)), true));
    numTriangles += mGltfModels.at(2)->getTriangleCount();
  }

  mRenderData.rdTriangleCount = numTriangles;
  mRenderData.rdNumberOfInstances = mGltfInstances.size();

  if (!mGltfInstances.size()) {
    Logger::log(1, "%s: glTF instance creation failed\n", __FUNCTION__);
    return false;
  }

  return true;
}

void VkRenderer::cleanup() {
  vkDeviceWaitIdle(mRenderData.rdVkbDevice.device);

  for (int i = 0; i < mGltfModels.size(); ++i) {
    mGltfModels.at(i)->cleanup(mRenderData);
    mGltfModels.at(i).reset();
  }

  mUserInterface.cleanup(mRenderData);

  SyncObjects::cleanup(mRenderData);
  CommandBuffer::cleanup(mRenderData, mRenderData.rdCommandBuffer);
  CommandPool::cleanup(mRenderData);
  Framebuffer::cleanup(mRenderData);
  GltfGPUPipeline::cleanup(mRenderData, mRenderData.rdGltfGPUDQPipeline);
  GltfGPUPipeline::cleanup(mRenderData, mRenderData.rdGltfGPUPipeline);
  GltfSkeletonPipeline::cleanup(mRenderData, mRenderData.rdGltfSkeletonPipeline);
  Pipeline::cleanup(mRenderData, mRenderData.rdLinePipeline);
  PipelineLayout::cleanup(mRenderData, mRenderData.rdGltfPipelineLayout);
  Renderpass::cleanup(mRenderData);
  UniformBuffer::cleanup(mRenderData, mRenderData.rdPerspViewMatrixUBO);
  ShaderStorageBuffer::cleanup(mRenderData, mRenderData.rdJointDualQuatSSBO);
  ShaderStorageBuffer::cleanup(mRenderData, mRenderData.rdJointMatrixSSBO);
  VertexBuffer::cleanup(mRenderData, mRenderData.rdVertexBufferData);

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

  if (vkWaitForFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    Logger::log(1, "%s error: waiting for fence failed\n", __FUNCTION__);
    return false;
  }

  if (vkResetFences(mRenderData.rdVkbDevice.device, 1, &mRenderData.rdRenderFence) != VK_SUCCESS) {
    Logger::log(1, "%s error:  fence reset failed\n", __FUNCTION__);
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
    return recreateSwapchain();
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

  mPerspViewMatrices.at(0) = mCamera.getViewMatrix(mRenderData);
  mPerspViewMatrices.at(1) = glm::perspective(
    glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
    static_cast<float>(mRenderData.rdVkbSwapchain.extent.width) /
    static_cast<float>(mRenderData.rdVkbSwapchain.extent.height), 0.01f, 500.0f);

  /* animate and update inverse kinematics */
  mRenderData.rdIKTime = 0.0f;
  for (auto &instance : mGltfInstances) {
    instance->updateAnimation();

    mIKTimer.start();
    instance->solveIK();
    mRenderData.rdIKTime += mIKTimer.stop();
  }

  /* save value to avoid changes during later calls */
  int selectedInstance = mRenderData.rdCurrentSelectedInstance;
  glm::vec2 modelWorldPos = mGltfInstances.at(selectedInstance)->getWorldPosition();
  glm::quat modelWorldRot = mGltfInstances.at(selectedInstance)->getWorldRotation();

  mLineMesh->vertices.clear();

  /* get gltTF skeleton */
  mSkeletonLineIndexCount = 0;
  for (const auto &instance : mGltfInstances) {
    ModelSettings settings = instance->getInstanceSettings();
    if (settings.msDrawSkeleton) {
      std::shared_ptr<VkMesh> mesh = instance->getSkeleton();
      mSkeletonLineIndexCount += mesh->vertices.size();
      mLineMesh->vertices.insert(mLineMesh->vertices.begin(),
        mesh->vertices.begin(), mesh->vertices.end());
    }
  }

  /* get coordinate arrows for the IK target of current instance only */
  mCoordArrowsLineIndexCount = 0;
  {
    ModelSettings ikSettings = mGltfInstances.at(selectedInstance)->getInstanceSettings();
    if (ikSettings.msIkMode == ikMode::ccd ||
        ikSettings.msIkMode == ikMode::fabrik) {
      mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
      mCoordArrowsLineIndexCount += mCoordArrowsMesh.vertices.size();
      std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
        [=](auto &n){
          n.color /= 2.0f;
          n.position = modelWorldRot * n.position;
          n.position += ikSettings.msIkTargetWorldPos;
      });

      mLineMesh->vertices.insert(mLineMesh->vertices.end(),
        mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());
    }
  }

  /* draw coordiante arrows*/
  mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
  mCoordArrowsLineIndexCount += mCoordArrowsMesh.vertices.size();
  std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
    [=](auto &n){
      n.color /= 2.0f;
      n.position = modelWorldRot * n.position;
      n.position += glm::vec3(modelWorldPos.x, 0.0f, modelWorldPos.y);
  });

  mLineMesh->vertices.insert(mLineMesh->vertices.end(),
    mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());

  mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

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

  if (mLineMesh->vertices.size() > 0) {
    VertexBuffer::uploadData(mRenderData, mRenderData.rdVertexBufferData, *mLineMesh);
  }

  if (mModelUploadRequired) {
    for (int i = 0; i < mGltfModels.size(); ++i) {
    /* upload glTF model data */
      mGltfModels.at(i)->uploadVertexBuffers(mRenderData);
      mGltfModels.at(i)->uploadIndexBuffer(mRenderData);
    }
    mModelUploadRequired = false;
  }

  mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

  /* prepare the vectors with matrix and dual quat data, update triangle count */
  mModelJointMatrices.clear();
  mModelJointDualQuats.clear();

  mGltfMatrixInstances.clear();
  mGltfDQInstances.clear();
  unsigned int numTriangles = 0;

  for (const auto &instance : mGltfInstances) {
    ModelSettings settings = instance->getInstanceSettings();
    if (!settings.msDrawModel) {
      continue;
    }

    if (settings.msVertexSkinningMode == skinningMode::dualQuat) {
      std::vector<glm::mat2x4> quats = instance->getJointDualQuats();
      mModelJointDualQuats.insert(mModelJointDualQuats.end(),
        quats.begin(), quats.end());
      mGltfDQInstances.emplace_back(instance);
    } else {
      std::vector<glm::mat4> mats = instance->getJointMatrices();
      mModelJointMatrices.insert(mModelJointMatrices.end(),
        mats.begin(), mats.end());
      mGltfMatrixInstances.emplace_back(instance);
    }
    numTriangles += instance->getModel()->getTriangleCount();
  }

  mRenderData.rdTriangleCount = numTriangles;

  /* the rendering itself happens here */
  vkCmdBeginRenderPass(mRenderData.rdCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

  /* required for dynamic viewport */
  vkCmdSetViewport(mRenderData.rdCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(mRenderData.rdCommandBuffer, 0, 1, &scissor);

  /* UBOs */
  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    mRenderData.rdGltfPipelineLayout, 1, 1,
      &mRenderData.rdPerspViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);
  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    mRenderData.rdGltfPipelineLayout, 2, 1, &mRenderData.rdJointMatrixSSBO.rdSSBODescriptorSet,
    0, nullptr);
  vkCmdBindDescriptorSets(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    mRenderData.rdGltfPipelineLayout, 3, 1,
      &mRenderData.rdJointDualQuatSSBO.rdSSBODescriptorSet, 0, nullptr);

  /* line and skeleton vertex buffer */
  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1,
    &mRenderData.rdVertexBufferData.rdVertexBuffer, &offset);

  /* draw the glTF models */
  VkPushConstants modelStride;
  modelStride.pkModelStride = 0;

  vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
   mRenderData.rdGltfGPUPipeline);
  for (const auto &instance : mGltfMatrixInstances) {
    /* set position inside the SSBO */
    vkCmdPushConstants(mRenderData.rdCommandBuffer, mRenderData.rdGltfPipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkPushConstants), &modelStride);

    instance->getModel()->draw(mRenderData);
    modelStride.pkModelStride += instance->getJointMatrixSize();
  }

  modelStride.pkModelStride = 0;

  vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    mRenderData.rdGltfGPUDQPipeline);
  for (const auto &instance : mGltfDQInstances) {
    vkCmdPushConstants(mRenderData.rdCommandBuffer, mRenderData.rdGltfPipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VkPushConstants), &modelStride);

    instance->getModel()->draw(mRenderData);
    modelStride.pkModelStride += instance->getJointMatrixSize();
  }

  if (mCoordArrowsLineIndexCount > 0 || mSkeletonLineIndexCount > 0) {
    vkCmdBindVertexBuffers(mRenderData.rdCommandBuffer, 0, 1,
      &mRenderData.rdVertexBufferData.rdVertexBuffer, &offset);
    vkCmdSetLineWidth(mRenderData.rdCommandBuffer, 3.0f);
  }

  /* draw the coordinate arrow WITH depth buffer */
  if (mCoordArrowsLineIndexCount > 0) {
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      mRenderData.rdLinePipeline);
    vkCmdDraw(mRenderData.rdCommandBuffer, mCoordArrowsLineIndexCount, 1,
      mSkeletonLineIndexCount, 0);
  }

  /* draw the skeleton last, disable depth test to overlay */
  if (mSkeletonLineIndexCount > 0) {
    vkCmdBindPipeline(mRenderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      mRenderData.rdGltfSkeletonPipeline);
    vkCmdDraw(mRenderData.rdCommandBuffer, mSkeletonLineIndexCount, 1, 0, 0);
  }

  /* imgui overlay */
  mUIGenerateTimer.start();

  ModelSettings settings = mGltfInstances.at(selectedInstance)->getInstanceSettings();
  mUserInterface.createFrame(mRenderData, settings);
  mGltfInstances.at(selectedInstance)->setInstanceSettings(settings);
  mGltfInstances.at(selectedInstance)->checkForUpdates();

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

  UniformBuffer::uploadData(mRenderData, mRenderData.rdPerspViewMatrixUBO, mPerspViewMatrices);

  ShaderStorageBuffer::uploadData(mRenderData, mRenderData.rdJointDualQuatSSBO, mModelJointDualQuats);
  ShaderStorageBuffer::uploadData(mRenderData, mRenderData.rdJointMatrixSSBO, mModelJointMatrices);

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
    return recreateSwapchain();
  } else {
    if (result != VK_SUCCESS) {
      Logger::log(1, "%s error: failed to present swapchain image\n", __FUNCTION__);
      return false;
    }
  }
  mLastTickTime = tickTime;

  return true;
}
