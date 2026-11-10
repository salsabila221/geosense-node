#include "JSON_Packet.h"

namespace GeoSense {
namespace {

void formatFloat(float value, uint8_t precision, char* output,
                 size_t outputSize) {
  if (output == nullptr || outputSize == 0) {
    return;
  }

  dtostrf(value, 0, precision, output);
  output[outputSize - 1] = '\0';
}

}  // namespace

String JSONPacket::build(const NodePacket& packet) {
  char voltage[20];
  char rms[20];
  char peakFrequency[20];
  char peakAmplitude[20];
  char spectralEnergy[20];
  char mean[20];
  char variance[20];

  formatFloat(packet.features.voltage, 6, voltage, sizeof(voltage));
  formatFloat(packet.features.rms, 6, rms, sizeof(rms));
  formatFloat(packet.features.peakFrequency, 3, peakFrequency,
              sizeof(peakFrequency));
  formatFloat(packet.features.peakAmplitude, 6, peakAmplitude,
              sizeof(peakAmplitude));
  formatFloat(packet.features.spectralEnergy, 6, spectralEnergy,
              sizeof(spectralEnergy));
  formatFloat(packet.features.mean, 6, mean, sizeof(mean));
  formatFloat(packet.features.variance, 6, variance, sizeof(variance));

  char buffer[384];
  snprintf(buffer, sizeof(buffer),
           "{\"node\":\"%s\",\"packet\":%lu,\"timestamp\":%lu,"
           "\"voltage\":%s,\"rms\":%s,\"peak_frequency\":%s,"
           "\"peak_amplitude\":%s,\"spectral_energy\":%s,"
           "\"mean\":%s,\"variance\":%s}",
           packet.node, static_cast<unsigned long>(packet.packet),
           static_cast<unsigned long>(packet.timestamp), voltage, rms,
           peakFrequency, peakAmplitude, spectralEnergy, mean, variance);
  return String(buffer);
}

String JSONPacket::buildRadio(const NodePacket& packet) {
  char voltage[16];
  char rms[16];
  char peakFrequency[16];
  char peakAmplitude[16];
  char spectralEnergy[16];
  char mean[16];
  char variance[16];

  formatFloat(packet.features.voltage, 5, voltage, sizeof(voltage));
  formatFloat(packet.features.rms, 5, rms, sizeof(rms));
  formatFloat(packet.features.peakFrequency, 2, peakFrequency,
              sizeof(peakFrequency));
  formatFloat(packet.features.peakAmplitude, 5, peakAmplitude,
              sizeof(peakAmplitude));
  formatFloat(packet.features.spectralEnergy, 5, spectralEnergy,
              sizeof(spectralEnergy));
  formatFloat(packet.features.mean, 5, mean, sizeof(mean));
  formatFloat(packet.features.variance, 5, variance, sizeof(variance));

  char buffer[160];
  snprintf(buffer, sizeof(buffer),
           "{\"n\":\"%s\",\"p\":%lu,\"t\":%lu,\"d\":[%s,%s,%s,%s,%s,%s,%s]}",
           packet.node, static_cast<unsigned long>(packet.packet),
           static_cast<unsigned long>(packet.timestamp), voltage, rms,
           peakFrequency, peakAmplitude, spectralEnergy, mean, variance);
  return String(buffer);
}

}  // namespace GeoSense
