#include "edger/geometry/polygon.hpp"

#include <gtest/gtest.h>

TEST(Week3Polygon, InsideSquare) {
  const std::vector<edger::geometry::Point> square = {
      {0.1f, 0.1f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f}};
  EXPECT_TRUE(edger::geometry::PointInPolygon(0.5f, 0.5f, square));
  EXPECT_FALSE(edger::geometry::PointInPolygon(0.05f, 0.5f, square));
}
