#pragma once
#include <string>
#include <GLFW/glfw3.h>

class Window {
  public:
    bool init(unsigned int width, unsigned int height, std::string title);
    void mainLoop();
    void cleanup();

  private:
    GLFWwindow *mWindow = nullptr;

    void handleWindowMoveEvents(int xpos, int ypos);
    void handleWindowMinimizedEvents(int minimized);
    void handleWindowMaximizedEvents(int maximized);
    void handleWindowCloseEvents();

    void handleKeyEvents(int key, int scancode, int action, int mods);
    void handleMouseButtonEvents(int button, int action, int mods);
    void handleMousePositionEvents(double xpos, double ypos);
    void handleMouseEnterLeaveEvents(int enter);
};
