#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GltfModel.h"
#include "Logger.h"

bool GltfModel::loadModel(VkRenderData &renderData, VkGltfRenderData &gltfRenderData,
    std::string modelFilename, std::string textureFilename) {
  if (!Texture::loadTexture(renderData, gltfRenderData.rdGltfModelTexture, textureFilename)) {
    Logger::log(1, "%s: texture loading failed\n", __FUNCTION__);
    return false;
  }
  Logger::log(1, "%s: glTF model texture '%s' successfully loaded\n", __FUNCTION__,
    modelFilename.c_str());

  mModel = std::make_shared<tinygltf::Model>();

  tinygltf::TinyGLTF gltfLoader;
  std::string loaderErrors;
  std::string loaderWarnings;
  bool result = false;

  result = gltfLoader.LoadASCIIFromFile(mModel.get(), &loaderErrors, &loaderWarnings,
    modelFilename);

  if (!loaderWarnings.empty()) {
    Logger::log(1, "%s: warnings while loading glTF model:\n%s\n", __FUNCTION__,
      loaderWarnings.c_str());
  }

  if (!loaderErrors.empty()) {
    Logger::log(1, "%s: errors while loading glTF model:\n%s\n", __FUNCTION__,
      loaderErrors.c_str());
  }

  if (!result) {
    Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__,
      modelFilename.c_str());
    return false;
  }

  createVertexBuffers(renderData, gltfRenderData);
  createIndexBuffer(renderData, gltfRenderData);

  /* extract joints, weights, and invers bind matrices*/
  getJointData();
  getWeightData();
  getInvBindMatrices();

  /* build model tree */
  int nodeCount = mModel->nodes.size();
  int rootNode = mModel->scenes.at(0).nodes.at(0);
  Logger::log(1, "%s: model has %i nodes, root node is %i\n", __FUNCTION__, nodeCount, rootNode);

  mRootNode = GltfNode::createRoot(rootNode);
  const tinygltf::Node &node = mModel->nodes.at(rootNode);
  getNodeData(mRootNode);
  getNodes(mRootNode);

  /* get Skeleton data */
  mSkeletonMesh = std::make_shared<VkMesh>();

  //mRootNode->printTree();

  renderData.rdGltfTriangleCount = getTriangleCount();;

  return true;
}

void GltfModel::getJointData() {
  std::string jointsAccessorAttrib = "JOINTS_0";
  int jointsAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(jointsAccessorAttrib);
  Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, jointsAccessor,
    jointsAccessorAttrib.c_str());

  const tinygltf::Accessor &accessor = mModel->accessors.at(jointsAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  int jointVecSize = accessor.count;
  Logger::log(1, "%s: %i short vec4 in JOINTS_0\n", __FUNCTION__, jointVecSize);
  mJointVec.resize(jointVecSize);

  std::memcpy(mJointVec.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);

  mNodeToJoint.resize(mModel->nodes.size());

  const tinygltf::Skin &skin = mModel->skins.at(0);
  for (int i = 0; i < skin.joints.size(); ++i) {
    int destinationNode = skin.joints.at(i);
    mNodeToJoint.at(destinationNode) = i;
    Logger::log(2, "%s: joint %i affects node %i\n", __FUNCTION__, i, destinationNode);
  }
}

