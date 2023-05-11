/* OpenGL shader stroage buffer */
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class ShaderStorageBuffer {
  public:
    void init(size_t bufferSize);
    void uploadSsboData(std::vector<glm::mat4> bufferData, int bindingPoint);
    void cleanup();

  private:
    size_t mBufferSize;
    GLuint mShaderStorageBuffer = 0;
};
