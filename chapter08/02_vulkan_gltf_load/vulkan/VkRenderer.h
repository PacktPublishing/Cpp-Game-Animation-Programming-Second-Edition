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
#include "GltfPipeline.h"
#include "PipelineLayout.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "Model.h"
#include "CoordArrowsModel.h"
#include "ArrowModel.h"
#include "GltfModel.h"

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
    VkGltfRenderData mGltfRenderData{};

    UserInterface mUserInterface{};
    Camera mCamera{};

    CoordArrowsModel mCoordArrowsModel{};
    VkMesh mCoordArrowsMesh{};
    VkMesh mEulerCoordArrowsMesh{};

    ArrowModel mArrowModel{};
    VkMesh mQuatArrowMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<VkMesh> mEulerModelMesh = nullptr;
    std::unique_ptr<VkMesh> mQuatModelMesh = nullptr;
    std::unique_ptr<VkMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    std::shared_ptr<GltfModel> mGltfModel = nullptr;

    glm::mat4 mRotYMat = glm::mat4(1.0f);
    glm::mat4 mRotZMat = glm::mat4(1.0f);

    glm::vec3 mEulerModelDist = glm::vec3(-2.5f, 0.0f, 0.0f);
    glm::vec3 mQuatModelDist = glm::vec3(2.5f, 0.0f, 0.0f);

    glm::vec3 mRotXAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 mRotYAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 mRotZAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::mat3 mEulerRotMatrix = glm::mat3(1.0f);
    glm::quat mQuatModelOrientation = glm::quat();
    glm::quat mQuatModelOrientConjugate = glm::quat();

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
    bool createGltfPipelineLayout(VkRenderData& renderData, VkGltfRenderData &gltfRenderData);
    bool createGltfPipeline(VkRenderData& renderData);
    bool createFramebuffer(VkRenderData &renderData);
    bool createCommandPool(VkRenderData &renderData);
    bool createCommandBuffer(VkRenderData &renderData);
    bool createSyncObjects(VkRenderData &renderData);
    bool loadTexture(VkRenderData &renderData, VkTextureData &textureData);
    bool initUserInterface(VkRenderData &renderData);
    bool loadGltfModel(VkRenderData &renderData, VkGltfRenderData &gltfRenderData);

    bool initVma();

    bool recreateSwapchain(VkRenderData &renderData);
};
