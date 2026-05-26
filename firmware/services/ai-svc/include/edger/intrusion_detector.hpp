#pragma once

#include "edger/config.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace edger {

class IntrusionDetector {
 public:
  void SetPolygon(const std::vector<std::pair<float, float>>& normalized_polygon);
  void Reset();

  // Returns true when motion inside ROI exceeds threshold.
  bool ProcessGrayFrame(const std::vector<uint8_t>& gray, int width, int height);

  int last_motion_pixels() const { return last_motion_pixels_; }

 private:
  std::vector<std::pair<float, float>> polygon_;
  std::vector<uint8_t> previous_;
  int last_motion_pixels_ = 0;
  int width_ = 0;
  int height_ = 0;
};

}  // namespace edger
