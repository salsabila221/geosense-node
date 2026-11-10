#include "TimeStamp.h"

namespace GeoSense {

void TimeStamp::begin() {
  _epochOffsetMs = 0;
}

uint32_t TimeStamp::nowMs() const {
  return millis();
}

uint64_t TimeStamp::nowUs() const {
  return micros();
}

void TimeStamp::setEpochOffsetMs(uint64_t offsetMs) {
  _epochOffsetMs = offsetMs;
}

uint64_t TimeStamp::epochMs() const {
  return _epochOffsetMs + millis();
}

}  // namespace GeoSense
