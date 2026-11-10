#include "LoRa_Node.h"

namespace GeoSense {

LoRaNode::LoRaNode()
    : _radio(&Serial1, PIN_LORA_AUX, PIN_LORA_M0, PIN_LORA_M1) {}

bool LoRaNode::begin() {
  pinMode(PIN_LORA_M0, OUTPUT);
  pinMode(PIN_LORA_M1, OUTPUT);
  pinMode(PIN_LORA_AUX, INPUT);

  digitalWrite(PIN_LORA_M0, LOW);
  digitalWrite(PIN_LORA_M1, LOW);
  delay(LORA_MODE_SETTLE_MS);

#if defined(ARDUINO_ARCH_STM32)
  Serial1.setRx(PIN_LORA_RX);
  Serial1.setTx(PIN_LORA_TX);
#endif
  Serial1.begin(LORA_BAUD);

  if (!_radio.begin()) {
    _initialized = false;
    _lastError = "lora begin failed";
    return false;
  }

  _radio.setMode(MODE_0_NORMAL);
  _initialized = true;
  _lastError = "ok";
  return true;
}

bool LoRaNode::transmit(const String& payload) {
  if (!_initialized) {
    _lastError = "lora not initialized";
    return false;
  }

  ResponseStatus status = _radio.sendMessage(payload);
  const bool ok = status.code == E220_SUCCESS;
  _lastError = ok ? "ok" : "lora tx failed";
  return ok;
}

}  // namespace GeoSense
