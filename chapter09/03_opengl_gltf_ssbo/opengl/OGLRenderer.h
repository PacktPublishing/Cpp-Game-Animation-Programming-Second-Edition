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
#include "Model.h"
#include "CoordArrowsModel.h"
#include "ArrowModel.h"
#include "GltfModel.h"

#include "OGLRenderData.h"

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

    Shader mBasicShader{};
    Shader mLineShader{};
    Shader mGltfShader{};
    Shader mGltfGPUShader{};

    Framebuffer mFramebuffer{};
    Texture mTex{};
    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    ShaderStorageBuffer mGltfUniformBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

    CoordArrowsModel mCoordArrowsModel{};
    OGLMesh mCoordArrowsMesh{};
    OGLMesh mEulerCoordArrowsMesh{};

    ArrowModel mArrowModel{};
    OGLMesh mQuatArrowMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<OGLMesh> mEulerModelMesh = nullptr;
    std::unique_ptr<OGLMesh> mQuatModelMesh = nullptr;
    std::unique_ptr<OGLMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    std::shared_ptr<OGLMesh> mSkeletonMesh = nullptr;
    unsigned int mSkeletonLineIndexCount = 0;

    std::shared_ptr<GltfModel> mGltfModel = nullptr;;
    bool mModelUploadRequired = true;

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

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

};
