/* Vulkan */
#pragma once
#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

struct VkVertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 uv;
};

struct VkMesh {
  std::vector<VkVertex> vertices;
};

enum class skinningMode {
  linear = 0,
  dualQuat
};

enum class replayDirection {
  forward = 0,
  backward
};

enum class blendMode {
  fadeinout = 0,
  crossfade,
  additive
};

enum class ikMode {
  off = 0,
  ccd,
  fabrik
};

struct VkTextureData {
  VkImage texTextureImage = VK_NULL_HANDLE;
  VkImageView texTextureImageView = VK_NULL_HANDLE;
  VkSampler texTextureSampler = VK_NULL_HANDLE;
  VmaAllocation texTextureImageAlloc = nullptr;

  VkDescriptorPool texTextureDescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout texTextureDescriptorLayout = VK_NULL_HANDLE;
  VkDescriptorSet texTextureDescriptorSet = VK_NULL_HANDLE;
};

struct VkVertexBufferData {
  unsigned int rdVertexBufferSize = 0;
	VkBuffer rdVertexBuffer = VK_NULL_HANDLE;
	VmaAllocation rdVertexBufferAlloc = nullptr;
	VkBuffer rdStagingBuffer = VK_NULL_HANDLE;
	VmaAllocation rdStagingBufferAlloc = nullptr;
};

struct VkIndexBufferData {
  unsigned int rdIndexBufferSize = 0;
	VkBuffer rdIndexBuffer = VK_NULL_HANDLE;
	VmaAllocation rdIndexBufferAlloc = nullptr;
	VkBuffer rdStagingBuffer = VK_NULL_HANDLE;
	VmaAllocation rdStagingBufferAlloc = nullptr;
};

struct VkUniformBufferData {
  VkBuffer rdUboBuffer = VK_NULL_HANDLE;
  VmaAllocation rdUboBufferAlloc = nullptr;

  VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
  VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;
};

struct VkShaderStorageBufferData {
  VkBuffer rdSsboBuffer = VK_NULL_HANDLE;
  VmaAllocation rdSsboBufferAlloc = nullptr;

  VkDescriptorPool rdSSBODescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout rdSSBODescriptorLayout = VK_NULL_HANDLE;
  VkDescriptorSet rdSSBODescriptorSet = VK_NULL_HANDLE;
};

struct VkRenderData {
  GLFWwindow *rdWindow = nullptr;

  int rdWidth = 0;
  int rdHeight = 0;

  unsigned int rdTriangleCount = 0;
  unsigned int rdGltfTriangleCount = 0;

  int rdFieldOfView = 60;

  float rdFrameTime = 0.0f;
  float rdMatrixGenerateTime = 0.0f;
  float rdIKTime = 0.0f;
  float rdUploadToVBOTime = 0.0f;
  float rdUploadToUBOTime = 0.0f;
  float rdUIGenerateTime = 0.0f;
  float rdUIDrawTime = 0.0f;

  int rdMoveForward = 0;
  int rdMoveRight = 0;
  int rdMoveUp = 0;

  float rdTickDiff = 0.0f;

  float rdViewAzimuth = 0.0f;
  float rdViewElevation = 0.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(-0.5f, 2.5f, 6.0f);

  bool rdDrawGltfModel = true;
  bool rdDrawSkeleton = true;
  skinningMode rdGPUDualQuatVertexSkinning = skinningMode::linear;

  bool rdPlayAnimation = true;
  std::vector<std::string> rdClipNames{};
  int rdAnimClip = 0;
  int rdAnimClipSize = 0;
  float rdAnimSpeed = 1.0f;
  float rdAnimTimePosition = 0.0f;
  float rdAnimEndTime = 0.0f;
  int rdModelNodeCount = 0;

  replayDirection rdAnimationPlayDirection = replayDirection::forward;

  float rdAnimBlendFactor = 1.0f;

  blendMode rdBlendingMode = blendMode::fadeinout;
  int rdCrossBlendDestAnimClip = 0;
  float rdAnimCrossBlendFactor = 0.0f;

  int rdSkelSplitNode = 0;
  std::vector<std::string> rdSkelNodeNames{};

  ikMode rdIkMode = ikMode::off;
  int rdIkIterations = 10;
  glm::vec3 rdIkTargetPos = glm::vec3(0.0f, 3.0f, 1.0f);
  int rdIkEffectorNode = 0;
  int rdIkRootNode = 0;

  VmaAllocator rdAllocator = nullptr;

  vkb::Instance rdVkbInstance{};
  vkb::PhysicalDevice rdVkbPhysicalDevice{};
  vkb::Device rdVkbDevice{};
  vkb::Swapchain rdVkbSwapchain{};

  std::vector<VkImage> rdSwapchainImages;
  std::vector<VkImageView> rdSwapchainImageViews;
  std::vector<VkFramebuffer> rdFramebuffers;

  VkQueue rdGraphicsQueue = VK_NULL_HANDLE;
  VkQueue rdPresentQueue = VK_NULL_HANDLE;

  VkImage rdDepthImage = VK_NULL_HANDLE;
  VkImageView rdDepthImageView = VK_NULL_HANDLE;
  VkFormat rdDepthFormat;
  VmaAllocation rdDepthImageAlloc = VK_NULL_HANDLE;

  VkRenderPass rdRenderpass;
  VkPipelineLayout rdGltfPipelineLayout = VK_NULL_HANDLE;
  VkPipeline rdLinePipeline = VK_NULL_HANDLE;
  VkPipeline rdGltfGPUPipeline = VK_NULL_HANDLE;
  VkPipeline rdGltfGPUDQPipeline = VK_NULL_HANDLE;
  VkPipeline rdGltfSkeletonPipeline = VK_NULL_HANDLE;

  VkCommandPool rdCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

  VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
  VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
  VkFence rdRenderFence = VK_NULL_HANDLE;

  VkVertexBufferData rdVertexBufferData{};

  VkUniformBufferData rdPerspViewMatrixUBO{};
  VkShaderStorageBufferData rdJointMatrixSSBO{};
  VkShaderStorageBufferData rdJointDualQuatSSBO{};

  VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};

struct VkGltfRenderData {
  std::vector<VkVertexBufferData> rdGltfVertexBufferData{};
  VkIndexBufferData rdGltfIndexBufferData{};
	VkTextureData rdGltfModelTexture{};
};
