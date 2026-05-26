#include "edger/alert_dispatcher.hpp"

#include <utility>

namespace edger {

namespace {

std::int64_t NowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

AlertDispatcher::AlertDispatcher(AlertConfig config) : config_(std::move(config)) {}

bool AlertDispatcher::ShouldDispatch(const AlertEvent& event) {
  std::lock_guard<std::mutex> lock(mutex_);
  const Key key{event.channel_id, event.type};
  const auto it = last_sent_ms_.find(key);
  if (it == last_sent_ms_.end()) {
    return true;
  }
  return NowMs() - it->second >= config_.cooldown_sec * 1000;
}

void AlertDispatcher::MarkDispatched(const AlertEvent& event) {
  std::lock_guard<std::mutex> lock(mutex_);
  last_sent_ms_[Key{event.channel_id, event.type}] = NowMs();
}

}  // namespace edger
