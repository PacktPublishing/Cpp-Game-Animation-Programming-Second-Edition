#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"
#include "Logger.h"

void Texture::cleanup() {
  glDeleteTextures(1, &mTexture);
}

bool Texture::loadTexture(std::string textureFilename) {
  mTextureName = textureFilename;

  stbi_set_flip_vertically_on_load(true);
  unsigned char *textureData = stbi_load(textureFilename.c_str(), &mTexWidth, &mTexHeight, &mNumberOfChannels, 0);

  if (!textureData) {
    Logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__, mTextureName.c_str());
    stbi_image_free(textureData);
    return false;
  }

  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_2D, mTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(textureData);

  Logger::log(1, "%s: texture '%s' loaded (%dx%d, %d channels)\n", __FUNCTION__, mTextureName.c_str(), mTexWidth, mTexHeight, mNumberOfChannels);
  return true;
}

void Texture::bind() {
  glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
