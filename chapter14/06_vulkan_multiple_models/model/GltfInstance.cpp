#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <cstdlib> // rand

#include "GltfInstance.h"
#include "Logger.h"

GltfInstance::~GltfInstance() {
  Logger::log(2, "%s: model instance for '%s' removed\n", __FUNCTION__,
    mGltfModel->getModelFilename().c_str());
}

GltfInstance::GltfInstance(std::shared_ptr<GltfModel> model, glm::vec2 worldPos, bool randomize) {
  if (!model) {
    Logger::log(1, "%s error: invalid glTF model\n", __FUNCTION__);
    return;
  }

  Logger::log(2, "%s: spawning model from glTF file '%s' on position %s\n", __FUNCTION__,
    model->getModelFilename().c_str(), glm::to_string(mModelSettings.msWorldPosition).c_str());

  mGltfModel = model;
  mModelSettings.msWorldPosition = worldPos;
  mNodeCount = mGltfModel->getNodeCount();

  mInverseBindMatrices = mGltfModel->getInverseBindMatrices();
  mNodeToJoint = mGltfModel->getNodeToJoint();

  mJointMatrices.resize(mInverseBindMatrices.size());
  mJointDualQuats.resize(mInverseBindMatrices.size());

  mAdditiveAnimationMask.resize(mNodeCount);
  mInvertedAdditiveAnimationMask.resize(mNodeCount);

  std::fill(mAdditiveAnimationMask.begin(), mAdditiveAnimationMask.end(), true);
  mInvertedAdditiveAnimationMask = mAdditiveAnimationMask;
  mInvertedAdditiveAnimationMask.flip();

  GltfNodeData nodeData;
  nodeData = mGltfModel->getGltfNodes();
  mRootNode = nodeData.rootNode;
  mRootNode->setWorldPosition(glm::vec3(mModelSettings.msWorldPosition.x, 0.0f,
    mModelSettings.msWorldPosition.y));

  mNodeList = nodeData.nodeList;

  /* reset skeleton split */
  mModelSettings.msSkelSplitNode = mNodeCount - 1;

  for (const auto &node : mNodeList) {
    if (node) {
      mModelSettings.msSkelNodeNames.push_back(node->getNodeName());
    } else {
      mModelSettings.msSkelNodeNames.push_back("(invalid)");
    }
  }

  updateNodeMatrices(mRootNode);

  // mRootNode->printTree();

  mAnimClips = mGltfModel->getAnimClips();
  for (const auto &clip : mAnimClips) {
    mModelSettings.msClipNames.push_back(clip->getClipName());
  }
  unsigned int animClipSize = mAnimClips.size();

  /* randomize some settings */
  if (randomize) {
    int animClip = std::rand() % animClipSize;
    float animClipSpeed = (std::rand() % 100) / 100.0f + 0.5f;
    float initRotation = std::rand() % 360 - 180;

    mModelSettings.msAnimClip = animClip;
    mModelSettings.msAnimSpeed = animClipSpeed;
    mModelSettings.msWorldRotation = glm::vec3(0.0f, initRotation, 0.0f);
    mRootNode->setWorldRotation(mModelSettings.msWorldRotation);
  }

  /* update initial clips etc */
  checkForUpdates();

  /* get Skeleton data */
  mSkeletonMesh = std::make_shared<VkMesh>();
  mSkeletonMesh->vertices.resize(mNodeCount * 2);

  /* set values for inverse kinematics */
  /* hard-code zero here as we have different models */
  mModelSettings.msIkEffectorNode = 0;
  mModelSettings.msIkRootNode = 0;
  setInverseKinematicsNodes(mModelSettings.msIkEffectorNode, mModelSettings.msIkRootNode);
  setNumIKIterations(mModelSettings.msIkIterations);

  mModelSettings.msIkTargetWorldPos = getWorldRotation() *
    mModelSettings.msIkTargetPos + glm::vec3(worldPos.x, 0.0f, worldPos.y);
}

void GltfInstance::resetNodeData() {
  mGltfModel->resetNodeData(mRootNode);
  updateNodeMatrices(mRootNode);
}

std::shared_ptr<VkMesh> GltfInstance::getSkeleton() {
  mSkeletonMesh->vertices.clear();

  /* start from Armature child */
  getSkeletonPerNode(mRootNode->getChilds().at(0));
  return mSkeletonMesh;
}

