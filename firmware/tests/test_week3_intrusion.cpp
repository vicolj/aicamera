#include "edger/intrusion_detector.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace {

std::vector<uint8_t> SolidFrame(int w, int h, uint8_t value) {
  return std::vector<uint8_t>(w * h, value);
}

std::vector<uint8_t> FrameWithPatch(int w, int h, uint8_t base, uint8_t patch,
                                    int x0, int y0, int pw, int ph) {
  auto frame = SolidFrame(w, h, base);
  for (int y = y0; y < y0 + ph; ++y) {
    for (int x = x0; x < x0 + pw; ++x) {
      frame[y * w + x] = patch;
    }
  }
  return frame;
}

}  // namespace

TEST(Week3Intrusion, DetectsMotionInsideRoi) {
  edger::IntrusionDetector detector;
  detector.SetPolygon({{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}});

  const int w = 160;
  const int h = 90;
  const auto base = SolidFrame(w, h, 20);
  ASSERT_FALSE(detector.ProcessGrayFrame(base, w, h));

  const auto moved = FrameWithPatch(w, h, 20, 220, 40, 20, 80, 40);
  EXPECT_TRUE(detector.ProcessGrayFrame(moved, w, h));
}

TEST(Week3Intrusion, IgnoresStaticScene) {
  edger::IntrusionDetector detector;
  detector.SetPolygon({{0.2f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.2f, 0.8f}});

  const int w = 160;
  const int h = 90;
  const auto frame = SolidFrame(w, h, 50);
  EXPECT_FALSE(detector.ProcessGrayFrame(frame, w, h));
  EXPECT_FALSE(detector.ProcessGrayFrame(frame, w, h));
}
