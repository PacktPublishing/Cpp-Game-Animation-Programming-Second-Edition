#include <vector>

#include "Window.h"
#include "Logger.h"

bool Window::init(unsigned int width, unsigned int height, std::string title) {
  if (!glfwInit()) {
    Logger::log(1, "%s: glfwInit() error\n", __FUNCTION__);
    return false;
  }

  if (!glfwVulkanSupported()) {
    glfwTerminate();
    Logger::log(1, "%s: Vulkan is not supported\n", __FUNCTION__);
    return false;
  }

  /* set a "hint" for the NEXT window created*/
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  mApplicationName = title;

  /* Vulkan needs no context */
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  mWindow = glfwCreateWindow(width, height, mApplicationName.c_str(), nullptr, nullptr);

  if (!mWindow) {
    Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
    glfwTerminate();
    return false;
  }

  if (!initVulkan()) {
    Logger::log(1, "%s: Could not init Vulkan\n", __FUNCTION__);
    glfwTerminate();
    return false;
  }

  Logger::log(1, "%s: Window successfully initialized\n", __FUNCTION__);
  return true;
}

bool Window::initVulkan() {
  VkResult result = VK_ERROR_UNKNOWN;

  VkApplicationInfo mAppInfo{};
  mAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  mAppInfo.pNext = nullptr;
  mAppInfo.pApplicationName = mApplicationName.c_str();
  mAppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
  mAppInfo.pEngineName = "Game Animations Programming";
  mAppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  mAppInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 1, 0);

  uint32_t extensionCount = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

  if (extensionCount == 0) {
    Logger::log(1, "%s error: no Vulkan extensions found, need at least 'VK_KHR_surface'\n", __FUNCTION__);
    return false;
  }

  Logger::log(1, "%s: Found %u Vulkan extensions\n", __FUNCTION__, extensionCount);
  for (int i = 0; i < extensionCount; ++i) {
    Logger::log(1, "%s: %s\n", __FUNCTION__, std::string(extensions[i]).c_str());
  }

  VkInstanceCreateInfo mCreateInfo{};
  mCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  mCreateInfo.pNext = nullptr;
  mCreateInfo.pApplicationInfo = &mAppInfo;
  mCreateInfo.enabledExtensionCount = extensionCount;
  mCreateInfo.ppEnabledExtensionNames = extensions;
  mCreateInfo.enabledLayerCount = 0;

  result = vkCreateInstance(&mCreateInfo, nullptr, &mInstance);
  if (result != VK_SUCCESS) {
    Logger::log(1, "%s: Could not create Vulkan instance (%i)\n", __FUNCTION__, result);
    return false;
  }

  uint32_t physicalDeviceCount = 0;
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);

  if (physicalDeviceCount == 0) {
    Logger::log(1, "%s: No Vulkan capable GPU found\n", __FUNCTION__);
    return false;
  }

  std::vector<VkPhysicalDevice> devices;
  devices.resize(physicalDeviceCount);
  vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, devices.data());

  Logger::log(1, "%s: Found %u physical device(s)\n", __FUNCTION__, physicalDeviceCount);

  result = glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface);
  if (result != VK_SUCCESS) {
    Logger::log(1, "%s: Could not create Vulkan surface\n", __FUNCTION__);
    return false;
  }

  return true;
}

void Window::mainLoop() {
  while (!glfwWindowShouldClose(mWindow)) {

    /* poll events in a loop */
    glfwPollEvents();

  }
}

void Window::cleanup() {
  Logger::log(1, "%s: Terminating Window\n", __FUNCTION__);

  vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
  vkDestroyInstance(mInstance, nullptr);

  glfwDestroyWindow(mWindow);
  glfwTerminate();
}
