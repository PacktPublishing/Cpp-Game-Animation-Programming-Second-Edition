#include "Window.h"
#include "Logger.h"

bool Window::init(unsigned int width, unsigned int height, std::string title) {
  if (!glfwInit()) {
    Logger::log(1, "%s: glfwInit() error\n", __FUNCTION__);
    return false;
  }

  /* set a "hint" for the NEXT window created*/
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

  if (!mWindow) {
    glfwTerminate();
    Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
    return false;
  }

  glfwMakeContextCurrent(mWindow);

  Logger::log(1, "%s: Window successfully initialized\n", __FUNCTION__);
  return true;
}

void Window::mainLoop() {
  /* force VSYNC */
  glfwSwapInterval(1);

  float color = 0.0f;
  while (!glfwWindowShouldClose(mWindow)) {

    color >= 1.0f ? color = 0.0f : color += 0.01f;

    glClearColor(color, color, color, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* swap buffers */
    glfwSwapBuffers(mWindow);

    /* poll events in a loop */
    glfwPollEvents();

  }
}

void Window::cleanup() {
  Logger::log(1, "%s: Terminating Window\n", __FUNCTION__);
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}
