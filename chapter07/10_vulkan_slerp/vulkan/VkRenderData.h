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

struct VkUploadMatrices {
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
};

struct VkRenderData {
  GLFWwindow *rdWindow = nullptr;

  int rdWidth = 0;
  int rdHeight = 0;

  unsigned int rdTriangleCount = 0;

  int rdFieldOfView = 60;

  float rdFrameTime = 0.0f;
  float rdMatrixGenerateTime = 0.0f;
  float rdUploadToVBOTime = 0.0f;
  float rdUploadToUBOTime = 0.0f;
  float rdUIGenerateTime = 0.0f;
  float rdUIDrawTime = 0.0f;

  int rdMoveForward = 0;
  int rdMoveRight = 0;
  int rdMoveUp = 0;

  float rdTickDiff = 0.0f;

  float rdViewAzimuth = 0.0f;
  float rdViewElevation = -30.0f;
  glm::vec3 rdCameraWorldPosition = glm::vec3(-1.25f, 2.00f, 2.5f);

  bool rdDrawWorldCoordArrows = true;
  bool rdDrawModelCoordArrows = true;
  bool rdResetAnglesAndInterp = false;

  std::vector<int> rdRotXAngle = { 0, 0 };
  std::vector<int> rdRotYAngle = { 0, 0 };
  std::vector<int> rdRotZAngle = { 0, 0 };
  float rdInterpValue = 0.0f;

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
  VkPipelineLayout rdPipelineLayout = VK_NULL_HANDLE;
  VkPipeline rdBasicPipeline = VK_NULL_HANDLE;
  VkPipeline rdLinePipeline = VK_NULL_HANDLE;

  VkCommandPool rdCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer rdCommandBuffer = VK_NULL_HANDLE;

  VkSemaphore rdPresentSemaphore = VK_NULL_HANDLE;
  VkSemaphore rdRenderSemaphore = VK_NULL_HANDLE;
  VkFence rdRenderFence = VK_NULL_HANDLE;

  VkImage rdTextureImage = VK_NULL_HANDLE;
  VkImageView rdTextureImageView = VK_NULL_HANDLE;
  VkSampler rdTextureSampler = VK_NULL_HANDLE;
  VmaAllocation rdTextureImageAlloc = nullptr;

  VkDescriptorPool rdTextureDescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout rdTextureDescriptorLayout = VK_NULL_HANDLE;
  VkDescriptorSet rdTextureDescriptorSet = VK_NULL_HANDLE;

  unsigned int rdVertexBufferSize = 2048;
  VkBuffer rdVertexBuffer = VK_NULL_HANDLE;
  VmaAllocation rdVertexBufferAlloc = nullptr;
  VkBuffer rdVertexStagingBuffer = VK_NULL_HANDLE;
  VmaAllocation rdVertexStagingBufferAlloc = nullptr;

  VkBuffer rdUboBuffer = VK_NULL_HANDLE;
  VmaAllocation rdUboBufferAlloc = nullptr;

  VkDescriptorPool rdUBODescriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout rdUBODescriptorLayout = VK_NULL_HANDLE;
  VkDescriptorSet rdUBODescriptorSet = VK_NULL_HANDLE;

  VkDescriptorPool rdImguiDescriptorPool = VK_NULL_HANDLE;
};
