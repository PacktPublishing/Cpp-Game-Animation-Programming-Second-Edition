#include <memory>
#include <cstdlib>

#include "Window.h"
#include "Logger.h"

int main(int argc, char *argv[]) {
  std::unique_ptr<Window> w = std::make_unique<Window>();

  if (!w->init(640, 480, "Vulkan Test Window")) {
    Logger::log(1, "%s error: Window init error\n", __FUNCTION__);
    return -1;
  }

  if (const char* envVar = std::getenv("XDG_SESSION_TYPE")) {
    if (std::string(envVar) == "wayland") {
      Logger::log(1, "%s: NOTE - Window does NOT appear on Wayland since we don't write to any buffer\n", __FUNCTION__);
      Logger::log(1, "%s: NOTE - Exiting now as there's literally nothing to see here\n", __FUNCTION__);
      w->cleanup();
      return 0;
    }
  }

  w->mainLoop();

  w->cleanup();
}
