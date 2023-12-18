/* Vulkan renderer */
#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
/* Vulkan also before GLFW */
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include "VkRenderData.h"
#include "Renderpass.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"

class VkRenderer {
  public:
    VkRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    bool uploadData(VkMesh vertexData);
    bool draw();
    void cleanup();

  private:
    VkRenderData mRenderData{};

    int mTriangleCount = 0;

    GLFWwindow *mWindow = nullptr;

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    vkb::PhysicalDevice mPhysDevice;

    VkBuffer mVertexBuffer;
    VmaAllocation mVertexBufferAlloc;

    bool deviceInit();
    bool getQueue();
    bool createDepthBuffer();
    bool createSwapchain();
    bool createRenderPass();
    bool createPipeline();
    bool createFramebuffer();
    bool createCommandPool();
    bool createCommandBuffer();
    bool createSyncObjects();
    bool loadTexture();

    bool initVma();

    bool recreateSwapchain();
};
