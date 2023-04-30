/* Vulkan renderer */
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
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
#include "ArrowModel.h"
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

    ArrowModel mArrowModel{};
    VkMesh mStartPosArrowMesh{};
    VkMesh mEndPosArrowMesh{};
    VkMesh mQuatPosArrowMesh{};

    SplineModel mSplineModel{};
    VkMesh mSplineMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<VkMesh> mModelMesh = nullptr;
    std::unique_ptr<VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    glm::quat mQuatModelOrientation[2] = { glm::quat(), glm::quat() };
    glm::quat mQuatModelOrientationConjugate[2] = { glm::quat(), glm::quat() };
    glm::quat mQuatMix = glm::quat();
    glm::quat mQuatMixConjugate = glm::quat();

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

    bool deviceInit(VkRenderData &renderData);
    bool getQueue();
    bool createDepthBuffer(VkRenderData &renderData);
    bool createVBO(VkRenderData &renderData);
    bool createUBO(VkRenderData &renderData);
    bool createSwapchain(VkRenderData &renderData);
    bool createRenderPass(VkRenderData &renderData);
    bool createPipelineLayout(VkRenderData &renderData);
    bool createBasicPipeline(VkRenderData &renderData);
    bool createLinePipeline(VkRenderData &renderData);
    bool createFramebuffer(VkRenderData &renderData);
    bool createCommandPool(VkRenderData &renderData);
    bool createCommandBuffer(VkRenderData &renderData);
    bool createSyncObjects(VkRenderData &renderData);
    bool loadTexture(VkRenderData &renderData);
    bool initUserInterface(VkRenderData &renderData);

    bool initVma();

    bool recreateSwapchain(VkRenderData &renderData);
};
