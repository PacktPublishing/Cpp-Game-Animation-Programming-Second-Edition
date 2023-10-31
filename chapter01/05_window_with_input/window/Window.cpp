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
    Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
    glfwTerminate();
    return false;
  }

  /* the C handlers needs a little 'stunt' here */
  /* 1) save the pointer to the instance as user pointer */
  glfwSetWindowUserPointer(mWindow, this);

  /* 2) use a lambda to get the pointer and call the member function */
  glfwSetWindowPosCallback(mWindow, [](GLFWwindow *win, int xpos, int ypos) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleWindowMoveEvents(xpos, ypos);
    }
  );

  glfwSetWindowIconifyCallback(mWindow, [](GLFWwindow *win, int minimized) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleWindowMinimizedEvents(minimized);
    }
  );

  glfwSetWindowMaximizeCallback(mWindow, [](GLFWwindow *win, int maximized) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleWindowMaximizedEvents(maximized);
    }
  );

  glfwSetWindowCloseCallback(mWindow, [](GLFWwindow *win) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleWindowCloseEvents();
    }
  );

  glfwSetKeyCallback(mWindow, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleKeyEvents(key, scancode, action, mods);
    }
  );

  glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *win, int button, int action, int mods) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleMouseButtonEvents(button, action, mods);
    }
  );

  glfwSetCursorPosCallback(mWindow, [](GLFWwindow *win, double xpos, double ypos) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleMousePositionEvents(xpos, ypos);
    }
  );

  glfwSetCursorEnterCallback(mWindow, [](GLFWwindow *win, int enter) {
    auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
    thisWindow->handleMouseEnterLeaveEvents(enter);
    }
  );

  Logger::log(1, "%s: Window successfully initialized\n", __FUNCTION__);
  return true;
}

void Window::handleWindowMoveEvents(int xpos, int ypos) {
  Logger::log(1, "%s: Window has been moved to %i/%i\n", __FUNCTION__, xpos, ypos);
}

void Window::handleWindowMinimizedEvents(int minimized) {
  if (minimized) {
    Logger::log(1, "%s: Window has been minimized\n", __FUNCTION__);
  } else {
    Logger::log(1, "%s: Window has been restored\n", __FUNCTION__);
  }
}

void Window::handleWindowMaximizedEvents(int maximized) {
  if (maximized) {
    Logger::log(1, "%s: Window has been maximized\n", __FUNCTION__);
  } else {
    Logger::log(1, "%s: Window has been restored\n", __FUNCTION__);
  }
}

void Window::handleWindowCloseEvents() {
  Logger::log(1, "%s: Window got close event... bye!\n", __FUNCTION__);
}

void Window::handleKeyEvents(int key, int scancode, int action, int mods) {
  std::string actionName;
  switch (action) {
    case GLFW_PRESS:
      actionName = "pressed";
      break;
    case GLFW_RELEASE:
      actionName = "released";
      break;
    case GLFW_REPEAT:
      actionName = "repeated";
      break;
    default:
      actionName = "invalid";
      break;
  }

  const char *keyName = glfwGetKeyName(key, 0);
  Logger::log(1, "%s: key %s (key %i, scancode %i) %s\n", __FUNCTION__, keyName, key, scancode, actionName.c_str());
}

void Window::handleMouseButtonEvents(int button, int action, int mods) {
  std::string actionName;
  switch (action) {
    case GLFW_PRESS:
      actionName = "pressed";
      break;
    case GLFW_RELEASE:
      actionName = "released";
      break;
    case GLFW_REPEAT:
      actionName = "repeated";
      break;
    default:
      actionName = "invalid";
      break;
  }

  std::string mouseButtonName;
  switch(button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      mouseButtonName = "left";
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      mouseButtonName = "middle";
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      mouseButtonName = "right";
      break;
    default:
      mouseButtonName = "other";
      break;
  }

  Logger::log(1, "%s: %s mouse button (%i) %s\n", __FUNCTION__, mouseButtonName.c_str(), button, actionName.c_str());
}

void Window::handleMousePositionEvents(double xpos, double ypos) {
  Logger::log(1, "%s: Mouse is at position %lf/%lf\n", __FUNCTION__, xpos, ypos);
}

void Window::handleMouseEnterLeaveEvents(int enter) {
  if (enter) {
    Logger::log(1, "%s: Mouse entered window\n", __FUNCTION__);
  } else {
    Logger::log(1, "%s: Mouse left window\n", __FUNCTION__);
  }
}

void Window::mainLoop() {
  while (!glfwWindowShouldClose(mWindow)) {

    /* poll events in a loop */
    glfwPollEvents();

  }
}

void Window::cleanup() {
  Logger::log(1, "%s: Terminating Window\n", __FUNCTION__);
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}
