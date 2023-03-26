#include "Timer.h"
#include "Logger.h"

void Timer::start() {
  if (mRunning) {
    Logger::log(1, "%s error: timer already running\n", __FUNCTION__);
    return;
  }

  mRunning = true;
  mStartTime = std::chrono::steady_clock::now();
}

float Timer::stop() {
  if (!mRunning) {
    Logger::log(1, "%s error: timer not running\n", __FUNCTION__);
    return 0;
  }
  mRunning = false;

  auto stopTime = std::chrono::steady_clock::now();
  float timerMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - mStartTime).count() / 1000.0f;

  return timerMilliSeconds;
}
