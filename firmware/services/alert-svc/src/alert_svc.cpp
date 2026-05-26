#include "edger/alert_svc.hpp"

#include "edger/http_client.hpp"

#include <nlohmann/json.hpp>

namespace edger {

std::string AlertService::BuildPayload(const AlertEvent& event) const {
  nlohmann::json payload = {{"type", event.type},
                            {"channel_id", event.channel_id},
                            {"confidence", event.confidence},
                            {"timestamp_ms", event.timestamp_ms}};
  return payload.dump();
}

bool AlertService::SendWebhook(const std::string& url, const AlertEvent& event) {
  if (url.empty()) {
    return false;
  }

  const std::string payload = BuildPayload(event);
  long http_code = 0;
  return http::PostJson(url, payload, &http_code);
}

}  // namespace edger
