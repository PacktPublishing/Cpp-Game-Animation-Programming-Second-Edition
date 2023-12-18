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
#include "PipelineLayout.h"
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
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void cleanup();

  private:
    VkRenderData mRenderData{};

    int mTriangleCount = 0;

    GLFWwindow *mWindow = nullptr;

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    vkb::PhysicalDevice mPhysDevice;
    VkDeviceSize mMinUniformBufferOffsetAlignment;

    VkBuffer mVertexBuffer;
    VmaAllocation mVertexBufferAlloc;

    VkUploadMatrices mMatrices;

    bool mUseChangedShader = false;

    bool deviceInit();
    bool getQueue();
    bool createDepthBuffer();
    bool createSwapchain();
    bool createRenderPass();
    bool createPipelineLayout();
    bool createBasicPipeline();
    bool createChangedPipeline();
    bool createFramebuffer();
    bool createCommandPool();
    bool createCommandBuffer();
    bool createSyncObjects();
    bool loadTexture();

    bool initVma();

    bool recreateSwapchain();
};
