#include "TextureBuffer.h"
#include "Logger.h"

void TextureBuffer::init(size_t bufferSize) {
  mBufferSize = bufferSize;

  glGenBuffers(1, &mTextureBuffer);
  glBindBuffer(GL_TEXTURE_BUFFER, mTextureBuffer);
  glBufferData(GL_TEXTURE_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);

  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_BUFFER, mTexture);
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, mTextureBuffer);

  glBindTexture(GL_TEXTURE_BUFFER, 0);
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void TextureBuffer::uploadTboData(std::vector<glm::mat4> bufferData, int bindingPoint) {
  if (bufferData.size() == 0) {
    return;
  }
  mTexNum = bindingPoint;
  size_t bufferSize = bufferData.size() * sizeof(glm::mat4);
  glBindBuffer(GL_TEXTURE_BUFFER, mTextureBuffer);
  glBufferSubData(GL_TEXTURE_BUFFER, 0, bufferSize, bufferData.data());
  glBindBufferRange(GL_TEXTURE_BUFFER, bindingPoint, mTextureBuffer, 0, bufferSize);
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void TextureBuffer::cleanup() {
  glDeleteTextures(1, &mTexture);
  glDeleteBuffers(1, &mTextureBuffer);
}

void TextureBuffer::bind() {
  glActiveTexture(GL_TEXTURE0 + mTexNum);
  glBindTexture(GL_TEXTURE_BUFFER, mTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}
