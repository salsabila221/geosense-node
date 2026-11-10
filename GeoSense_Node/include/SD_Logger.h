#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "FFT_Process.h"
#include "config.h"

namespace GeoSense {

class SDLogger {
 public:
  SDLogger(SPIClass& spi, uint8_t csPin);

  bool begin();
  bool logFeatures(const SignalFeatures& features);
  bool healthy() const { return _initialized; }
  const char* fileName() const { return _fileName; }
  const char* lastError() const { return _lastError; }

 private:
  SPIClass& _spi;
  uint8_t _csPin;
  bool _initialized = false;
  char _fileName[24] = {0};
  const char* _lastError = "not initialized";

  bool createLogFile();
  bool writeHeaderIfNeeded();
};

}  // namespace GeoSense
