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

    mChildNodes.push_back(child);
  }
}

std::vector<std::shared_ptr<GltfNode>> GltfNode::getChilds() {
  return mChildNodes;
}

int GltfNode::getNodeNum() {
  return mNodeNum;
}

void GltfNode::setNodeName(std::string name) {
  mNodeName = name;
}

void GltfNode::setScale(glm::vec3 scale) {
  mScale = scale;
  mBlendScale = scale;
}

void GltfNode::setTranslation(glm::vec3 translation) {
  mTranslation = translation;
  mBlendTranslation = translation;
}

void GltfNode::setRotation(glm::quat rotation) {
  mRotation = rotation;
  mBlendRotation = rotation;
}

void GltfNode::blendScale(glm::vec3 scale, float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendScale = scale * factor + mScale * (1.0f - factor);
}

void GltfNode::blendTranslation(glm::vec3 translation, float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendTranslation = translation * factor + mTranslation * (1.0f - factor) ;
}

void GltfNode::blendRotation(glm::quat rotation,  float blendFactor) {
  float factor = std::min(std::max(blendFactor, 0.0f), 1.0f);
  mBlendRotation = glm::slerp(mRotation, rotation, factor);
}

void GltfNode::calculateLocalTRSMatrix() {
  glm::mat4 sMatrix = glm::scale(glm::mat4(1.0f), mBlendScale);
  glm::mat4 rMatrix = glm::mat4_cast(mBlendRotation);
  glm::mat4 tMatrix = glm::translate(glm::mat4(1.0f), mBlendTranslation);
  mLocalTRSMatrix = tMatrix * rMatrix * sMatrix;
}

void GltfNode::calculateNodeMatrix(glm::mat4 parentNodeMatrix) {
  mNodeMatrix = parentNodeMatrix * mLocalTRSMatrix;
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
