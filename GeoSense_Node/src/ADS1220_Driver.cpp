#include "ADS1220_Driver.h"

namespace GeoSense {
namespace {
constexpr uint32_t ADS_SPI_CLOCK_HZ = 2000000;
constexpr uint8_t ADS_REG_CONFIG0 = 0x00;
constexpr uint8_t ADS_REG_CONFIG1 = 0x01;
constexpr uint8_t ADS_REG_CONFIG2 = 0x02;
constexpr uint8_t ADS_REG_CONFIG3 = 0x03;
const SPISettings ADS_SPI_SETTINGS(ADS_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE1);

// Register values:
// CONFIG0: AIN0-AIN1, gain 128, PGA enabled.
// CONFIG1: 90 SPS, normal mode, continuous conversion.
// CONFIG2: internal 2.048 V reference, default filter and current sources off.
// CONFIG3: default DRDY behavior.
constexpr uint8_t ADS_CONFIG0 = 0x0E;
constexpr uint8_t ADS_CONFIG1 = 0x44;
constexpr uint8_t ADS_CONFIG2 = 0x00;
constexpr uint8_t ADS_CONFIG3 = 0x00;
}  // namespace

ADS1220Driver::ADS1220Driver(SPIClass& spi, uint8_t csPin, uint8_t drdyPin)
    : _spi(spi), _csPin(csPin), _drdyPin(drdyPin) {}

bool ADS1220Driver::begin() {
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
  pinMode(_drdyPin, INPUT);

  _spi.begin();
  delay(5);

  sendCommand(Command::Reset);
  delay(5);

  if (!configure()) {
    _initialized = false;
    return false;
  }

  sendCommand(Command::StartSync);
  _initialized = true;
  _lastError = "ok";
  return true;
}

bool ADS1220Driver::isReady() const {
  return _initialized && digitalRead(_drdyPin) == LOW;
}

bool ADS1220Driver::readSample(GeophoneSample& sample) {
  if (!_initialized) {
    _lastError = "ads not initialized";
    return false;
  }

  if (!isReady()) {
    return false;
  }

  const int32_t raw = readRaw24();
  sample.timestampMs = millis();
  sample.voltage = rawToVoltage(raw);
  return true;
}

float ADS1220Driver::rawToVoltage(int32_t rawCode) const {
  const float inputRange =
      ADS1220_REFERENCE_VOLTAGE / static_cast<float>(ADS1220_GAIN);
  return (static_cast<float>(rawCode) / ADS1220_FULL_SCALE_COUNTS) * inputRange;
}

void ADS1220Driver::select() {
  digitalWrite(_csPin, LOW);
}

void ADS1220Driver::deselect() {
  digitalWrite(_csPin, HIGH);
}

void ADS1220Driver::sendCommand(Command command) {
  _spi.beginTransaction(ADS_SPI_SETTINGS);
  select();
  _spi.transfer(static_cast<uint8_t>(command));
  deselect();
  _spi.endTransaction();
}

void ADS1220Driver::writeRegister(uint8_t address, uint8_t value) {
  _spi.beginTransaction(ADS_SPI_SETTINGS);
  select();
  _spi.transfer(static_cast<uint8_t>(Command::WriteRegister) |
                ((address & 0x03) << 2));
  _spi.transfer(value);
  deselect();
  _spi.endTransaction();
}

uint8_t ADS1220Driver::readRegister(uint8_t address) {
  _spi.beginTransaction(ADS_SPI_SETTINGS);
  select();
  _spi.transfer(static_cast<uint8_t>(Command::ReadRegister) |
                ((address & 0x03) << 2));
  const uint8_t value = _spi.transfer(0x00);
  deselect();
  _spi.endTransaction();
  return value;
}

int32_t ADS1220Driver::readRaw24() {
  _spi.beginTransaction(ADS_SPI_SETTINGS);
  select();
  _spi.transfer(static_cast<uint8_t>(Command::ReadData));
  uint32_t raw = static_cast<uint32_t>(_spi.transfer(0x00)) << 16;
  raw |= static_cast<uint32_t>(_spi.transfer(0x00)) << 8;
  raw |= static_cast<uint32_t>(_spi.transfer(0x00));
  deselect();
  _spi.endTransaction();

  if ((raw & 0x800000UL) != 0) {
    raw |= 0xFF000000UL;
  }
  return static_cast<int32_t>(raw);
}

bool ADS1220Driver::configure() {
  writeRegister(ADS_REG_CONFIG0, ADS_CONFIG0);
  writeRegister(ADS_REG_CONFIG1, ADS_CONFIG1);
  writeRegister(ADS_REG_CONFIG2, ADS_CONFIG2);
  writeRegister(ADS_REG_CONFIG3, ADS_CONFIG3);

  const bool ok = readRegister(ADS_REG_CONFIG0) == ADS_CONFIG0 &&
                  readRegister(ADS_REG_CONFIG1) == ADS_CONFIG1 &&
                  readRegister(ADS_REG_CONFIG2) == ADS_CONFIG2 &&
                  readRegister(ADS_REG_CONFIG3) == ADS_CONFIG3;

  if (!ok) {
    _lastError = "ads config verify failed";
  }
  return ok;
}

}  // namespace GeoSense
