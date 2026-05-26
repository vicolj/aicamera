#include "edger/geometry/polygon.hpp"

#include <cstddef>

namespace edger::geometry {

bool PointInPolygon(float x, float y, const std::vector<Point>& polygon) {
  if (polygon.size() < 3) {
    return false;
  }

  bool inside = false;
  for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
    const auto [xi, yi] = polygon[i];
    const auto [xj, yj] = polygon[j];
    const bool intersect =
        ((yi > y) != (yj > y)) &&
        (x < (xj - xi) * (y - yi) / ((yj - yi) + 1e-6f) + xi);
    if (intersect) {
      inside = !inside;
    }
  }

  return inside;
}

}  // namespace edger::geometry
