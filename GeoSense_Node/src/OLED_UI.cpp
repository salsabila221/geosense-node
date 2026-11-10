#include "OLED_UI.h"

#include <Wire.h>
#include "Utility.h"

namespace GeoSense {
namespace {
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SH1106
constexpr uint16_t OLED_WHITE = SH110X_WHITE;
#else
constexpr uint16_t OLED_WHITE = SSD1306_WHITE;
#endif

void formatFloat(float value, uint8_t precision, char* output,
                 size_t outputSize) {
  if (output == nullptr || outputSize == 0) {
    return;
  }

  dtostrf(value, 0, precision, output);
  output[outputSize - 1] = '\0';
}

}  // namespace

OLEDUI::OLEDUI()
    : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {}

bool OLEDUI::begin() {
  pinMode(PIN_OLED_SWITCH, INPUT_PULLUP);

#if defined(ARDUINO_ARCH_STM32)
  Wire.setSDA(PIN_OLED_SDA);
  Wire.setSCL(PIN_OLED_SCL);
#endif
  Wire.begin();

  _enabled = true;
  _lastSwitchState = _enabled;

  if (!beginDisplayAt(OLED_ADDRESS) && !beginDisplayAt(OLED_FALLBACK_ADDRESS)) {
    _initialized = false;
    return false;
  }

  _initialized = true;
  _display.clearDisplay();
  _display.setTextColor(OLED_WHITE);
  _display.setTextSize(1);
  setPanelEnabled(true);
  _display.setCursor(0, 0);
  _display.println(F("GeoSense"));
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SH1106
  _display.println(F("OLED SH1106 OK"));
#else
  _display.println(F("OLED OK"));
#endif
  _display.display();
  return true;
}

void OLEDUI::serviceSwitch() {
  if (!_initialized || !OLED_SWITCH_ENABLED) {
    return;
  }

  const bool switchState = readSwitchOn();
  if (switchState != _lastSwitchState) {
    _lastSwitchState = switchState;
    setPanelEnabled(switchState);
  }
}

void OLEDUI::update(const SignalFeatures& features,
                    const SystemStatus& status) {
  if (!_initialized || !_enabled) {
    return;
  }

  char line[24];
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print(NODE_ID);
  _display.setCursor(76, 0);
  _display.print("LORA:");
  _display.print(okFail(status.loraOk));
  _display.drawLine(0, 10, 127, 10, OLED_WHITE);

  if (status.adcOk) {
    char voltage[16];
    formatFloat(features.voltage, 5, voltage, sizeof(voltage));
    snprintf(line, sizeof(line), "%s V", voltage);
    drawStatusLine(15, "ADC", line);
  } else {
    drawStatusLine(15, "ADC", "FAIL");
  }

  if (status.packetCounter > 0) {
    char peak[16];
    formatFloat(features.peakFrequency, 2, peak, sizeof(peak));
    snprintf(line, sizeof(line), "%s Hz", peak);
    drawStatusLine(25, "Peak", line);

    char rms[16];
    formatFloat(features.rms, 5, rms, sizeof(rms));
    drawStatusLine(35, "RMS", rms);
  } else {
    drawStatusLine(25, "Peak", "WAIT");
    drawStatusLine(35, "RMS", "WAIT");
  }

  _display.drawLine(0, 45, 127, 45, OLED_WHITE);
  drawStatusLine(49, "SD", okFail(status.sdOk));
  drawStatusLine(57, "FFT", status.fftRunning ? "RUN" : "IDLE");

  _display.setCursor(72, 57);
  _display.print("TX:");
  if (!status.txAttempted) {
    _display.print("WAIT");
  } else {
    _display.print(status.txSuccess ? "OK" : "FAIL");
  }
  _display.display();
}

bool OLEDUI::beginDisplayAt(uint8_t address) {
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SH1106
  return _display.begin(address, true);
#else
  return _display.begin(SSD1306_SWITCHCAPVCC, address);
#endif
}

bool OLEDUI::readSwitchOn() const {
  const uint8_t level = digitalRead(PIN_OLED_SWITCH);
  return OLED_SWITCH_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}

void OLEDUI::setPanelEnabled(bool enabled) {
  _enabled = enabled;
  if (!_initialized) {
    return;
  }

  if (_enabled) {
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SSD1306
    _display.ssd1306_command(SSD1306_DISPLAYON);
#endif
  } else {
    _display.clearDisplay();
    _display.display();
#if GEOSENSE_OLED_DRIVER == GEOSENSE_OLED_SSD1306
    _display.ssd1306_command(SSD1306_DISPLAYOFF);
#endif
  }
}

void OLEDUI::drawStatusLine(uint8_t y, const char* label, const char* value) {
  _display.setCursor(0, y);
  _display.print(label);
  _display.print("  : ");
  _display.print(value);
}

}  // namespace GeoSense
