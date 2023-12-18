/* Vulkan renderer */
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
/* Vulkan also before GLFW */
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include "Timer.h"
#include "Renderpass.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "Model.h"
#include "CoordArrowsModel.h"
#include "SplineModel.h"
#include "VkRenderData.h"

class VkRenderer {
  public:
    VkRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    bool draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xPos, double yPos);

    void cleanup();

  private:
    VkRenderData mRenderData{};

    UserInterface mUserInterface{};
    Camera mCamera{};

    CoordArrowsModel mCoordArrowsModel{};
    VkMesh mCoordArrowsMesh{};

    SplineModel mSplineModel{};
    VkMesh mSplineMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<VkMesh> mModelMesh = nullptr;
    std::unique_ptr<VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double mLastTickTime = 0.0;

    void handleMovementKeys();
    int mCameraForward = 0;
    int mCameraStrafe = 0;
    int mCameraUpDown = 0;

    Timer mFrameTimer{};
    Timer mMatrixGenerateTimer{};
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    VkDeviceSize mMinUniformBufferOffsetAlignment = 0;

    VkUploadMatrices mMatrices{};

    bool deviceInit();
    bool getQueue();
    bool createDepthBuffer();
    bool createVBO();
    bool createUBO();
    bool createSwapchain();
    bool createRenderPass();
    bool createPipelineLayout();
    bool createBasicPipeline();
    bool createLinePipeline();
    bool createFramebuffer();
    bool createCommandPool();
    bool createCommandBuffer();
    bool createSyncObjects();
    bool loadTexture();
    bool initUserInterface();

    bool initVma();

    bool recreateSwapchain();
};