void GltfModel::getWeightData() {
  std::string weightsAccessorAttrib = "WEIGHTS_0";
  int weightAccessor = mModel->meshes.at(0).primitives.at(0).attributes.at(weightsAccessorAttrib);
  Logger::log(1, "%s: using accessor %i to get %s\n", __FUNCTION__, weightAccessor,
    weightsAccessorAttrib.c_str());

  const tinygltf::Accessor &accessor = mModel->accessors.at(weightAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  int weightVecSize = accessor.count;
  Logger::log(1, "%s: %i vec4 in WEIGHTS_0\n", __FUNCTION__, weightVecSize);
  mWeightVec.resize(weightVecSize);

  std::memcpy(mWeightVec.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);
}

void GltfModel::getInvBindMatrices() {
  const tinygltf::Skin &skin = mModel->skins.at(0);
  int invBindMatAccessor = skin.inverseBindMatrices;

  const tinygltf::Accessor &accessor = mModel->accessors.at(invBindMatAccessor);
  const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

  mInverseBindMatrices.resize(skin.joints.size());
  mJointMatrices.resize(skin.joints.size());

  std::memcpy(mInverseBindMatrices.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);
}

std::shared_ptr<VkMesh> GltfModel::getSkeleton(bool enableSkinning) {
  mSkeletonMesh->vertices.resize(mModel->nodes.size() * 2);
  mSkeletonMesh->vertices.clear();

  /* start from Armature child */
  getSkeletonPerNode(mRootNode->getChilds().at(0), enableSkinning);
  return mSkeletonMesh;
}

void GltfModel::getSkeletonPerNode(std::shared_ptr<GltfNode> treeNode, bool enableSkinning) {
  glm::vec3 parentPos = glm::vec3(0.0f);
  if (enableSkinning) {
    parentPos = glm::vec3(treeNode->getNodeMatrix() * glm::vec4(1.0f));
  } else {
    glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(treeNode->getNodeNum())));
    parentPos = bindMatrix  * treeNode->getNodeMatrix() * glm::vec4(1.0f);
  }
  VkVertex parentVertex;
  parentVertex.position = parentPos;
  parentVertex.color = glm::vec3(0.0f, 1.0f, 1.0f);

  for (const auto &childNode : treeNode->getChilds()) {
    glm::vec3 childPos = glm::vec3(0.0f);
    if (enableSkinning) {
      childPos = glm::vec3(childNode->getNodeMatrix() * glm::vec4(1.0f));
    } else {
      glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(childNode->getNodeNum())));
      childPos = bindMatrix * childNode->getNodeMatrix() * glm::vec4(1.0f);
    }
    VkVertex childVertex;
    childVertex.position = childPos;
    childVertex.color = glm::vec3(0.0f, 0.0f, 1.0f);
    mSkeletonMesh->vertices.emplace_back(parentVertex);
    mSkeletonMesh->vertices.emplace_back(childVertex);

    getSkeletonPerNode(childNode, enableSkinning);
  }
}

void GltfModel::getNodes(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();
  std::vector<int> childNodes = mModel->nodes.at(nodeNum).children;

  /* remove the child node with skin/mesh metadata */
  auto removeIt = std::remove_if(childNodes.begin(), childNodes.end(),
    [&](int num) { return mModel->nodes.at(num).skin != -1; }
  );
  childNodes.erase(removeIt, childNodes.end());

  treeNode->addChilds(childNodes);

  for (const auto &childNode : treeNode->getChilds()) {
    getNodeData(childNode);
    getNodes(childNode);
  }
}

void GltfModel::getNodeData(std::shared_ptr<GltfNode> treeNode) {
  int nodeNum = treeNode->getNodeNum();
  const tinygltf::Node &node = mModel->nodes.at(nodeNum);
  treeNode->setNodeName(node.name);

  if (node.matrix.size()) {
    treeNode->setMatrix(glm::make_mat4(node.matrix.data()));
  } else {
    if (node.translation.size()) {
      treeNode->setTranslation(glm::make_vec3(node.translation.data()));
    }
    if (node.rotation.size()) {
      treeNode->setRotation(glm::make_quat(node.rotation.data()));
    }
    if (node.scale.size()) {
      treeNode->setScale(glm::make_vec3(node.scale.data()));
    }
  }

  treeNode->calculateLocalTRSMatrix();

  mJointMatrices.at(mNodeToJoint.at(nodeNum)) =
    treeNode->getNodeMatrix() * mInverseBindMatrices.at(mNodeToJoint.at(nodeNum));
}

void GltfModel::createVertexBuffers(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  gltfRenderData.rdGltfVertexBufferData.resize(primitives.attributes.size());
  mAttribAccessors.resize(primitives.attributes.size());

  for (const auto& attrib : primitives.attributes) {
    const std::string attribType = attrib.first;
    const int accessorNum = attrib.second;

    const tinygltf::Accessor &accessor = mModel->accessors.at(accessorNum);
    const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

    if ((attribType.compare("POSITION") != 0) && (attribType.compare("NORMAL") != 0)
        && (attribType.compare("TEXCOORD_0") != 0)) {
      Logger::log(1, "%s: skipping attribute type %s\n", __FUNCTION__, attribType.c_str());
      continue;
    }

    Logger::log(1, "%s: data for %s uses accessor %i\n", __FUNCTION__, attribType.c_str(),
      accessorNum);
    if (attribType.compare("POSITION") == 0) {
      int numPositionEntries = accessor.count;
      mAlteredPositions.resize(numPositionEntries);
      Logger::log(1, "%s: loaded %i vertices from glTF file\n", __FUNCTION__,
        numPositionEntries);
    }

    mAttribAccessors.at(attributes.at(attribType)) = accessorNum;

    /* buffers for position, normal and tex coordinates */
    VertexBuffer::init(renderData,
      gltfRenderData.rdGltfVertexBufferData.at(attributes.at(attribType)),
      bufferView.byteLength);
  }
}

