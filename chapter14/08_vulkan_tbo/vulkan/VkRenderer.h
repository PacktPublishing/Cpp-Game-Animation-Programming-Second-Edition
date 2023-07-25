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
#include "GltfSkeletonPipeline.h"
#include "GltfGPUPipeline.h"
#include "PipelineLayout.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "SyncObjects.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "TexelBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "CoordArrowsModel.h"
#include "GltfModel.h"
#include "GltfInstance.h"

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

    std::shared_ptr<GltfModel> mGltfModel = nullptr;
    bool mModelUploadRequired = true;

    std::vector<std::shared_ptr<GltfInstance>> mGltfInstances{};

    std::vector<glm::mat4> mModelJointMatrices{};
    std::vector<glm::mat2x4> mModelJointDualQuats{};

    CoordArrowsModel mCoordArrowsModel{};
    VkMesh mCoordArrowsMesh{};
    std::shared_ptr<VkMesh> mLineMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;
    unsigned int mCoordArrowsLineIndexCount = 0;

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

    std::vector<glm::mat4> mPerspViewMatrices{};

    bool deviceInit(VkRenderData &renderData);
    bool getQueue();
    bool createDepthBuffer(VkRenderData &renderData);
    bool createVBO(VkRenderData &renderData);
    bool createUBO(VkRenderData &renderData, VkUniformBufferData &UBOData,
      std::vector<glm::mat4> matricesToUpload);
    bool createMatrixTBO(VkRenderData &renderData, VkTexelBufferData &TBOData);
    bool createDQSSBO(VkRenderData &renderData, VkShaderStorageBufferData &SSBOData);
    bool createSwapchain(VkRenderData &renderData);
    bool createRenderPass(VkRenderData &renderData);
    bool createGltfPipelineLayout(VkRenderData& renderData, VkTextureData textureData);
    bool createLinePipeline(VkRenderData& renderData);
    bool createGltfSkeletonPipeline(VkRenderData& renderData);
    bool createGltfGPUPipeline(VkRenderData& renderData);
    bool createGltfGPUDQPipeline(VkRenderData& renderData);
    bool createFramebuffer(VkRenderData &renderData);
    bool createCommandPool(VkRenderData &renderData);
    bool createCommandBuffer(VkRenderData &renderData);
    bool createSyncObjects(VkRenderData &renderData);
    bool loadTexture(VkRenderData &renderData, VkTextureData &textureData);
    bool initUserInterface(VkRenderData &renderData);
    bool loadGltfModel(VkRenderData &renderData);
    bool createInstances(VkRenderData &renderData);

    bool initVma();

    bool recreateSwapchain(VkRenderData &renderData);
};
