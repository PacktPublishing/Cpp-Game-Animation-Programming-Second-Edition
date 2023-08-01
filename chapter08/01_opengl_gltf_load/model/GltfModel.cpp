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

  renderData.rdGltfTriangleCount = getTriangleCount();

  return true;
}

void GltfModel::createVertexBuffers() {
  const tinygltf::Primitive &primitives = mModel->meshes.at(0).primitives.at(0);
  mVertexVBO.resize(primitives.attributes.size());

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
  for (int i = 0; i < 3; ++i) {
    const tinygltf::Accessor& accessor = mModel->accessors.at(i);
    const tinygltf::BufferView& bufferView = mModel->bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = mModel->buffers.at(bufferView.buffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO.at(i));
    glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
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
      triangles =  indexAccessor.count;
      break;
    default:
      Logger::log(1, "%s error: unknown draw mode %i\n", __FUNCTION__, primitives.mode);
      break;
  }
  return triangles;
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
