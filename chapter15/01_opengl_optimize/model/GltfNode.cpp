#include <algorithm>
#include <glm/gtx/quaternion.hpp>

#include "GltfNode.h"
#include "Logger.h"

GltfNode::~GltfNode() {
  Logger::log(2, "%s: removing node number %i (%s)\n", __FUNCTION__, mNodeNum, mNodeName.c_str());
}

std::shared_ptr<GltfNode> GltfNode::createRoot(int rootNodeNum) {
  std::shared_ptr<GltfNode> mParentNode = std::make_shared<GltfNode>();
  mParentNode->mNodeNum = rootNodeNum;
  return mParentNode;
}

void GltfNode::addChilds(std::vector<int> childNodes) {
  for (const int childNode : childNodes) {
    std::shared_ptr<GltfNode> child = std::make_shared<GltfNode>();
    child->mNodeNum = childNode;
    child->mParentNode = shared_from_this();

    mChildNodes.push_back(child);
  }
}

std::shared_ptr<GltfNode> GltfNode::getParentNode() {
  std::shared_ptr<GltfNode> pNode = mParentNode.lock();
  if (pNode) {
    return pNode;
  }
  return nullptr;
}

std::vector<std::shared_ptr<GltfNode>> GltfNode::getChilds() {
  return mChildNodes;
}

void GltfNode::updateNodeAndChildMatrices() {
  /* update this matrix */
  calculateNodeMatrix();

  /* call recursive for child update */
  for (auto &node : mChildNodes) {
    if (node) {
      node->updateNodeAndChildMatrices();
    }
  }
}

int GltfNode::getNodeNum() {
  return mNodeNum;
}

void GltfNode::setNodeName(std::string name) {
  mNodeName = name;
}

std::string GltfNode::getNodeName() {
  return mNodeName;
}

void GltfNode::setScale(glm::vec3 scale) {
  mScale = scale;
  mBlendScale = scale;
  mScaleMatrix = glm::scale(glm::mat4(1.0f), mBlendScale);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::setTranslation(glm::vec3 translation) {
  mTranslation = translation;
  mBlendTranslation = translation;
  mTranslationMatrix = glm::translate(glm::mat4(1.0f), mBlendTranslation);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::setRotation(glm::quat rotation) {
  mRotation = rotation;
  mBlendRotation = rotation;
  mRotationMatrix = glm::mat4_cast(mBlendRotation);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::blendScale(glm::vec3 scale, float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendScale = scale * factor + mScale * (1.0f - factor);
  mScaleMatrix = glm::scale(glm::mat4(1.0f), mBlendScale);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::blendTranslation(glm::vec3 translation, float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendTranslation = translation * factor + mTranslation * (1.0f - factor) ;
  mTranslationMatrix = glm::translate(glm::mat4(1.0f), mBlendTranslation);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::blendRotation(glm::quat rotation, float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendRotation = glm::slerp(mRotation, rotation, factor);
  mRotationMatrix = glm::mat4_cast(mBlendRotation);
  mLocalMatrixNeedsUpdate = true;
}

void GltfNode::setWorldPosition(glm::vec3 worldPos) {
  mWorldPosition = worldPos;
  mWorldTranslationMatrix = glm::translate(glm::mat4(1.0f), mWorldPosition);
  mWorldTRMatrix = mWorldTranslationMatrix * mWorldRotationMatrix;
  mLocalMatrixNeedsUpdate = true;
  updateNodeAndChildMatrices();
}

void GltfNode::setWorldRotation(glm::vec3 worldRot) {
  mWorldRotation = worldRot;
  mWorldRotationMatrix = glm::mat4_cast(glm::quat(glm::vec3(
    glm::radians(mWorldRotation.x),
    glm::radians(mWorldRotation.y),
    glm::radians(mWorldRotation.z)
  )));
  mWorldTRMatrix = mWorldTranslationMatrix * mWorldRotationMatrix;
  mLocalMatrixNeedsUpdate = true;
  updateNodeAndChildMatrices();
}

glm::vec3 GltfNode::getWorldPosition() {
  return mWorldPosition;
}

void GltfNode::calculateLocalTRSMatrix() {
  if (mLocalMatrixNeedsUpdate) {
    mLocalTRSMatrix = mWorldTRMatrix * mTranslationMatrix * mRotationMatrix * mScaleMatrix;
    mLocalMatrixNeedsUpdate = false;
  }
}

void GltfNode::calculateNodeMatrix() {
  calculateLocalTRSMatrix();

  if (std::shared_ptr<GltfNode> pNode = mParentNode.lock()) {
    mParentNodeMatrix = pNode->getNodeMatrix();
  }

  mNodeMatrix = mParentNodeMatrix * mLocalTRSMatrix;
}

glm::mat4 GltfNode::getNodeMatrix() {
  return mNodeMatrix;
}

void GltfNode::printTree() {
  Logger::log(1, "%s: ---- tree ----\n", __FUNCTION__);
  Logger::log(1, "%s: parent : %i (%s)\n", __FUNCTION__, mNodeNum, mNodeName.c_str());
  for (const auto& childNode : mChildNodes) {
    GltfNode::printNodes(childNode, 1);
  }
  Logger::log(1, "%s: -- end tree --\n", __FUNCTION__);
}

void GltfNode::printNodes(std::shared_ptr<GltfNode> node, int indent) {
  std::string indendString = "";
  for (int i = 0; i < indent; ++i) {
    indendString += " ";
  }
  indendString += "-";
  Logger::log(1, "%s: %s child : %i (%s)\n", __FUNCTION__,
    indendString.c_str(), node->mNodeNum, node->mNodeName.c_str());

  for (const auto& childNode : node->mChildNodes) {
    GltfNode::printNodes(childNode, indent + 1);
  }
}
