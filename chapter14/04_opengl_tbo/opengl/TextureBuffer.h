#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class TextureBuffer {
  public:
    void init(size_t bufferSize);
    void uploadTboData(std::vector<glm::mat4> bufferData, int bindingPoint);
    void bind();
    void cleanup();

  private:
    size_t mBufferSize = 0;
    GLuint mTexNum = 0;
    GLuint mTexture = 0;
    GLuint mTextureBuffer = 0;
};
