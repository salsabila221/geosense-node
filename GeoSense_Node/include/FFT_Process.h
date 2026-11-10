#pragma once

#include <Arduino.h>
#include "config.h"

namespace GeoSense {

struct GeophoneSample {
  uint32_t timestampMs = 0;
  float voltage = 0.0F;
};

struct SignalFeatures {
  uint32_t timestampMs = 0;
  float voltage = 0.0F;
  float rms = 0.0F;
  float peakFrequency = 0.0F;
  float peakAmplitude = 0.0F;
  float spectralEnergy = 0.0F;
  float mean = 0.0F;
  float variance = 0.0F;
};

class FFTProcess {
 public:
  explicit FFTProcess(float sampleRateHz = ADC_SAMPLE_RATE_HZ);

  SignalFeatures analyze(const float* samples, size_t sampleCount,
                         uint32_t timestampMs) const;

 private:
  float _sampleRateHz;

  static float calculateMean(const float* samples, size_t sampleCount);
  static float calculateVariance(const float* samples, size_t sampleCount,
                                 float mean);
  static float calculateRms(const float* samples, size_t sampleCount);
};

}  // namespace GeoSense
