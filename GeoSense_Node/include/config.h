#pragma once

#include <Arduino.h>

#define GEOSENSE_OLED_SSD1306 1
#define GEOSENSE_OLED_SH1106 2

// Hardcode OLED type here when you build directly from config.h:
//   GEOSENSE_OLED_SSD1306 = bigger/current 0.96" SSD1306 OLED
//   GEOSENSE_OLED_SH1106  = smaller/different SH1106 OLED, often 1.3"
#ifndef GEOSENSE_OLED_DRIVER
#define GEOSENSE_OLED_DRIVER GEOSENSE_OLED_SSD1306
#endif

namespace GeoSense {

// ---------------------------------------------------------------------------
// Node identity
// ---------------------------------------------------------------------------
// Change these two values before uploading to each physical STM32 node.
//
// Examples:
//   NODE01 -> GEOSENSE_NODE_INDEX 0
//   NODE02 -> GEOSENSE_NODE_INDEX 1
//   NODE03 -> GEOSENSE_NODE_INDEX 2
//
// NODE_INDEX is used to stagger LoRa transmit timing so multiple nodes are
// less likely to transmit at exactly the same moment.
#define GEOSENSE_NODE_ID "NODE01"
#define GEOSENSE_NODE_INDEX 0

constexpr const char* NODE_ID = GEOSENSE_NODE_ID;
constexpr uint8_t NODE_INDEX = GEOSENSE_NODE_INDEX;
constexpr uint32_t SERIAL_BAUD = 115200;

// ---------------------------------------------------------------------------
// Sampling and signal processing
// ADS1220 is configured for 90 SPS, so the scheduler targets the same rate.
// ---------------------------------------------------------------------------
constexpr float ADC_SAMPLE_RATE_HZ = 90.0F;
constexpr uint16_t WAVEFORM_BUFFER_SIZE = 128;
constexpr uint32_t SAMPLE_INTERVAL_US =
    static_cast<uint32_t>(1000000.0F / ADC_SAMPLE_RATE_HZ);

// ---------------------------------------------------------------------------
// STA/LTA event trigger
// ---------------------------------------------------------------------------
// The current ADS1220 configuration is 90 SPS, so a 1-second STA window is
// 90 samples and a 30-second LTA window is 2700 samples. If the ADC is later
// changed to 1000 SPS, these constants will scale automatically.
constexpr bool STA_LTA_TRIGGER_ENABLED = true;
constexpr float STA_WINDOW_SECONDS = 1.0F;
constexpr float LTA_WINDOW_SECONDS = 30.0F;
constexpr size_t STA_WINDOW_SAMPLES =
    static_cast<size_t>((ADC_SAMPLE_RATE_HZ * STA_WINDOW_SECONDS) + 0.5F);
constexpr size_t LTA_WINDOW_SAMPLES =
    static_cast<size_t>((ADC_SAMPLE_RATE_HZ * LTA_WINDOW_SECONDS) + 0.5F);
static_assert(STA_WINDOW_SAMPLES > 0, "STA window must contain samples");
static_assert(LTA_WINDOW_SAMPLES > STA_WINDOW_SAMPLES,
              "LTA window must be longer than STA window");
constexpr float STA_LTA_THRESHOLD = 3.0F;
constexpr float STA_LTA_RESET_RATIO = 1.5F;
constexpr float STA_LTA_EPSILON = 1.0e-12F;
constexpr float STA_LTA_DC_ALPHA = 0.01F;

// true = keep SD logs for every processed feature window.
// false = only log windows that pass the STA/LTA trigger.
constexpr bool STA_LTA_LOG_ALL_TO_SD = true;

// Internal ADS1220 reference with PGA gain 128.
constexpr float ADS1220_REFERENCE_VOLTAGE = 2.048F;
constexpr uint8_t ADS1220_GAIN = 128;
constexpr float ADS1220_FULL_SCALE_COUNTS = 8388607.0F;

// ---------------------------------------------------------------------------
// ADS1220 on SPI2
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_ADS_CS = PB12;
constexpr uint8_t PIN_ADS_SCLK = PB13;
constexpr uint8_t PIN_ADS_MISO = PB14;
constexpr uint8_t PIN_ADS_MOSI = PB15;
constexpr uint8_t PIN_ADS_DRDY = PA0;

// ---------------------------------------------------------------------------
// OLED SSD1306 on I2C1
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_OLED_SCL = PB6;
constexpr uint8_t PIN_OLED_SDA = PB7;
constexpr uint8_t PIN_OLED_SWITCH = PA1;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t OLED_FALLBACK_ADDRESS = 0x3D;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_DRIVER = GEOSENSE_OLED_DRIVER;

// OLED switch wiring: PA1 open/high keeps OLED on; PA1 connected to GND turns
// OLED off. Keep this enabled after display hardware is verified.
constexpr bool OLED_SWITCH_ENABLED = true;

// Default wiring: PA1 is held HIGH by the internal pull-up, so the OLED turns
// on when the switch is open and turns off when PA1 is connected to GND.
constexpr bool OLED_SWITCH_ACTIVE_LOW = false;

// ---------------------------------------------------------------------------
// SD card on SPI1
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_SD_CS = PA4;
constexpr uint8_t PIN_SD_SCK = PA5;
constexpr uint8_t PIN_SD_MISO = PA6;
constexpr uint8_t PIN_SD_MOSI = PA7;

// ---------------------------------------------------------------------------
// EByte E220 LoRa module on UART1
// ---------------------------------------------------------------------------
constexpr uint8_t PIN_LORA_TX = PA9;
constexpr uint8_t PIN_LORA_RX = PA10;
// Current wiring: M0 on PA8/A8, M1 on PB0/B0, AUX on PB1/B1.
constexpr uint8_t PIN_LORA_AUX = PB1;
constexpr uint8_t PIN_LORA_M0 = PA8;
constexpr uint8_t PIN_LORA_M1 = PB0;
constexpr uint32_t LORA_BAUD = 9600;
constexpr uint32_t LORA_MODE_SETTLE_MS = 50;
constexpr uint32_t LORA_AUX_READY_TIMEOUT_MS = 1000;
constexpr uint32_t LORA_TX_GUARD_MS = 40;

// ---------------------------------------------------------------------------
// Main task cadence
// ---------------------------------------------------------------------------
constexpr uint32_t OLED_UPDATE_INTERVAL_MS = 250;
constexpr uint32_t HEALTH_RETRY_INTERVAL_MS = 5000;
constexpr uint32_t LORA_HEARTBEAT_INTERVAL_MS = 5000;
constexpr bool LORA_HEARTBEAT_ENABLED = false;
// true while testing the LoRa link: every feature window is transmitted.
// false for deployment: LoRa transmits only when STA/LTA trigger is active.
constexpr bool LORA_SEND_ALL_FEATURES_DURING_TEST = true;
constexpr uint32_t LORA_NODE_SLOT_MS = 250;
constexpr uint32_t LORA_NODE_JITTER_MS = 80;
constexpr uint8_t LORA_MAX_TX_RETRIES = 2;
constexpr uint32_t LORA_RETRY_DELAY_MS = 200;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
constexpr const char* CSV_HEADER =
    "timestamp,node,voltage,rms,peak_frequency,peak_amplitude,"
    "spectral_energy,mean,variance";
// Arduino SD.h is based on a small FAT library that is happiest with 8.3
// filenames. Keep the basename <= 8 characters: GS000.CSV, GS001.CSV, ...
constexpr const char* LOG_FILE_PREFIX = "/GS";
constexpr const char* LOG_FILE_EXTENSION = ".CSV";

// ---------------------------------------------------------------------------
// Extension points reserved for future firmware generations.
// ---------------------------------------------------------------------------
constexpr bool FEATURE_GPS_SYNC_ENABLED = false;
constexpr bool FEATURE_TDOA_ENABLED = false;
constexpr bool FEATURE_ML_INFERENCE_ENABLED = false;
constexpr bool FEATURE_MQTT_ENABLED = false;
constexpr bool FEATURE_OTA_ENABLED = false;

}  // namespace GeoSense