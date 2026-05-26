#pragma once

#include <cstdint>
#include <string>

namespace edger {

struct AlertEvent {
  std::string type = "intrusion";
  int channel_id = 0;
  float confidence = 0.0f;
  std::int64_t timestamp_ms = 0;
};

}  // namespace edger
