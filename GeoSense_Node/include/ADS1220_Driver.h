#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "FFT_Process.h"
#include "config.h"

namespace GeoSense {

class ADS1220Driver {
 public:
  ADS1220Driver(SPIClass& spi, uint8_t csPin, uint8_t drdyPin);

  bool begin();
  bool isReady() const;
  bool readSample(GeophoneSample& sample);
  float rawToVoltage(int32_t rawCode) const;
  bool healthy() const { return _initialized; }
  const char* lastError() const { return _lastError; }

 private:
  enum class Command : uint8_t {
    PowerDown = 0x02,
    Reset = 0x06,
    StartSync = 0x08,
    ReadData = 0x10,
    ReadRegister = 0x20,
    WriteRegister = 0x40,
  };

  SPIClass& _spi;
  uint8_t _csPin;
  uint8_t _drdyPin;
  bool _initialized = false;
  const char* _lastError = "not initialized";

  void select();
  void deselect();
  void sendCommand(Command command);
  void writeRegister(uint8_t address, uint8_t value);
  uint8_t readRegister(uint8_t address);
  int32_t readRaw24();
  bool configure();
};

}  // namespace GeoSense
