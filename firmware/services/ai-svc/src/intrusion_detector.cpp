#include "edger/intrusion_detector.hpp"

#include "edger/geometry/polygon.hpp"

#include <algorithm>

namespace edger {

void IntrusionDetector::SetPolygon(
    const std::vector<std::pair<float, float>>& normalized_polygon) {
  polygon_ = normalized_polygon;
  Reset();
}

void IntrusionDetector::Reset() {
  previous_.clear();
  width_ = 0;
  height_ = 0;
  last_motion_pixels_ = 0;
}

bool IntrusionDetector::ProcessGrayFrame(const std::vector<uint8_t>& gray,
                                         int width, int height) {
  if (polygon_.size() < 3 || width <= 0 || height <= 0 ||
      static_cast<int>(gray.size()) != width * height) {
    return false;
  }

  if (previous_.empty() || width_ != width || height_ != height) {
    previous_ = gray;
    width_ = width;
    height_ = height;
    last_motion_pixels_ = 0;
    return false;
  }

  std::vector<std::pair<float, float>> pixel_polygon;
  pixel_polygon.reserve(polygon_.size());
  for (const auto& [nx, ny] : polygon_) {
    pixel_polygon.emplace_back(nx * width, ny * height);
  }

  int motion_pixels = 0;
  const int threshold = 25;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (!geometry::PointInPolygon(static_cast<float>(x + 0.5f),
                                    static_cast<float>(y + 0.5f),
                                    pixel_polygon)) {
        continue;
      }

      const int idx = y * width + x;
      const int diff = std::abs(static_cast<int>(gray[idx]) -
                                static_cast<int>(previous_[idx]));
      if (diff > threshold) {
        ++motion_pixels;
      }
    }
  }

  previous_ = gray;
  last_motion_pixels_ = motion_pixels;

  const int min_motion = std::max(50, (width * height) / 500);
  return motion_pixels >= min_motion;
}

}  // namespace edger