void GltfInstance::getSkeletonPerNode(std::shared_ptr<GltfNode> treeNode) {
  glm::vec3 parentPos = glm::vec3(0.0f);
  parentPos = glm::vec3(treeNode->getNodeMatrix()[3]);

  VkVertex parentVertex;
  parentVertex.position = parentPos;
  parentVertex.color = glm::vec3(0.0f, 1.0f, 1.0f);

  for (const auto &childNode : treeNode->getChilds()) {
    glm::vec3 childPos = glm::vec3(0.0f);
    childPos = glm::vec3(childNode->getNodeMatrix()[3]);

    VkVertex childVertex;
    childVertex.position = childPos;
    childVertex.color = glm::vec3(0.0f, 0.0f, 1.0f);
    mSkeletonMesh->vertices.emplace_back(parentVertex);
    mSkeletonMesh->vertices.emplace_back(childVertex);

    getSkeletonPerNode(childNode);
  }
}

void GltfInstance::updateNodeMatrices(std::shared_ptr<GltfNode> treeNode) {
  treeNode->calculateNodeMatrix();

  if (mModelSettings.msVertexSkinningMode == skinningMode::linear) {
    updateJointMatrices(treeNode);
  } else {
    updateJointDualQuats(treeNode);
  }

  for (auto& childNode : treeNode->getChilds()) {
    updateNodeMatrices(childNode);
  }
}

void GltfInstance::updateJointMatrices(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();
  mJointMatrices.at(mNodeToJoint.at(nodeNum)) =
    treeNode->getNodeMatrix() * mInverseBindMatrices.at(mNodeToJoint.at(nodeNum));
}

void GltfInstance::updateJointDualQuats(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();

  glm::quat orientation;
  glm::vec3 scale;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::dualquat dq;

  /* extract components from updated node matrix and create dual quaternion */
  glm::mat4 nodeJointMat = treeNode->getNodeMatrix() *
    mInverseBindMatrices.at(mNodeToJoint.at(nodeNum));
  if (glm::decompose(nodeJointMat, scale, orientation, translation, skew, perspective)) {
    dq[0] = orientation;
    dq[1] = glm::quat(0.0, translation.x, translation.y, translation.z) * orientation * 0.5f;
    mJointDualQuats.at(mNodeToJoint.at(nodeNum)) = glm::mat2x4_cast(dq);
  } else {
    Logger::log(1, "%s error: could not decompose matrix for node %i\n", __FUNCTION__,
      nodeNum);
  }
}

int GltfInstance::getJointMatrixSize() {
  return mJointMatrices.size();
}

std::vector<glm::mat4> GltfInstance::getJointMatrices() {
  return mJointMatrices;
}

int GltfInstance::getJointDualQuatsSize() {
  return mJointDualQuats.size();
}

std::vector<glm::mat2x4> GltfInstance::getJointDualQuats() {
  return mJointDualQuats;
}

void GltfInstance::checkForUpdates() {
  static blendMode lastBlendMode = mModelSettings.msBlendingMode;
  static int skelSplitNode = mModelSettings.msSkelSplitNode;
  static glm::vec2 worldPos = mModelSettings.msWorldPosition;
  static glm::vec3 worldRot = mModelSettings.msWorldRotation;
  static glm::vec3 ikTargetPos = mModelSettings.msIkTargetPos;
  static ikMode lastIkMode = mModelSettings.msIkMode;
  static int numIKIterations = mModelSettings.msIkIterations;
  static int ikEffectorNode = mModelSettings.msIkEffectorNode;
  static int ikRootNode = mModelSettings.msIkRootNode;

  if (skelSplitNode != mModelSettings.msSkelSplitNode) {
    setSkeletonSplitNode(mModelSettings.msSkelSplitNode);
    skelSplitNode = mModelSettings.msSkelSplitNode;
    resetNodeData();
  }

  if (lastBlendMode != mModelSettings.msBlendingMode) {
    lastBlendMode = mModelSettings.msBlendingMode;
    if (mModelSettings.msBlendingMode != blendMode::additive) {
      mModelSettings.msSkelSplitNode = mNodeCount - 1;
    }
    resetNodeData();
  }

  if (worldPos != mModelSettings.msWorldPosition) {
    mRootNode->setWorldPosition(glm::vec3(mModelSettings.msWorldPosition.x, 0.0f,
      mModelSettings.msWorldPosition.y));
    worldPos = mModelSettings.msWorldPosition;
    mModelSettings.msIkTargetWorldPos = getWorldRotation() *
      mModelSettings.msIkTargetPos + glm::vec3(worldPos.x, 0.0f, worldPos.y);
  }

  if (worldRot != mModelSettings.msWorldRotation) {
    mRootNode->setWorldRotation(mModelSettings.msWorldRotation);
    worldRot = mModelSettings.msWorldRotation;
    mModelSettings.msIkTargetWorldPos = getWorldRotation() *
      mModelSettings.msIkTargetPos + glm::vec3(worldPos.x, 0.0f, worldPos.y);
  }

  if (ikTargetPos != mModelSettings.msIkTargetPos) {
    ikTargetPos = mModelSettings.msIkTargetPos;
    mModelSettings.msIkTargetWorldPos = getWorldRotation() *
      mModelSettings.msIkTargetPos + glm::vec3(worldPos.x, 0.0f, worldPos.y);
  }

  if (lastIkMode != mModelSettings.msIkMode) {
    resetNodeData();
    lastIkMode = mModelSettings.msIkMode;
  }

  if (numIKIterations != mModelSettings.msIkIterations) {
    setNumIKIterations(mModelSettings.msIkIterations);
    resetNodeData();
    numIKIterations = mModelSettings.msIkIterations;
  }

  if (ikEffectorNode != mModelSettings.msIkEffectorNode ||
      ikRootNode != mModelSettings.msIkRootNode) {
    setInverseKinematicsNodes(mModelSettings.msIkEffectorNode, mModelSettings.msIkRootNode);
    resetNodeData();
    ikEffectorNode = mModelSettings.msIkEffectorNode;
    ikRootNode = mModelSettings.msIkRootNode;
  }
}

