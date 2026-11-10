#pragma once

#include <Arduino.h>
#include "config.h"

namespace GeoSense {

struct StaLtaState {
  bool ready = false;
  bool active = false;
  float sta = 0.0F;
  float lta = 0.0F;
  float ratio = 0.0F;
};

class StaLtaTrigger {
 public:
  StaLtaState update(float voltage);
  const StaLtaState& state() const { return _state; }

 private:
  float _staBuffer[STA_WINDOW_SAMPLES] = {0.0F};
  float _ltaBuffer[LTA_WINDOW_SAMPLES] = {0.0F};
  size_t _staIndex = 0;
  size_t _ltaIndex = 0;
  size_t _staFilled = 0;
  size_t _ltaFilled = 0;
  float _staSum = 0.0F;
  float _ltaSum = 0.0F;
  float _dcEstimate = 0.0F;
  bool _dcInitialized = false;
  StaLtaState _state;

  float denoise(float voltage);
  void updateWindow(float energy, float* buffer, size_t windowSize,
                    size_t& index, size_t& filled, float& sum);
};

}  // namespace GeoSense
