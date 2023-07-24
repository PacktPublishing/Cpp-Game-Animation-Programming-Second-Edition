#pragma once
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Timer.h"
#include "Framebuffer.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "UserInterface.h"
#include "Camera.h"
#include "CoordArrowsModel.h"
#include "GltfModel.h"
#include "GltfInstance.h"

#include "OGLRenderData.h"
#include "ModelSettings.h"

class OGLRenderer {
  public:
    OGLRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    void uploadData(OGLMesh vertexData);
    void draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xPos, double yPos);

    void cleanup();

  private:
    OGLRenderData mRenderData{};

    Timer mFrameTimer{};
    Timer mMatrixGenerateTimer{};
    Timer mUploadToVBOTimer{};
    Timer mUploadToUBOTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};

    Shader mLineShader{};
    Shader mGltfGPUShader{};
    Shader mGltfGPUDualQuatShader{};

    Framebuffer mFramebuffer{};
    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    ShaderStorageBuffer mGltfShaderStorageBuffer{};
    ShaderStorageBuffer mGltfDualQuatSSBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

    std::vector<std::shared_ptr<GltfModel>> mGltfModels{};

    std::vector<std::shared_ptr<GltfInstance>> mGltfInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfMatrixInstances{};
    std::vector<std::shared_ptr<GltfInstance>> mGltfDQInstances{};

    std::vector<glm::mat4> mModelJointMatrices{};
    std::vector<glm::mat2x4> mModelJointDualQuats{};

    CoordArrowsModel mCoordArrowsModel{};
    OGLMesh mCoordArrowsMesh{};
    std::shared_ptr<OGLMesh> mLineMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;
    unsigned int mCoordArrowsLineIndexCount = 0;

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double mLastTickTime = 0.0;

    void handleMovementKeys();

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

};
