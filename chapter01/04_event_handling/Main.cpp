#include <memory>
#include <cstdlib>

#include "Window.h"
#include "Logger.h"

int main(int argc, char *argv[]) {
  std::unique_ptr<Window> w = std::make_unique<Window>();

  if (!w->init(640, 480, "Event Test Window")) {
    Logger::log(1, "%s error: Window init error\n", __FUNCTION__);
    return -1;
  }

  if (const char* envVar = std::getenv("XDG_SESSION_TYPE")) {
    if (std::string(envVar) == "wayland") {
      Logger::log(1, "%s: NOTE - only min/max/close events will be seen on Wayland\n", __FUNCTION__);
    }
  }

  w->mainLoop();

  w->cleanup();

  return 0;
}
