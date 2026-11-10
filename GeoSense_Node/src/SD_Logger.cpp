#include "SD_Logger.h"

namespace GeoSense {

SDLogger::SDLogger(SPIClass& spi, uint8_t csPin) : _spi(spi), _csPin(csPin) {}

bool SDLogger::begin() {
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);

#if defined(ARDUINO_ARCH_STM32)
  SPI.setMOSI(PIN_SD_MOSI);
  SPI.setMISO(PIN_SD_MISO);
  SPI.setSCLK(PIN_SD_SCK);
#endif
  SPI.begin();
  _spi.begin();

  if (!SD.begin(4000000, _csPin)) {
    _initialized = false;
    _lastError = "sd begin failed";
    return false;
  }

  if (!createLogFile()) {
    _initialized = false;
    return false;
  }

  _initialized = writeHeaderIfNeeded();
  _lastError = _initialized ? "ok" : "sd header failed";
  return _initialized;
}

bool SDLogger::logFeatures(const SignalFeatures& features) {
  if (!_initialized) {
    _lastError = "sd not initialized";
    return false;
  }

  File file = SD.open(_fileName, FILE_WRITE);
  if (!file) {
    _lastError = "sd open failed";
    _initialized = false;
    return false;
  }

  file.print(features.timestampMs);
  file.print(',');
  file.print(NODE_ID);
  file.print(',');
  file.print(features.voltage, 6);
  file.print(',');
  file.print(features.rms, 6);
  file.print(',');
  file.print(features.peakFrequency, 3);
  file.print(',');
  file.print(features.peakAmplitude, 6);
  file.print(',');
  file.print(features.spectralEnergy, 6);
  file.print(',');
  file.print(features.mean, 6);
  file.print(',');
  file.println(features.variance, 6);
  file.close();

  _lastError = "ok";
  return true;
}

bool SDLogger::createLogFile() {
  for (uint16_t index = 0; index < 1000; ++index) {
    snprintf(_fileName, sizeof(_fileName), "%s%03u%s", LOG_FILE_PREFIX, index,
             LOG_FILE_EXTENSION);
    if (!SD.exists(_fileName)) {
      File file = SD.open(_fileName, FILE_WRITE);
      if (!file) {
        _lastError = "sd create failed";
        return false;
      }
      file.close();
      return true;
    }
  }

  _lastError = "sd log index full";
  return false;
}

bool SDLogger::writeHeaderIfNeeded() {
  File file = SD.open(_fileName, FILE_WRITE);
  if (!file) {
    return false;
  }
  file.println(CSV_HEADER);
  file.close();
  return true;
}

}  // namespace GeoSense
