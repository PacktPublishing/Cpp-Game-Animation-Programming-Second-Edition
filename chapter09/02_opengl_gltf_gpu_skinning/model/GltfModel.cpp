#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "GltfModel.h"
#include "Logger.h"

bool GltfModel::loadModel(OGLRenderData &renderData,
    std::string modelFilename, std::string textureFilename) {
  if (!mTex.loadTexture(textureFilename, false)) {
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

  glGenVertexArrays(1, &mVAO);
  glBindVertexArray(mVAO);

  /* extract position, normal, texture coords, and indices */
  createVertexBuffers();
  createIndexBuffer();

  glBindVertexArray(0);

  /* extract joints, weights, and invers bind matrices*/
  getJointData();
  getWeightData();
  getInvBindMatrices();

  /* build model tree */
  int nodeCount = mModel->nodes.size();
  int rootNode = mModel->scenes.at(0).nodes.at(0);
  Logger::log(1, "%s: model has %i nodes, root node is %i\n", __FUNCTION__, nodeCount,
    rootNode);

  mRootNode = GltfNode::createRoot(rootNode);
  getNodeData(mRootNode, glm::mat4(1.0f));
  getNodes(mRootNode);

  /* get Skeleton data */
  mSkeletonMesh = std::make_shared<OGLMesh>();

  mRootNode->printTree();

  renderData.rdGltfTriangleCount = getTriangleCount();

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

std::shared_ptr<OGLMesh> GltfModel::getSkeleton(bool enableSkinning) {
  mSkeletonMesh->vertices.resize(mModel->nodes.size() * 2);
  mSkeletonMesh->vertices.clear();

  /* start from Armature child */
  getSkeletonPerNode(mRootNode->getChilds().at(0), enableSkinning);
  return mSkeletonMesh;
}

void GltfModel::getSkeletonPerNode(std::shared_ptr<GltfNode> treeNode, bool enableSkinning) {
  glm::vec3 parentPos = glm::vec3(0.0f);
  if (enableSkinning) {
    parentPos = glm::vec3(treeNode->getNodeMatrix()[3]);
  } else {
    glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(treeNode->getNodeNum())));
    parentPos = bindMatrix  * treeNode->getNodeMatrix()[3];
  }
  OGLVertex parentVertex;
  parentVertex.position = parentPos;
  parentVertex.color = glm::vec3(0.0f, 1.0f, 1.0f);

  for (const auto &childNode : treeNode->getChilds()) {
    glm::vec3 childPos = glm::vec3(0.0f);
    if (enableSkinning) {
      childPos = glm::vec3(childNode->getNodeMatrix()[3]);
    } else {
      glm::mat4 bindMatrix = glm::inverse(mInverseBindMatrices.at(mNodeToJoint.at(childNode->getNodeNum())));
      childPos = bindMatrix * childNode->getNodeMatrix()[3];
    }
    OGLVertex childVertex;
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

  /* remove the child node with skin/mesh metadata, confuses skeleton */
  auto removeIt = std::remove_if(childNodes.begin(), childNodes.end(),
    [&](int num) { return mModel->nodes.at(num).skin != -1; }
  );
  childNodes.erase(removeIt, childNodes.end());

  treeNode->addChilds(childNodes);
  glm::mat4 treeNodeMatrix = treeNode->getNodeMatrix();

  for (auto &childNode : treeNode->getChilds()) {
    getNodeData(childNode, treeNodeMatrix);
    getNodes(childNode);
  }
}

void GltfModel::getNodeData(std::shared_ptr<GltfNode> treeNode, glm::mat4 parentNodeMatrix) {
  int nodeNum = treeNode->getNodeNum();
  const tinygltf::Node &node = mModel->nodes.at(nodeNum);
  treeNode->setNodeName(node.name);

  if (node.translation.size()) {
    treeNode->setTranslation(glm::make_vec3(node.translation.data()));
  }
  if (node.rotation.size()) {
    treeNode->setRotation(glm::make_quat(node.rotation.data()));
  }
  if (node.scale.size()) {
    treeNode->setScale(glm::make_vec3(node.scale.data()));
  }

  treeNode->calculateLocalTRSMatrix();
  treeNode->calculateNodeMatrix(parentNodeMatrix);

  mJointMatrices.at(mNodeToJoint.at(nodeNum)) =
    treeNode->getNodeMatrix() * mInverseBindMatrices.at(mNodeToJoint.at(nodeNum));
}

void GltfModel::createVertexBuffers() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  mVertexVBO.resize(primitives.attributes.size());
  mAttribAccessors.resize(primitives.attributes.size());

  for (const auto& attrib : primitives.attributes) {
    const std::string attribType = attrib.first;
    const int accessorNum = attrib.second;

    const tinygltf::Accessor &accessor = mModel->accessors.at(accessorNum);
    const tinygltf::BufferView &bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer &buffer = mModel->buffers.at(bufferView.buffer);

    if ((attribType.compare("POSITION") != 0) && (attribType.compare("NORMAL") != 0)
        && (attribType.compare("TEXCOORD_0") != 0) && (attribType.compare("JOINTS_0") != 0
        && (attribType.compare("WEIGHTS_0") != 0))) {
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

    int dataSize = 1;
    switch(accessor.type) {
      case TINYGLTF_TYPE_SCALAR:
        dataSize = 1;
        break;
      case TINYGLTF_TYPE_VEC2:
        dataSize = 2;
        break;
      case TINYGLTF_TYPE_VEC3:
        dataSize = 3;
        break;
      case TINYGLTF_TYPE_VEC4:
        dataSize = 4;
        break;
      default:
        Logger::log(1, "%s error: accessor %i uses data size %i\n", __FUNCTION__,
          accessorNum, accessor.type);
        break;
    }

    GLuint dataType = GL_FLOAT;
    switch(accessor.componentType) {
      case TINYGLTF_COMPONENT_TYPE_FLOAT:
        dataType = GL_FLOAT;
        break;
      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        dataType = GL_UNSIGNED_SHORT;
        break;
      default:
        Logger::log(1, "%s error: accessor %i uses unknown data type %i\n", __FUNCTION__,
          accessorNum, accessor.componentType);
        break;
    }

    /* buffers for position, normal and tex coordinates */
    glGenBuffers(1, &mVertexVBO.at(attributes.at(attribType)));
    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO.at(attributes.at(attribType)));

    glVertexAttribPointer(attributes.at(attribType), dataSize, dataType, GL_FALSE,
      0, (void*) 0);
    glEnableVertexAttribArray(attributes.at(attribType));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void GltfModel::createIndexBuffer() {
  glGenBuffers(1, &mIndexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVBO);

  /* do NOT unbind the element buffer here */
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GltfModel::uploadVertexBuffers() {
  for (int i = 0; i < 5; ++i) {
    const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(i));
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO.at(i));
    glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

void GltfModel::applyCPUVertexSkinning() {
  const tinygltf::Accessor& accessor = mModel->accessors.at(mAttribAccessors.at(0));
  const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
  const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

  std::memcpy(mAlteredPositions.data(), &buffer.data.at(0) + bufferView.byteOffset, bufferView.byteLength);

  for (int i = 0; i < mJointVec.size(); ++i) {
    glm::ivec4 jointIndex = glm::make_vec4(mJointVec.at(i));
    glm::vec4 weightIndex = glm::make_vec4(mWeightVec.at(i));
    glm::mat4 skinMat =
      weightIndex.x * mJointMatrices.at(jointIndex.x) +
      weightIndex.y * mJointMatrices.at(jointIndex.y) +
      weightIndex.z * mJointMatrices.at(jointIndex.z) +
      weightIndex.w * mJointMatrices.at(jointIndex.w);
    mAlteredPositions.at(i) = skinMat * glm::vec4(mAlteredPositions.at(i), 1.0f);
  }
  glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO.at(0));
  glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
   mAlteredPositions.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GltfModel::uploadIndexBuffer() {
  /* buffer for vertex indices */
  const tinygltf::Primitive& primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor& indexAccessor = mModel->accessors.at(primitives.indices);
  const tinygltf::BufferView& indexBufferView = mModel->bufferViews.at(indexAccessor.bufferView);
  const tinygltf::Buffer& indexBuffer = mModel->buffers.at(indexBufferView.buffer);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength,
    &indexBuffer.data.at(0) + indexBufferView.byteOffset, GL_STATIC_DRAW);
}

int GltfModel::getTriangleCount() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);

  unsigned int triangles = 0;
  switch (primitives.mode) {
    case TINYGLTF_MODE_TRIANGLES:
      triangles =  indexAccessor.count / 3;
      break;
    default:
      Logger::log(1, "%s error: unknown draw mode %i\n", __FUNCTION__, primitives.mode);
      break;
  }
  return triangles;
}

int GltfModel::getJointMatrixSize() {
  return mJointMatrices.size();
}

std::vector<glm::mat4> GltfModel::getJointMatrices() {
  return mJointMatrices;
}

void GltfModel::draw() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  const tinygltf::Accessor &indexAccessor = mModel->accessors.at(primitives.indices);

  GLuint drawMode = GL_TRIANGLES;
  switch (primitives.mode) {
    case TINYGLTF_MODE_TRIANGLES:
      drawMode = GL_TRIANGLES;
      break;
    default:
      Logger::log(1, "%s error: unknown draw mode %i\n", __FUNCTION__, primitives.mode);
      break;
  }

  mTex.bind();
  glBindVertexArray(mVAO);
  glDrawElements(drawMode, indexAccessor.count, indexAccessor.componentType, nullptr);
  glBindVertexArray(0);
  mTex.unbind();
}

void GltfModel::cleanup() {
  glDeleteBuffers(mVertexVBO.size(), mVertexVBO.data());
  glDeleteBuffers(1, &mVAO);
  glDeleteBuffers(1, &mIndexVBO);
  mTex.cleanup();
  mModel.reset();
}
