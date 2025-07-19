#include <algorithm>

#include <imgui_impl_glfw.h>

#include <glm/gtc/matrix_transform.hpp>

#include "OGLRenderer.h"
#include "Logger.h"

OGLRenderer::OGLRenderer(GLFWwindow *window) {
  mRenderData.rdWindow = window;
}

bool OGLRenderer::init(unsigned int width, unsigned int height) {
  /* required for perspective */
  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

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

  mUniformBuffer.init();
  Logger::log(1, "%s: uniform buffer successfully created\n", __FUNCTION__);

  if (!mBasicShader.loadShaders("shader/basic.vert", "shader/basic.frag")) {
    Logger::log(1, "%s: bsaic shader loading failed\n", __FUNCTION__);
    return false;
  }
  if (!mLineShader.loadShaders("shader/line.vert", "shader/line.frag")) {
    Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
    return false;
  }
  if (!mGltfShader.loadShaders("shader/gltf.vert", "shader/gltf.frag")) {
    Logger::log(1, "%s: glTF model shader loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: shaders succesfully loaded\n", __FUNCTION__);

  mUserInterface.init(mRenderData);
  Logger::log(1, "%s: user interface initialized\n", __FUNCTION__);

  /* add backface culling and depth test already here */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0);

  /* disable sRGB framebuffer */
  glDisable(GL_FRAMEBUFFER_SRGB);

  mModel = std::make_unique<Model>();

  mEulerModelMesh = std::make_unique<OGLMesh>();
  mQuatModelMesh = std::make_unique<OGLMesh>();
  Logger::log(1, "%s: model mesh storage initialized\n", __FUNCTION__);

  mGltfModel = std::make_shared<GltfModel>();
  std::string modelFilename = "assets/Woman.gltf";
  std::string modelTexFilename = "textures/Woman.png";
  if (!mGltfModel->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
    return false;
  }
  mGltfModel->uploadIndexBuffer();
  Logger::log(1, "%s: glTF model '%s' succesfully loaded\n", __FUNCTION__, modelFilename.c_str());

 /* valid, but emtpy */
  mSkeletonMesh = std::make_shared<OGLMesh>();
  Logger::log(1, "%s: skeleton mesh storage initialized\n", __FUNCTION__);

  mAllMeshes = std::make_unique<OGLMesh>();
  Logger::log(1, "%s: global mesh storage initialized\n", __FUNCTION__);

  mFrameTimer.start();

  return true;
}

void OGLRenderer::setSize(unsigned int width, unsigned int height) {
  /* handle minimize */
  if (width == 0 || height == 0) {
    return;
  }

  mRenderData.rdWidth = width;
  mRenderData.rdHeight = height;

  mFramebuffer.resize(width, height);
  glViewport(0, 0, width, height);

  Logger::log(1, "%s: resized window to %dx%d\n", __FUNCTION__, width, height);
}

void OGLRenderer::uploadData(OGLMesh vertexData) {
  mVertexBuffer.uploadData(vertexData);
}

void OGLRenderer::handleKeyEvents(int key, int scancode, int action, int mods) {
}

