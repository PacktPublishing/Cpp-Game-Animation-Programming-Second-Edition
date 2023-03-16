#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
  public:
    bool loadTexture(std::string textureFilename);
    void bind();
    void unbind();
    void cleanup();

  private:
    GLuint mTexture = 0;
    int mTexWidth;
    int mTexHeight;
    int mNumberOfChannels;
    std::string mTextureName;
};
