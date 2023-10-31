#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Framebuffer.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Shader.h"

#include "OGLRenderData.h"

class OGLRenderer {
  public:
    OGLRenderer(GLFWwindow *window);

    bool init(unsigned int width, unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    void uploadData(OGLMesh vertexData);
    void draw();
    void handleKeyEvents(int key, int scancode, int action, int mods);

    void cleanup();

  private:
    Shader mBasicShader{};
    Shader mChangedShader{};
    Framebuffer mFramebuffer{};
    Texture mTex{};
    VertexBuffer mVertexBuffer{};

    GLFWwindow* mWindow = nullptr;
    int mTriangleCount = 0;
    bool mUseChangedShader = false;
    int mWidth = 0;
    int mHeight = 0;
};
