#include <Arduino.h>
#include <SPI.h>

#include "ADS1220_Driver.h"
#include "FFT_Process.h"
#include "JSON_Packet.h"
#include "LoRa_Node.h"
#include "OLED_UI.h"
#include "SD_Logger.h"
#include "StaLta_Trigger.h"
#include "TimeStamp.h"
#include "Utility.h"
#include "config.h"

using namespace GeoSense;

SPIClass adsSpi(PIN_ADS_MOSI, PIN_ADS_MISO, PIN_ADS_SCLK);
SPIClass sdSpi(PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_SCK);

ADS1220Driver adc(adsSpi, PIN_ADS_CS, PIN_ADS_DRDY);
SDLogger sdLogger(sdSpi, PIN_SD_CS);
LoRaNode lora;
OLEDUI oled;
FFTProcess fft;
StaLtaTrigger staLta;
TimeStamp timeStamp;

PeriodicTimer oledTimer(OLED_UPDATE_INTERVAL_MS);
PeriodicTimer healthRetryTimer(HEALTH_RETRY_INTERVAL_MS);
PeriodicTimer loraHeartbeatTimer(LORA_HEARTBEAT_INTERVAL_MS);

SystemStatus status;
SignalFeatures latestFeatures;

float waveformBuffer[WAVEFORM_BUFFER_SIZE] = {0.0F};
size_t waveformIndex = 0;
uint32_t packetCounter = 0;
uint32_t nextSampleUs = 0;
String pendingRadioPayload;
bool pendingRadioTx = false;
uint32_t pendingRadioTxAtMs = 0;
uint8_t pendingRadioRetryCount = 0;

void initializeHardware();
void retryFailedHardware();
void serviceAcquisition();
void processWaveform(uint32_t timestampMs);
void publishFeatures(const SignalFeatures& features, bool radioEnabled);
void scheduleRadioPayload(const String& radioPayload);
void servicePendingRadioTx(uint32_t nowMs);
void publishLoRaHeartbeat(uint32_t nowMs);

void setup() {
  Serial.begin(SERIAL_BAUD);
  timeStamp.begin();
  initializeHardware();
  nextSampleUs = micros();
}

void loop() {
  oled.serviceSwitch();
  serviceAcquisition();

  const uint32_t nowMs = millis();
  if (healthRetryTimer.due(nowMs)) {
    retryFailedHardware();
  }

  if (LORA_HEARTBEAT_ENABLED && loraHeartbeatTimer.due(nowMs)) {
    publishLoRaHeartbeat(nowMs);
  }

  servicePendingRadioTx(nowMs);

  if (oledTimer.due(nowMs)) {
    oled.update(latestFeatures, status);
  }
}

void initializeHardware() {
  status.adcOk = adc.begin();
  status.sdOk = sdLogger.begin();
  status.loraOk = lora.begin();
  oled.begin();

  Serial.println(F("GeoSense node boot"));
  Serial.print(F("ADC: "));
  Serial.println(okFail(status.adcOk));
  Serial.print(F("SD : "));
  Serial.println(okFail(status.sdOk));
  if (!status.sdOk) {
    Serial.print(F("SD error: "));
    Serial.println(sdLogger.lastError());
  }
  Serial.print(F("LoRa: "));
  Serial.println(okFail(status.loraOk));

  randomSeed(micros() ^ (static_cast<uint32_t>(NODE_INDEX) * 2654435761UL));
}

void retryFailedHardware() {
  if (!status.adcOk) {
    status.adcOk = adc.begin();
  }

  if (!status.sdOk) {
    status.sdOk = sdLogger.begin();
  }

  if (!status.loraOk) {
    status.loraOk = lora.begin();
  }
}

void serviceAcquisition() {
  const uint32_t nowUs = micros();
  if (static_cast<int32_t>(nowUs - nextSampleUs) < 0) {
    return;
  }
  nextSampleUs += SAMPLE_INTERVAL_US;

  if (static_cast<int32_t>(nowUs - nextSampleUs) > 0) {
    nextSampleUs = nowUs + SAMPLE_INTERVAL_US;
  }

  GeophoneSample sample;
  if (!adc.readSample(sample)) {
    status.adcOk = adc.healthy();
    return;
  }

  status.adcOk = true;
  staLta.update(sample.voltage);
  waveformBuffer[waveformIndex++] = sample.voltage;

  if (waveformIndex >= WAVEFORM_BUFFER_SIZE) {
    waveformIndex = 0;
    processWaveform(sample.timestampMs);
  }
}

void processWaveform(uint32_t timestampMs) {
  status.fftRunning = true;
  latestFeatures = fft.analyze(waveformBuffer, WAVEFORM_BUFFER_SIZE,
                               timestampMs);
  status.fftRunning = false;

  const bool radioEnabled =
      LORA_SEND_ALL_FEATURES_DURING_TEST || !STA_LTA_TRIGGER_ENABLED ||
      staLta.state().active;
  publishFeatures(latestFeatures, radioEnabled);
}

void publishFeatures(const SignalFeatures& features, bool radioEnabled) {
  NodePacket packet;
  packet.packet = ++packetCounter;
  packet.timestamp = features.timestampMs;
  packet.features = features;
  status.packetCounter = packetCounter;

  const String payload = JSONPacket::build(packet);
  const String radioPayload = JSONPacket::buildRadio(packet) + '\n';

  if (STA_LTA_LOG_ALL_TO_SD || radioEnabled) {
    status.sdOk = sdLogger.logFeatures(features);
  }

  if (radioEnabled) {
    scheduleRadioPayload(radioPayload);
  }

  Serial.println(payload);
}

void scheduleRadioPayload(const String& radioPayload) {
  if (pendingRadioTx) {
    return;
  }

  pendingRadioPayload = radioPayload;
  pendingRadioTx = true;
  pendingRadioRetryCount = 0;
  pendingRadioTxAtMs =
      millis() + (static_cast<uint32_t>(NODE_INDEX) * LORA_NODE_SLOT_MS) +
      random(LORA_NODE_JITTER_MS + 1);
}

void servicePendingRadioTx(uint32_t nowMs) {
  if (!pendingRadioTx ||
      static_cast<int32_t>(nowMs - pendingRadioTxAtMs) < 0) {
    return;
  }

  status.txAttempted = true;
  status.txSuccess = lora.transmit(pendingRadioPayload);
  status.loraOk = lora.healthy();
  Serial.print(F("LoRa TX "));
  Serial.println(status.txSuccess ? F("OK") : F("FAIL"));
  if (!status.txSuccess) {
    Serial.print(F("LoRa error: "));
    Serial.println(lora.lastError());
  }

  if (status.txSuccess || pendingRadioRetryCount >= LORA_MAX_TX_RETRIES) {
    pendingRadioPayload = "";
    pendingRadioTx = false;
    pendingRadioRetryCount = 0;
    return;
  }

  ++pendingRadioRetryCount;
  pendingRadioTxAtMs = nowMs + LORA_RETRY_DELAY_MS;
}

void publishLoRaHeartbeat(uint32_t nowMs) {
  if (!lora.healthy()) {
    status.loraOk = false;
    status.txSuccess = false;
    return;
  }

  char heartbeat[80];
  snprintf(heartbeat, sizeof(heartbeat),
           "{\"n\":\"%s\",\"type\":\"hb\",\"t\":%lu}\n", NODE_ID,
           static_cast<unsigned long>(nowMs));

  scheduleRadioPayload(String(heartbeat));
  Serial.println(F("LoRa heartbeat queued"));
}
