#include "Window.h"
#include "Logger.h"

bool Window::init(unsigned int width, unsigned int height, std::string title) {
  if (!glfwInit()) {
    Logger::log(1, "%s error: glfwInit() failed\n", __FUNCTION__);
    return false;
  }

  if (!glfwVulkanSupported()) {
    glfwTerminate();
    Logger::log(1, "%s error: Vulkan is not supported\n", __FUNCTION__);
    return false;
  }

  /* Vulkan needs no context */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  if (!mWindow) {
    glfwTerminate();
    Logger::log(1, "%s error: Could not create window\n", __FUNCTION__);
    return false;
  }

  mRenderer = std::make_unique<VkRenderer>(mWindow);
  if (!mRenderer->init(width, height)) {
    glfwTerminate();
    Logger::log(1, "%s error: Could not init Vulkan\n", __FUNCTION__);
    return false;
  }

  glfwSetWindowUserPointer(mWindow, mRenderer.get());
  glfwSetWindowSizeCallback(mWindow, [](GLFWwindow *win, int width, int height) {
    auto renderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(win));
    renderer->setSize(width, height);
    }
  );

  mModel = std::make_unique<Model>();
  mModel->init();
  Logger::log(1, "%s: mockup model data loaded\n", __FUNCTION__);

  Logger::log(1, "%s: Window with Vulkan successfully initialized\n", __FUNCTION__);
  return true;
}


void Window::mainLoop() {
  /* upload only once for now */
  mRenderer->uploadData(mModel->getVertexData());

  while (!glfwWindowShouldClose(mWindow)) {
    if (!mRenderer->draw()) {
      break;
    }

    /* poll events in a loop */
    glfwPollEvents();

  }
}

void Window::cleanup() {
  mRenderer->cleanup();

  glfwDestroyWindow(mWindow);
  glfwTerminate();
  Logger::log(1, "%s: Terminating Window\n", __FUNCTION__);
}
