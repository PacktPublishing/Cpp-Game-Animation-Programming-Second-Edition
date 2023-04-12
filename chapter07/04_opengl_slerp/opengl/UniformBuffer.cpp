#include <glm/gtc/type_ptr.hpp>

#include "UniformBuffer.h"
#include "Logger.h"

void UniformBuffer::init() {
  /* create UBO for 2x 4x4 matrices */
  glGenBuffers(1, &mUboBuffer);

  glBindBuffer(GL_UNIFORM_BUFFER, mUboBuffer);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::uploadUboData(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
  glBindBuffer(GL_UNIFORM_BUFFER, mUboBuffer);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(viewMatrix));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUboBuffer, 0, 2 * sizeof(glm::mat4));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::cleanup() {
  glDeleteBuffers(1, &mUboBuffer);
}
