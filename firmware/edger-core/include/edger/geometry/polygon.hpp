#pragma once

#include <utility>
#include <vector>

namespace edger::geometry {

using Point = std::pair<float, float>;

bool PointInPolygon(float x, float y, const std::vector<Point>& polygon);

}  // namespace edger::geometry
