#pragma once

#include <Arduino.h>
#include "config.h"

#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SH1106
#include <Adafruit_SH110X.h>
#else
#include <Adafruit_SSD1306.h>
#endif

#include "FFT_Process.h"

namespace GeoSense {

struct SystemStatus {
  bool adcOk = false;
  bool loraOk = false;
  bool sdOk = false;
  bool fftRunning = false;
  bool txAttempted = false;
  bool txSuccess = false;
  uint32_t packetCounter = 0;
};

class OLEDUI {
 public:
  OLEDUI();

  bool begin();
  void serviceSwitch();
  void update(const SignalFeatures& features, const SystemStatus& status);
  bool enabled() const { return _enabled; }
  bool healthy() const { return _initialized; }

 private:
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SH1106
  Adafruit_SH1106G _display;
#else
  Adafruit_SSD1306 _display;
#endif
  bool _initialized = false;
  bool _enabled = true;
  bool _lastSwitchState = true;

  bool beginDisplayAt(uint8_t address);
  bool readSwitchOn() const;
  void setPanelEnabled(bool enabled);
  void drawStatusLine(uint8_t y, const char* label, const char* value);
};

}  // namespace GeoSense
