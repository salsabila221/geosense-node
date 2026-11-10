#pragma once

#include <Arduino.h>

namespace GeoSense {

class PeriodicTimer {
 public:
  explicit PeriodicTimer(uint32_t intervalMs = 1000);

  bool due(uint32_t nowMs);
  void reset(uint32_t nowMs);
  void setInterval(uint32_t intervalMs);

 private:
  uint32_t _intervalMs;
  uint32_t _lastMs = 0;
};

template <typename T>
T clampValue(T value, T low, T high) {
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

const char* okFail(bool ok);
const char* runningIdle(bool running);
const char* successFail(bool success);

}  // namespace GeoSense
