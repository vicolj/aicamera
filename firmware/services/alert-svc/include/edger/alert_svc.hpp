#pragma once

#include "edger/alert_event.hpp"

#include <string>

namespace edger {

class AlertService {
 public:
  bool SendWebhook(const std::string& url, const AlertEvent& event);
  std::string BuildPayload(const AlertEvent& event) const;
};

}  // namespace edger
