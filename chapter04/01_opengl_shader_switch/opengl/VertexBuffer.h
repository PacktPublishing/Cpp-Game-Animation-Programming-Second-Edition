#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "OGLRenderData.h"

class VertexBuffer {
  public:
    void init();
    void uploadData(OGLMesh vertexData);
    void bind();
    void unbind();
    void draw(GLuint mode, unsigned int start, unsigned int num);
    void cleanup();

  private:
    GLuint mVAO = 0;
    GLuint mVertexVBO = 0;
};
