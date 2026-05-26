#include "edger/alert_svc.hpp"

namespace edger {

bool AlertService::SendWebhook(const std::string& /*url*/,
                               const std::string& /*payload*/) {
  // TODO Week 3: libcurl HTTP POST
  return true;
}

}  // namespace edger
