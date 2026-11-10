#pragma once

#include <Arduino.h>
#include "FFT_Process.h"
#include "config.h"

namespace GeoSense {

struct NodePacket {
  const char* node = NODE_ID;
  uint32_t packet = 0;
  uint32_t timestamp = 0;
  SignalFeatures features;
};

class JSONPacket {
 public:
  static String build(const NodePacket& packet);
  static String buildRadio(const NodePacket& packet);
};

}  // namespace GeoSense
