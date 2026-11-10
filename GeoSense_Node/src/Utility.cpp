#include "Utility.h"

namespace GeoSense {

PeriodicTimer::PeriodicTimer(uint32_t intervalMs) : _intervalMs(intervalMs) {}

bool PeriodicTimer::due(uint32_t nowMs) {
  if (nowMs - _lastMs >= _intervalMs) {
    _lastMs = nowMs;
    return true;
  }
  return false;
}

void PeriodicTimer::reset(uint32_t nowMs) {
  _lastMs = nowMs;
}

void PeriodicTimer::setInterval(uint32_t intervalMs) {
  _intervalMs = intervalMs;
}

const char* okFail(bool ok) {
  return ok ? "OK" : "FAIL";
}

const char* runningIdle(bool running) {
  return running ? "RUNNING" : "IDLE";
}

const char* successFail(bool success) {
  return success ? "SUCCESS" : "FAIL";
}

}  // namespace GeoSense
