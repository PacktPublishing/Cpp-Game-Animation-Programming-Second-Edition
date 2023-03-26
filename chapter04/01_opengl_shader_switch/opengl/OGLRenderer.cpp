#include <glm/gtc/matrix_transform.hpp>

#include "OGLRenderer.h"
#include "Logger.h"

OGLRenderer::OGLRenderer(GLFWwindow *window) {
  mWindow = window;
}

bool OGLRenderer::init(unsigned int width, unsigned int height) {
  /* required for perspective */
  mWidth = width;
  mHeight = height;

  /* initalize GLAD */
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Logger::log(1, "%s error: failed to initialize GLAD\n", __FUNCTION__);
    return false;
  }

  if (!GLAD_GL_VERSION_4_6) {
    Logger::log(1, "%s error: failed to get at least OpenGL 4.6\n", __FUNCTION__);
    return false;
  }

  GLint majorVersion, minorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  Logger::log(1, "%s: OpenGL %d.%d initializeed\n", __FUNCTION__, majorVersion, minorVersion);

  if (!mFramebuffer.init(width, height)) {
    Logger::log(1, "%s error: could not init Framebuffer\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: framebuffer succesfully initialized\n", __FUNCTION__);

  if (!mTex.loadTexture("textures/crate.png")) {
    Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: texture successfully loaded\n", __FUNCTION__);

  mVertexBuffer.init();
  Logger::log(1, "%s: vertex buffer successfully created\n", __FUNCTION__);

  if (!mBasicShader.loadShaders("shader/basic.vert", "shader/basic.frag")) {
    Logger::log(1, "%s: shader loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: shaders succesfully loaded\n", __FUNCTION__);

  if (!mChangedShader.loadShaders("shader/changed.vert", "shader/changed.frag")) {
    Logger::log(1, "%s: shader loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: shaders succesfully loaded\n", __FUNCTION__);

  /* add backface culling and depth test already here */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  return true;
}


void OGLRenderer::setSize(unsigned int width, unsigned int height) {
  /* handle minimize */
  if (width == 0 || height == 0) {
    return;
  }

  mWidth = width;
  mHeight = height;

  mFramebuffer.resize(width, height);
  glViewport(0, 0, width, height);

  Logger::log(1, "%s: resized window to %dx%d\n", __FUNCTION__, width, height);
}

void OGLRenderer::uploadData(OGLMesh vertexData) {
  mTriangleCount = vertexData.vertices.size();
  mVertexBuffer.uploadData(vertexData);
}

void OGLRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
    mUseChangedShader = !mUseChangedShader;
  }
}

void OGLRenderer::draw() {
  /* handle minimize */
  while (mWidth == 0 || mHeight == 0) {
    glfwGetFramebufferSize(mWindow, &mWidth, &mHeight);
    glfwWaitEvents();
  }

  /* draw to framebuffer */
  mFramebuffer.bind();

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (mUseChangedShader) {
    mChangedShader.use();
  } else {
    mBasicShader.use();
  }
  mTex.bind();
  mVertexBuffer.bind();

  mVertexBuffer.draw(GL_TRIANGLES, 0, mTriangleCount);
  mVertexBuffer.unbind();
  mTex.unbind();

  mFramebuffer.unbind();

  /* blit color buffer to screen */
  mFramebuffer.drawToScreen();
}

void OGLRenderer::cleanup() {
  mBasicShader.cleanup();
  mChangedShader.cleanup();
  mTex.cleanup();
  mVertexBuffer.cleanup();
  mFramebuffer.cleanup();
}
