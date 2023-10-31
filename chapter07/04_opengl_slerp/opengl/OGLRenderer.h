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
#include "UserInterface.h"
#include "Camera.h"
#include "Model.h"
#include "CoordArrowsModel.h"
#include "ArrowModel.h"

#include "OGLRenderData.h"

class OGLRenderer {
  public:
    OGLRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
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
    Framebuffer mFramebuffer{};
    Texture mTex{};
    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

    CoordArrowsModel mCoordArrowsModel{};
    OGLMesh mCoordArrowsMesh{};

    ArrowModel mArrowModel{};
    OGLMesh mStartPosArrowMesh{};
    OGLMesh mEndPosArrowMesh{};
    OGLMesh mQuatPosArrowMesh{};

    std::unique_ptr<Model> mModel = nullptr;
    std::unique_ptr<OGLMesh> mModelMesh = nullptr;
    std::unique_ptr<OGLMesh> mAllMeshes = nullptr;
    unsigned int mLineIndexCount = 0;

    glm::quat mQuatModelOrientation[2] = { glm::quat(), glm::quat() };
    glm::quat mQuatModelOrientationConjugate[2] = { glm::quat(), glm::quat() };
    glm::quat mQuatMix = glm::quat();
    glm::quat mQuatMixConjugate = glm::quat();

    glm::vec3 mQuatModelDist = glm::vec3(-2.5f, 0.0f, 0.0f);

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double mLastTickTime = 0.0;

    void handleMovementKeys();

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

};
