#include "UniformBuffer.h"
#include "Logger.h"

void UniformBuffer::init(size_t bufferSize) {
  mBufferSize = bufferSize;

  glGenBuffers(1, &mUboBuffer);

  glBindBuffer(GL_UNIFORM_BUFFER, mUboBuffer);
  glBufferData(GL_UNIFORM_BUFFER, mBufferSize, NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::uploadUboData(std::vector<glm::mat4> bufferData, int bindingPoint) {
  glBindBuffer(GL_UNIFORM_BUFFER, mUboBuffer);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, mBufferSize, bufferData.data());
  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, mUboBuffer, 0, mBufferSize);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::cleanup() {
  glDeleteBuffers(1, &mUboBuffer);
}