void GltfModel::createIndexBuffer(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  /* buffer for vertex indices */
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView &indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer &indexBuffer = mModel->buffers.at(indexBufferView.buffer);

  IndexBuffer::init(renderData, gltfRenderData.rdGltfIndexBufferData, indexBufferView.byteLength);
}

void GltfModel::uploadVertexBuffers(VkRenderData& renderData, VkGltfRenderData& gltfRenderData) {
  for (int i = 0; i < 3; ++i) {
    const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(i));
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    VertexBuffer::uploadData(renderData,
      gltfRenderData.rdGltfVertexBufferData.at(i), buffer, bufferView);
  }
}

void GltfModel::uploadPositionBuffer(VkRenderData& renderData, VkGltfRenderData& gltfRenderData) {
  const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(0));
  const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

  VertexBuffer::uploadData(renderData,
    gltfRenderData.rdGltfVertexBufferData.at(0), mAlteredPositions);
}

void GltfModel::applyVertexSkinning(bool enableSkinning) {
  const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(0));
  const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

  std::memcpy(mAlteredPositions.data(), &buffer.data.at(0) + bufferView.byteOffset,
    bufferView.byteLength);

  if (enableSkinning) {
    for (int i = 0; i < mJointVec.size(); ++i) {
      glm::ivec4 joindIndex = glm::make_vec4(mJointVec.at(i));
      glm::vec4 weightIndex = glm::make_vec4(mWeightVec.at(i));
      glm::mat4 skinMat =
        weightIndex.x * mJointMatrices.at(joindIndex.x) +
        weightIndex.y * mJointMatrices.at(joindIndex.y) +
        weightIndex.z * mJointMatrices.at(joindIndex.z) +
        weightIndex.w * mJointMatrices.at(joindIndex.w);
      mAlteredPositions.at(i) = skinMat * glm::vec4(mAlteredPositions.at(i), 1.0f);
    }
  }
}

void GltfModel::uploadIndexBuffer(VkRenderData& renderData, VkGltfRenderData& gltfRenderData) {
  /* buffer for vertex indices */
  const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

  IndexBuffer::uploadData(renderData, gltfRenderData.rdGltfIndexBufferData,
    indexBuffer, indexBufferView);
}

int GltfModel::getTriangleCount() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);
  return indexAccessor.count;
}

void GltfModel::draw(VkRenderData &renderData, VkGltfRenderData& gltfRenderData) {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);

  /* texture */
  vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    renderData.rdGltfPipelineLayout, 0, 1,
    &gltfRenderData.rdGltfModelTexture.texTextureDescriptorSet, 0, nullptr);

  /* vertex buffer */
  VkDeviceSize offset = 0;
  for (int i = 0; i < 3; ++i) {
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, i, 1,
      &gltfRenderData.rdGltfVertexBufferData.at(i).rdVertexBuffer, &offset);
  }

  /* index buffer */
  vkCmdBindIndexBuffer(renderData.rdCommandBuffer,
    gltfRenderData.rdGltfIndexBufferData.rdIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

  vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    renderData.rdGltfPipeline);
  vkCmdDrawIndexed(renderData.rdCommandBuffer,
    static_cast<uint32_t>(renderData.rdGltfTriangleCount), 1, 0, 0, 0);
}

void GltfModel::cleanup(VkRenderData &renderData, VkGltfRenderData &gltfRenderData) {
  for (int i = 0; i < 3 ; ++i) {
    VertexBuffer::cleanup(renderData, gltfRenderData.rdGltfVertexBufferData.at(i));
  }

  IndexBuffer::cleanup(renderData, gltfRenderData.rdGltfIndexBufferData);

  Texture::cleanup(renderData, gltfRenderData.rdGltfModelTexture);
  mModel.reset();
}
