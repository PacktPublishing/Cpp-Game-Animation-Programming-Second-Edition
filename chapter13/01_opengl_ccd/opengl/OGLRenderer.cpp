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

  mVertexBuffer.init();
  Logger::log(1, "%s: vertex buffer successfully created\n", __FUNCTION__);

  size_t uniformMatrixBufferSize = 2 * sizeof(glm::mat4);
  mUniformBuffer.init(uniformMatrixBufferSize);
  Logger::log(1, "%s: matrix uniform buffer (size %i bytes) successfully created\n", __FUNCTION__, uniformMatrixBufferSize);

  if (!mLineShader.loadShaders("shader/line.vert", "shader/line.frag")) {
    Logger::log(1, "%s: line shader loading failed\n", __FUNCTION__);
    return false;
  }
  if (!mGltfGPUShader.loadShaders("shader/gltf_gpu.vert", "shader/gltf_gpu.frag")) {
    Logger::log(1, "%s: gltTF GPU shader loading failed\n", __FUNCTION__);
    return false;
  }
  if (!mGltfGPUDualQuatShader.loadShaders("shader/gltf_gpu_dquat.vert",
      "shader/gltf_gpu_dquat.frag")) {
    Logger::log(1, "%s: glTF GPU dual quat shader loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: shaders succesfully loaded\n", __FUNCTION__);

  mUserInterface.init(mRenderData);
  Logger::log(1, "%s: user interface initialized\n", __FUNCTION__);

  /* add backface culling and depth test already here */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0);

  mGltfModel = std::make_shared<GltfModel>();
  std::string modelFilename = "assets/Woman.gltf";
  std::string modelTexFilename = "textures/Woman.png";
  if (!mGltfModel->loadModel(mRenderData, modelFilename, modelTexFilename)) {
    Logger::log(1, "%s: loading glTF model '%s' failed\n", __FUNCTION__, modelFilename.c_str());
    return false;
  }
  mGltfModel->uploadIndexBuffer();
  Logger::log(1, "%s: glTF model '%s' succesfully loaded\n", __FUNCTION__, modelFilename.c_str());

  size_t modelJointMatrixBufferSize = mGltfModel->getJointMatrixSize() * sizeof(glm::mat4);
  mGltfShaderStorageBuffer.init(modelJointMatrixBufferSize);
  Logger::log(1, "%s: glTF joint matrix shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointMatrixBufferSize);

  size_t modelJointDualQuatBufferSize = mGltfModel->getJointDualQuatsSize() *
    sizeof(glm::mat2x4);
  mGltfDualQuatSSBuffer.init(modelJointDualQuatBufferSize);
  Logger::log(1, "%s: glTF joint dual quaternions shader storage buffer (size %i bytes) successfully created\n", __FUNCTION__, modelJointDualQuatBufferSize);

  /* valid, but emtpy */
  mLineMesh = std::make_shared<OGLMesh>();
  Logger::log(1, "%s: line mesh storage initialized\n", __FUNCTION__);

  /* reset skeleton split */
  mRenderData.rdSkelSplitNode = mRenderData.rdModelNodeCount - 1;

  /* set values for inverse kinematics */
  /* hard-code left arm here for startup */
  mRenderData.rdIkEffectorNode = 19;
  mRenderData.rdIkRootNode = 26;
  mGltfModel->setInverseKinematicsNodes(mRenderData.rdIkEffectorNode,
    mRenderData.rdIkRootNode);
  mGltfModel->setNumIKIterations(mRenderData.rdIkIterations);

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

  /* check values and reset model nodes if required */
  static blendMode lastBlendMode = mRenderData.rdBlendingMode;
  if (lastBlendMode != mRenderData.rdBlendingMode) {
    lastBlendMode = mRenderData.rdBlendingMode;
    if (mRenderData.rdBlendingMode != blendMode::additive) {
      mRenderData.rdSkelSplitNode = mRenderData.rdModelNodeCount - 1;
    }
    mGltfModel->resetNodeData();
  }

  static int skelSplitNode = mRenderData.rdSkelSplitNode;
  if (skelSplitNode != mRenderData.rdSkelSplitNode) {
    mGltfModel->setSkeletonSplitNode(mRenderData.rdSkelSplitNode);
    skelSplitNode = mRenderData.rdSkelSplitNode;
    mGltfModel->resetNodeData();
  }

  static ikMode lastIkMode = mRenderData.rdIkMode;
  if (lastIkMode != mRenderData.rdIkMode) {
    mGltfModel->resetNodeData();
    lastIkMode = mRenderData.rdIkMode;
  }

  static int numIKIterations = mRenderData.rdIkIterations;
  if (numIKIterations != mRenderData.rdIkIterations) {
    mGltfModel->setNumIKIterations(mRenderData.rdIkIterations);
    mGltfModel->resetNodeData();
    numIKIterations = mRenderData.rdIkIterations;
  }

  static int ikEffectorNode = mRenderData.rdIkEffectorNode;
  static int ikRootNode = mRenderData.rdIkRootNode;
  if (ikEffectorNode != mRenderData.rdIkEffectorNode ||
      ikRootNode != mRenderData.rdIkRootNode) {
    mGltfModel->setInverseKinematicsNodes(mRenderData.rdIkEffectorNode,
      mRenderData.rdIkRootNode);
    mGltfModel->resetNodeData();
    ikEffectorNode = mRenderData.rdIkEffectorNode;
    ikRootNode = mRenderData.rdIkRootNode;
  }

  /* animate */
  if (mRenderData.rdPlayAnimation) {
    if (mRenderData.rdBlendingMode == blendMode::crossfade ||
        mRenderData.rdBlendingMode == blendMode::additive) {
      mGltfModel->playAnimation(mRenderData.rdAnimClip,
        mRenderData.rdCrossBlendDestAnimClip, mRenderData.rdAnimSpeed,
        mRenderData.rdAnimCrossBlendFactor,
        mRenderData.rdAnimationPlayDirection);
    } else {
      mGltfModel->playAnimation(mRenderData.rdAnimClip, mRenderData.rdAnimSpeed,
        mRenderData.rdAnimBlendFactor,
        mRenderData.rdAnimationPlayDirection);
    }
  } else {
    mRenderData.rdAnimEndTime = mGltfModel->getAnimationEndTime(mRenderData.rdAnimClip);
    if (mRenderData.rdBlendingMode == blendMode::crossfade ||
        mRenderData.rdBlendingMode == blendMode::additive) {
      mGltfModel->crossBlendAnimationFrame(mRenderData.rdAnimClip,
        mRenderData.rdCrossBlendDestAnimClip, mRenderData.rdAnimTimePosition,
        mRenderData.rdAnimCrossBlendFactor);
    } else {
      mGltfModel->blendAnimationFrame(mRenderData.rdAnimClip, mRenderData.rdAnimTimePosition,
        mRenderData.rdAnimBlendFactor);
    }
  }

  /* solve IK */
  if (mRenderData.rdIkMode == ikMode::ccd) {
    mGltfModel->solveIKByCCD(mRenderData.rdIkTargetPos);
  }

  mLineMesh->vertices.clear();

  /* get gltTF skeleton */
  mSkeletonLineIndexCount = 0;
  if (mRenderData.rdDrawSkeleton) {
    std::shared_ptr<OGLMesh> mesh = mGltfModel->getSkeleton();
    mSkeletonLineIndexCount += mesh->vertices.size();
    mLineMesh->vertices.insert(mLineMesh->vertices.begin(),
      mesh->vertices.begin(), mesh->vertices.end());
  }

  /* draw coordiante arrows on target position */
  mCoordArrowsLineIndexCount = 0;
  if (mRenderData.rdIkMode == ikMode::ccd) {
    mCoordArrowsMesh = mCoordArrowsModel.getVertexData();
    mCoordArrowsLineIndexCount = mCoordArrowsMesh.vertices.size();
    std::for_each(mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end(),
      [=](auto &n){
        n.color /= 2.0f;
        n.position += mRenderData.rdIkTargetPos;
    });

    mLineMesh->vertices.insert(mLineMesh->vertices.end(),
      mCoordArrowsMesh.vertices.begin(), mCoordArrowsMesh.vertices.end());
  }

  mRenderData.rdMatrixGenerateTime = mMatrixGenerateTimer.stop();

  mUploadToUBOTimer.start();
  std::vector<glm::mat4> matrixData;
  matrixData.push_back(mViewMatrix);
  matrixData.push_back(mProjectionMatrix);
  mUniformBuffer.uploadUboData(matrixData, 0);

  if (mRenderData.rdGPUDualQuatVertexSkinning == skinningMode::dualQuat) {
    mGltfDualQuatSSBuffer.uploadSsboData(mGltfModel->getJointDualQuats(), 2);
  } else {
    mGltfShaderStorageBuffer.uploadSsboData(mGltfModel->getJointMatrices(), 1);
  }
  mRenderData.rdUploadToUBOTime = mUploadToUBOTimer.stop();

  /* upload vertex data */
  mUploadToVBOTimer.start();

  if (mRenderData.rdDrawSkeleton ||
      mRenderData.rdIkMode == ikMode::ccd) {
    uploadData(*mLineMesh);
  }

  if (mModelUploadRequired) {
    mGltfModel->uploadVertexBuffers();
    mModelUploadRequired = false;
  }

  mRenderData.rdUploadToVBOTime = mUploadToVBOTimer.stop();

  /* draw the glTF model */
  if (mRenderData.rdDrawGltfModel) {
    if (mRenderData.rdGPUDualQuatVertexSkinning == skinningMode::dualQuat) {
      mGltfGPUDualQuatShader.use();
    } else {
      mGltfGPUShader.use();
    }
    mGltfModel->draw();
  }

  /* draw the coordinate arrow WITH depth buffer */
  if (mCoordArrowsLineIndexCount > 0) {
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, mSkeletonLineIndexCount, mCoordArrowsLineIndexCount);
  }

  /* draw the skeleton, disable depth test to overlay */
  if (mSkeletonLineIndexCount > 0) {
    glDisable(GL_DEPTH_TEST);
    mLineShader.use();
    mVertexBuffer.bindAndDraw(GL_LINES, 0, mSkeletonLineIndexCount);
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

  mGltfGPUDualQuatShader.cleanup();
  mGltfGPUShader.cleanup();
  mUserInterface.cleanup();
  mLineShader.cleanup();
  mVertexBuffer.cleanup();
  mGltfShaderStorageBuffer.cleanup();
  mGltfDualQuatSSBuffer.cleanup();
  mUniformBuffer.cleanup();
  mFramebuffer.cleanup();
}
