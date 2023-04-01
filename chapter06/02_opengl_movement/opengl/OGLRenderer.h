#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
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
    void toggleShader();

    void cleanup();

  private:
    OGLRenderData mRenderData{};

    Timer mFrameTimer{};
    Timer mMatrixGenerateTimer{};
    Timer mUploadToUBOTimer{};
    Timer mUIGenerateTimer{};
    Timer mUIDrawTimer{};

    Shader mBasicShader{};
    Shader mChangedShader{};
    Framebuffer mFramebuffer{};
    Texture mTex{};
    VertexBuffer mVertexBuffer{};
    UniformBuffer mUniformBuffer{};
    UserInterface mUserInterface{};
    Camera mCamera{};

    bool mMouseLock = false;
    int mMouseXPos = 0;
    int mMouseYPos = 0;

    double lastTickTime = 0.0;

    void handleMovementKeys();

    /* create identity matrix by default */
    glm::mat4 mViewMatrix = glm::mat4(1.0f);
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

};
