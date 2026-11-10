#pragma once

#include <Arduino.h>
#include <LoRa_E220.h>
#include "config.h"

namespace GeoSense {

class LoRaNode {
 public:
  LoRaNode();

  bool begin();
  bool transmit(const String& payload);
  bool healthy() const { return _initialized; }
  const char* lastError() const { return _lastError; }

 private:
  LoRa_E220 _radio;
  bool _initialized = false;
  const char* _lastError = "not initialized";
};

}  // namespace GeoSense
