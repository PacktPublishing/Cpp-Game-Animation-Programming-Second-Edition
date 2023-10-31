/* OpenGL uniform buffer */
#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

class UniformBuffer {
  public:
    void init();
    void uploadUboData(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    void cleanup();

  private:
    GLuint mUboBuffer = 0;
};