void OGLRenderer::handleMouseButtonEvents(int button, int action, int mods) {
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  if (button >= 0 && button < ImGuiMouseButton_COUNT) {
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
  }

  /* hide from application if above ImGui window */
  if (io.WantCaptureMouse) {
    return;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    mMouseLock = !mMouseLock;

    if (mMouseLock) {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      /* enable raw mode if possible */
      if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(mRenderData.rdWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
      }
    } else {
      glfwSetInputMode(mRenderData.rdWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void OGLRenderer::handleMousePositionEvents(double xPos, double yPos) {
  /* forward to ImGui */
  ImGuiIO& io = ImGui::GetIO();
  io.AddMousePosEvent((float)xPos, (float)yPos);

  /* hide from application if above ImGui window */
  if (io.WantCaptureMouse) {
    return;
  }

  /* calculate relative movement from last position */
  int mouseMoveRelX = static_cast<int>(xPos) - mMouseXPos;
  int mouseMoveRelY = static_cast<int>(yPos) - mMouseYPos;

  if (mMouseLock) {
    mRenderData.rdViewAzimuth += mouseMoveRelX / 10.0;
    /* keep between 0 and 360 degree */
    if (mRenderData.rdViewAzimuth < 0.0) {
      mRenderData.rdViewAzimuth += 360.0;
    }
    if (mRenderData.rdViewAzimuth >= 360.0) {
      mRenderData.rdViewAzimuth -= 360.0;
    }

    mRenderData.rdViewElevation -= mouseMoveRelY / 10.0;
    /* keep between -89 and +89 degree */
    if (mRenderData.rdViewElevation > 89.0) {
      mRenderData.rdViewElevation = 89.0;
    }
    if (mRenderData.rdViewElevation < -89.0) {
      mRenderData.rdViewElevation = -89.0;
    }
  }

  /* save old values*/
  mMouseXPos = static_cast<int>(xPos);
  mMouseYPos = static_cast<int>(yPos);
}

void OGLRenderer::handleMovementKeys() {
  mRenderData.rdMoveForward = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_W) == GLFW_PRESS) {
    mRenderData.rdMoveForward += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_S) == GLFW_PRESS) {
    mRenderData.rdMoveForward -= 1;
  }

  mRenderData.rdMoveRight = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_A) == GLFW_PRESS) {
    mRenderData.rdMoveRight -= 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_D) == GLFW_PRESS) {
    mRenderData.rdMoveRight += 1;
  }

  mRenderData.rdMoveUp = 0;
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_E) == GLFW_PRESS) {
    mRenderData.rdMoveUp += 1;
  }
  if (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_Q) == GLFW_PRESS) {
    mRenderData.rdMoveUp -= 1;
  }

  /* speed up movement with shift */
  if ((glfwGetKey(mRenderData.rdWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ||
      (glfwGetKey(mRenderData.rdWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
    mRenderData.rdMoveForward *= 4;
    mRenderData.rdMoveRight *= 4;
    mRenderData.rdMoveUp *= 4;
  }
}

void OGLRenderer::draw() {
  /* handle minimize */
  while (mRenderData.rdWidth == 0 || mRenderData.rdHeight == 0) {
    glfwGetFramebufferSize(mRenderData.rdWindow, &mRenderData.rdWidth, &mRenderData.rdHeight);
    glfwWaitEvents();
  }

  /* get time difference for movement */
  double tickTime = glfwGetTime();
  mRenderData.rdTickDiff = tickTime - mLastTickTime;

  mRenderData.rdFrameTime = mFrameTimer.stop();
  mFrameTimer.start();

  handleMovementKeys();

  mAllMeshes->vertices.clear();

  /* draw to framebuffer */
  mFramebuffer.bind();

  glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
  glClearDepth(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mMatrixGenerateTimer.start();
  mProjectionMatrix = glm::perspective(
    glm::radians(static_cast<float>(mRenderData.rdFieldOfView)),
    static_cast<float>(mRenderData.rdWidth) / static_cast<float>(mRenderData.rdHeight),
    0.01f, 50.0f);

  mViewMatrix = mCamera.getViewMatrix(mRenderData);

  /* glTF vertex skinning */
  mGltfModel->applyVertexSkinning(mRenderData.rdEnableVertexSkinning);
  /* get gltTF skeleton */
  mSkeletonMesh = mGltfModel->getSkeleton(mRenderData.rdEnableVertexSkinning);

  mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

  mUploadToUBOTimer.start();
  mUniformBuffer.uploadUboData(mViewMatrix, mProjectionMatrix);
  mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

  /* reset all values to zero when UI button is pressed */
  if (mRenderData.rdResetAngles) {
    mRenderData.rdResetAngles = false;

    mRenderData.rdRotXAngle = 0;
    mRenderData.rdRotYAngle = 0;
    mRenderData.rdRotZAngle = 0;

    mEulerRotMatrix = glm::mat3(1.0f);
    mQuatModelOrientation = glm::quat();
  }

  /* rotation order Y-Z-X */
  mRotYMat = glm::rotate(glm::mat4(1.0f),
    glm::radians(static_cast<float>(mRenderData.rdRotYAngle)), mRotYAxis);
  mRotZMat = glm::rotate(mRotYMat,
    glm::radians(static_cast<float>(mRenderData.rdRotZAngle)), mRotZAxis);
  mEulerRotMatrix = glm::rotate(mRotZMat,
    glm::radians(static_cast<float>(mRenderData.rdRotXAngle)), mRotXAxis);

  /* create quaternion from angles  */
  mQuatModelOrientation = glm::normalize(glm::quat(glm::vec3(
    glm::radians(static_cast<float>(mRenderData.rdRotXAngle)),
    glm::radians(static_cast<float>(mRenderData.rdRotYAngle)),
    glm::radians(static_cast<float>(mRenderData.rdRotZAngle))
  )));

  /* conjugate = same length, but opposite direction*/
  mQuatModelOrientConjugate = glm::conjugate(mQuatModelOrientation);

  /* draw a static coordinate system */
  mCoordArrowsMesh.vertices.clear();
  if (mRenderData.rdDrawWorldCoordArrows) {
    mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
    std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
      [=](auto &n){
        n.color /= 2.0f;
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());
  }

  mEulerCoordArrowsMesh.vertices.clear();
  mQuatArrowMesh.vertices.clear();
  if (mRenderData.rdDrawModelCoordArrows) {
    /* draw a full coordinate system on the normal rotated model */
    mEulerCoordArrowsMesh = mCoordArrowsModel.getVertexData();
    std::for_each(mEulerCoordArrowsMesh.vertices.begin(), mEulerCoordArrowsMesh.vertices.end(),
      [=](auto &n){
        n.position = mEulerRotMatrix * n.position;
        n.position += mEulerModelDist;
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mEulerCoordArrowsMesh.vertices.begin(), mEulerCoordArrowsMesh.vertices.end());

    /* draw an arrow to show quaternion orientation changes */
    mQuatArrowMesh = mArrowModel.getVertexData();
    std::for_each(mQuatArrowMesh.vertices.begin(), mQuatArrowMesh.vertices.end(),
      [=](auto &n){
        glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
        glm::quat newPosition =
          mQuatModelOrientation * position * mQuatModelOrientConjugate;
        n.position.x = newPosition.x;
        n.position.y = newPosition.y;
        n.position.z = newPosition.z;
        n.position += mQuatModelDist;
    });
    mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
      mQuatArrowMesh.vertices.begin(), mQuatArrowMesh.vertices.end());
  }

  /* add the glTF skeleton lines */
  mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
    mSkeletonMesh->vertices.begin(), mSkeletonMesh->vertices.end());

  /* draw both models */
  *mEulerModelMesh = mModel->getVertexData();
  mRenderData.rdTriangleCount = mEulerModelMesh->vertices.size() / 3;
  std::for_each(mEulerModelMesh->vertices.begin(), mEulerModelMesh->vertices.end(),
    [=](auto &n){
      n.position = mEulerRotMatrix * n.position;
      n.position += mEulerModelDist;
  });
  mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
    mEulerModelMesh->vertices.begin(), mEulerModelMesh->vertices.end());

  *mQuatModelMesh = mModel->getVertexData();
  mRenderData.rdTriangleCount += mQuatModelMesh->vertices.size() / 3;
  std::for_each(mQuatModelMesh->vertices.begin(), mQuatModelMesh->vertices.end(),
    [=](auto &n){
      glm::quat position = glm::quat(0.0f, n.position.x, n.position.y, n.position.z);
      glm::quat newPosition =
        mQuatModelOrientation * position * mQuatModelOrientConjugate;
      n.position.x = newPosition.x;
      n.position.y = newPosition.y;
      n.position.z = newPosition.z;
      n.position  += mQuatModelDist;
  });
  mAllMeshes->vertices.insert(mAllMeshes->vertices.end(),
    mQuatModelMesh->vertices.begin(), mQuatModelMesh->vertices.end());

  /* upload vertex data */
  mUploadToVBOTimer.start();
  /* upload linex and boxes*/
  uploadData(*mAllMeshes);
  /* upload glTF model data */
  mGltfModel->uploadVertexBuffers();
  mGltfModel->uploadPositionBuffer();
  mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

  mLineIndexCount =
    mCoordArrowsMesh.vertices.size() + mEulerCoordArrowsMesh.vertices.size() +
    mQuatArrowMesh.vertices.size();
  mSkeletonLineIndexCount = mSkeletonMesh->vertices.size();

  /* draw the lines first */
  if (mLineIndexCount > 0) {
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, 0, mLineIndexCount);
  }

  /* draw the box models */
  mBasicShader.use();
  mTex.bind();
  mVertexBuffer.bindAndDraw(GL_TRIANGLES, mLineIndexCount + mSkeletonLineIndexCount, mRenderData.rdTriangleCount * 3);
  mTex.unbind();

  /* draw the glTF model */
  if (mRenderData.rdDrawGltfModel) {
    mGltfShader.use();
    mGltfModel->draw();
  }

  /* draw the skeleton last, disable depth test to overlay */
  if (mSkeletonLineIndexCount > 0 && mRenderData.rdDrawSkeleton) {
    glDisable(GL_DEPTH_TEST);
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, mLineIndexCount, mSkeletonLineIndexCount);
    glEnable(GL_DEPTH_TEST);
  }

  mFramebuffer.unbind();

  /* blit color buffer to screen */
  mFramebuffer.drawToScreen();

  mUIGenerateTimer.start();
  mUserInterface.createFrame(mRenderData);
  mRenderData.rdUIGenerateTime = mUIGenerateTimer.stop();

  mUIDrawTimer.start();
  mUserInterface.render();
  mRenderData.rdUIDrawTime = mUIDrawTimer.stop();

  mLastTickTime = tickTime;
}

void OGLRenderer::cleanup() {
  mGltfModel->cleanup();
  mGltfModel.reset();

  mGltfShader.cleanup();
  mUserInterface.cleanup();
  mLineShader.cleanup();
  mBasicShader.cleanup();
  mTex.cleanup();
  mVertexBuffer.cleanup();
  mUniformBuffer.cleanup();
  mFramebuffer.cleanup();
}