void GltfInstance::updateAnimation() {
  if (mModelSettings.msPlayAnimation) {
    if (mModelSettings.msBlendingMode == blendMode::crossfade ||
        mModelSettings.msBlendingMode == blendMode::additive) {
      playAnimation(mModelSettings.msAnimClip,
        mModelSettings.msCrossBlendDestAnimClip, mModelSettings.msAnimSpeed,
        mModelSettings.msAnimCrossBlendFactor,
        mModelSettings.msAnimationPlayDirection);
    } else {
      playAnimation(mModelSettings.msAnimClip, mModelSettings.msAnimSpeed,
        mModelSettings.msAnimBlendFactor,
        mModelSettings.msAnimationPlayDirection);
    }
  } else {
    mModelSettings.msAnimEndTime = getAnimationEndTime(mModelSettings.msAnimClip);
    if (mModelSettings.msBlendingMode == blendMode::crossfade ||
        mModelSettings.msBlendingMode == blendMode::additive) {
      crossBlendAnimationFrame(mModelSettings.msAnimClip,
        mModelSettings.msCrossBlendDestAnimClip, mModelSettings.msAnimTimePosition,
        mModelSettings.msAnimCrossBlendFactor);
    } else {
      blendAnimationFrame(mModelSettings.msAnimClip, mModelSettings.msAnimTimePosition,
        mModelSettings.msAnimBlendFactor);
    }
  }
}

void GltfInstance::solveIK() {
  switch (mModelSettings.msIkMode) {
    case ikMode::ccd:
      solveIKByCCD(mModelSettings.msIkTargetWorldPos);
      break;
    case ikMode::fabrik:
      solveIKByFABRIK(mModelSettings.msIkTargetWorldPos);
      break;
    default:
      /* do nothing */
      break;
  }
}

void GltfInstance::playAnimation(int animNum, float speedDivider, float blendFactor,
    replayDirection direction) {
  double currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  if (direction == replayDirection::backward) {
    blendAnimationFrame(animNum, mAnimClips.at(animNum)->getClipEndTime() -
      std::fmod(currentTime / 1000.0 * speedDivider,
      mAnimClips.at(animNum)->getClipEndTime()), blendFactor);
  } else {
    blendAnimationFrame(animNum, std::fmod(currentTime / 1000.0 * speedDivider,
      mAnimClips.at(animNum)->getClipEndTime()), blendFactor);
  }
}

void GltfInstance::playAnimation(int sourceAnimNumber, int destAnimNumber,
    float speedDivider, float blendFactor, replayDirection direction) {
  double currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

  if (direction == replayDirection::backward) {
    crossBlendAnimationFrame(sourceAnimNumber, destAnimNumber,
      mAnimClips.at(sourceAnimNumber)->getClipEndTime() -
      std::fmod(currentTime / 1000.0 * speedDivider,
      mAnimClips.at(sourceAnimNumber)->getClipEndTime()), blendFactor);
  } else {
    crossBlendAnimationFrame(sourceAnimNumber, destAnimNumber,
      std::fmod(currentTime / 1000.0 * speedDivider,
      mAnimClips.at(sourceAnimNumber)->getClipEndTime()), blendFactor);
  }
}

void GltfInstance::blendAnimationFrame(int animNum, float time, float blendFactor) {
  mAnimClips.at(animNum)->blendAnimationFrame(mNodeList, mAdditiveAnimationMask, time,
    blendFactor);
  updateNodeMatrices(mRootNode);
}

