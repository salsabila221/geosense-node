#include "FFT_Process.h"

#include <math.h>

namespace GeoSense {
namespace {
constexpr float TWO_PI_F = 6.2831853071795864769F;
}

FFTProcess::FFTProcess(float sampleRateHz) : _sampleRateHz(sampleRateHz) {}

SignalFeatures FFTProcess::analyze(const float* samples, size_t sampleCount,
                                   uint32_t timestampMs) const {
  SignalFeatures features;
  features.timestampMs = timestampMs;

  if (samples == nullptr || sampleCount == 0) {
    return features;
  }

  features.voltage = samples[sampleCount - 1];
  features.mean = calculateMean(samples, sampleCount);
  features.variance = calculateVariance(samples, sampleCount, features.mean);
  features.rms = calculateRms(samples, sampleCount);

  float peakMagnitude = 0.0F;
  size_t peakBin = 0;
  float spectralEnergy = 0.0F;

  // A compact DFT is acceptable at 90 SPS and 128 samples. It keeps the first
  // firmware generation dependency-light while preserving the FFT module API.
  const size_t halfBins = sampleCount / 2;
  for (size_t k = 1; k < halfBins; ++k) {
    float real = 0.0F;
    float imag = 0.0F;

    for (size_t n = 0; n < sampleCount; ++n) {
      const float centered = samples[n] - features.mean;
      const float angle =
          TWO_PI_F * static_cast<float>(k) * static_cast<float>(n) /
          static_cast<float>(sampleCount);
      real += centered * cosf(angle);
      imag -= centered * sinf(angle);
    }

    const float magnitude = sqrtf(real * real + imag * imag) /
                            static_cast<float>(sampleCount);
    spectralEnergy += magnitude * magnitude;

    if (magnitude > peakMagnitude) {
      peakMagnitude = magnitude;
      peakBin = k;
    }
  }

  features.peakFrequency =
      static_cast<float>(peakBin) * _sampleRateHz /
      static_cast<float>(sampleCount);
  features.peakAmplitude = peakMagnitude;
  features.spectralEnergy = spectralEnergy;

  return features;
}

float FFTProcess::calculateMean(const float* samples, size_t sampleCount) {
  float sum = 0.0F;
  for (size_t i = 0; i < sampleCount; ++i) {
    sum += samples[i];
  }
  return sum / static_cast<float>(sampleCount);
}

float FFTProcess::calculateVariance(const float* samples, size_t sampleCount,
                                    float mean) {
  float sum = 0.0F;
  for (size_t i = 0; i < sampleCount; ++i) {
    const float delta = samples[i] - mean;
    sum += delta * delta;
  }
  return sum / static_cast<float>(sampleCount);
}

float FFTProcess::calculateRms(const float* samples, size_t sampleCount) {
  float sum = 0.0F;
  for (size_t i = 0; i < sampleCount; ++i) {
    sum += samples[i] * samples[i];
  }
  return sqrtf(sum / static_cast<float>(sampleCount));
}

}  // namespace GeoSense
