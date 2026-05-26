#pragma once

#include "edger/alert_event.hpp"
#include "edger/config.hpp"

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace edger {

class AlertDispatcher {
 public:
  explicit AlertDispatcher(AlertConfig config);

  bool ShouldDispatch(const AlertEvent& event);
  void MarkDispatched(const AlertEvent& event);

 private:
  struct Key {
    int channel_id = 0;
    std::string type;

    bool operator==(const Key& other) const {
      return channel_id == other.channel_id && type == other.type;
    }
  };

  struct KeyHash {
    std::size_t operator()(const Key& key) const {
      return std::hash<int>()(key.channel_id) ^
             (std::hash<std::string>()(key.type) << 1);
    }
  };

  AlertConfig config_;
  std::mutex mutex_;
  std::unordered_map<Key, std::int64_t, KeyHash> last_sent_ms_;
};

}  // namespace edger
