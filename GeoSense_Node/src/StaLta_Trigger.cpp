#include "StaLta_Trigger.h"

namespace GeoSense {

StaLtaState StaLtaTrigger::update(float voltage) {
  const float filtered = denoise(voltage);
  const float energy = filtered * filtered;

  updateWindow(energy, _staBuffer, STA_WINDOW_SAMPLES, _staIndex, _staFilled,
               _staSum);
  updateWindow(energy, _ltaBuffer, LTA_WINDOW_SAMPLES, _ltaIndex, _ltaFilled,
               _ltaSum);

  _state.ready =
      _staFilled >= STA_WINDOW_SAMPLES && _ltaFilled >= LTA_WINDOW_SAMPLES;
  _state.sta = _staFilled > 0 ? _staSum / static_cast<float>(_staFilled)
                              : 0.0F;
  _state.lta = _ltaFilled > 0 ? _ltaSum / static_cast<float>(_ltaFilled)
                              : 0.0F;
  _state.ratio =
      _state.lta > STA_LTA_EPSILON ? _state.sta / _state.lta : 0.0F;

  if (_state.ready) {
    if (!_state.active && _state.ratio >= STA_LTA_THRESHOLD) {
      _state.active = true;
    } else if (_state.active && _state.ratio <= STA_LTA_RESET_RATIO) {
      _state.active = false;
    }
  } else {
    _state.active = false;
  }

  return _state;
}

float StaLtaTrigger::denoise(float voltage) {
  if (!_dcInitialized) {
    _dcEstimate = voltage;
    _dcInitialized = true;
  }

  _dcEstimate += STA_LTA_DC_ALPHA * (voltage - _dcEstimate);
  return voltage - _dcEstimate;
}

void StaLtaTrigger::updateWindow(float energy, float* buffer,
                                 size_t windowSize, size_t& index,
                                 size_t& filled, float& sum) {
  if (filled >= windowSize) {
    sum -= buffer[index];
  } else {
    ++filled;
  }

  buffer[index] = energy;
  sum += energy;
  index = (index + 1) % windowSize;
}

}  // namespace GeoSense
