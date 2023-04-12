#pragma once
#include <string>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "OGLRenderer.h"
#include "Model.h"

class Window {
  public:
    bool init(unsigned int width, unsigned int height, std::string title);
    void mainLoop();
    void cleanup();

  private:
    GLFWwindow *mWindow = nullptr;

    std::unique_ptr<OGLRenderer> mRenderer;
    std::unique_ptr<Model> mModel;

    unsigned int mScreenWidth = 640;
    unsigned int mScreenHeight = 480;
};
