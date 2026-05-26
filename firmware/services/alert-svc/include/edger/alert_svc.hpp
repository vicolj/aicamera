#pragma once

#include <string>

namespace edger {

class AlertService {
 public:
  bool SendWebhook(const std::string& url, const std::string& payload);
};

}  // namespace edger
