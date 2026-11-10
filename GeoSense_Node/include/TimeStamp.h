#pragma once

#include <Arduino.h>

namespace GeoSense {

class TimeStamp {
 public:
  void begin();
  uint32_t nowMs() const;
  uint64_t nowUs() const;

  // Future GPS synchronization can update the epoch offset here.
  void setEpochOffsetMs(uint64_t offsetMs);
  uint64_t epochMs() const;

 private:
  uint64_t _epochOffsetMs = 0;
};

}  // namespace GeoSense
