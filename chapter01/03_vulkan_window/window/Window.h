#pragma once
#include <string>
/* include Vulkan header BEFORE GLFW */
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class Window {
  public:
    bool init(unsigned int width, unsigned int height, std::string title);
    bool initVulkan();
    void mainLoop();
    void cleanup();

  private:
    GLFWwindow *mWindow = nullptr;
    std::string mApplicationName;

    VkInstance mInstance{};
    VkSurfaceKHR mSurface{};
};