void GltfInstance::crossBlendAnimationFrame(int sourceAnimNumber, int destAnimNumber,
    float time, float blendFactor) {

  float sourceAnimDuration = mAnimClips.at(sourceAnimNumber)->getClipEndTime();
  float destAnimDuration = mAnimClips.at(destAnimNumber)->getClipEndTime();

  float scaledTime = time * (destAnimDuration / sourceAnimDuration);

  mAnimClips.at(sourceAnimNumber)->setAnimationFrame(mNodeList, mAdditiveAnimationMask, time);
  mAnimClips.at(destAnimNumber)->blendAnimationFrame(mNodeList, mAdditiveAnimationMask,
    scaledTime, blendFactor);

  mAnimClips.at(destAnimNumber)->setAnimationFrame(mNodeList, mInvertedAdditiveAnimationMask,
    scaledTime);
  mAnimClips.at(sourceAnimNumber)->blendAnimationFrame(mNodeList,
    mInvertedAdditiveAnimationMask, time, blendFactor);

  updateNodeMatrices(mRootNode);
}

void GltfInstance::updateAdditiveMask(std::shared_ptr<GltfNode> treeNode, int splitNodeNum) {
  /* break chain here */
  if (treeNode->getNodeNum() == splitNodeNum) {
    return;
  }

  mAdditiveAnimationMask.at(treeNode->getNodeNum()) = false;
  for (auto& childNode : treeNode->getChilds()) {
    updateAdditiveMask(childNode, splitNodeNum);
  }
}

void GltfInstance::setSkeletonSplitNode(int nodeNum) {
  std::fill(mAdditiveAnimationMask.begin(), mAdditiveAnimationMask.end(), true);
  updateAdditiveMask(mRootNode, nodeNum);

  mInvertedAdditiveAnimationMask = mAdditiveAnimationMask;
  mInvertedAdditiveAnimationMask.flip();
}

void GltfInstance::setInstanceSettings(ModelSettings settings) {
  mModelSettings = settings;
}

ModelSettings GltfInstance::getInstanceSettings() {
  return mModelSettings;
}

glm::vec2 GltfInstance::getWorldPosition() {
  return mModelSettings.msWorldPosition;
}

glm::quat GltfInstance::getWorldRotation() {
  return glm::normalize(glm::quat(glm::vec3(
    glm::radians(mModelSettings.msWorldRotation.x),
    glm::radians(mModelSettings.msWorldRotation.y),
    glm::radians(mModelSettings.msWorldRotation.z)
  )));
}

float GltfInstance::getAnimationEndTime(int animNum) {
  return mAnimClips.at(animNum)->getClipEndTime();
}

std::shared_ptr<GltfModel> GltfInstance::getModel() {
  return mGltfModel;
}

void GltfInstance::setInverseKinematicsNodes(int effectorNodeNum, int ikChainRootNodeNum) {
  if (effectorNodeNum < 0 || effectorNodeNum > (mNodeList.size() - 1)) {
    Logger::log(1, "%s error: effector node %i is out of range\n", __FUNCTION__,
      effectorNodeNum);
    return;
  }

  if (ikChainRootNodeNum < 0 || ikChainRootNodeNum > (mNodeList.size() - 1)) {
    Logger::log(1, "%s error: IK chaine root node %i is out of range\n", __FUNCTION__,
      ikChainRootNodeNum);
    return;
  }

  std::vector<std::shared_ptr<GltfNode>> ikNodes{};
  int currentNodeNum = effectorNodeNum;

  ikNodes.insert(ikNodes.begin(), mNodeList.at(effectorNodeNum));
  while (currentNodeNum != ikChainRootNodeNum) {
    std::shared_ptr<GltfNode> node = mNodeList.at(currentNodeNum);
    if (node) {
      std::shared_ptr<GltfNode> parentNode = node->getParentNode();
      if (parentNode) {
        currentNodeNum = parentNode->getNodeNum();
        ikNodes.push_back(parentNode);
      } else {
        /* force stopping on the root node */
        Logger::log(1, "%s error: reached skeleton root node, stopping\n", __FUNCTION__);
        break;
      }
    }
  }

  mIKSolver.setNodes(ikNodes);
}

void GltfInstance::setNumIKIterations(int iterations) {
  mIKSolver.setNumIterations(iterations);
}

void GltfInstance::solveIKByCCD(glm::vec3 target)  {
  mIKSolver.solveCCD(target);
  updateNodeMatrices(mIKSolver.getIkChainRootNode());
}

void GltfInstance::solveIKByFABRIK(glm::vec3 target)  {
  mIKSolver.solveFABRIK(target);
  updateNodeMatrices(mIKSolver.getIkChainRootNode());
}
