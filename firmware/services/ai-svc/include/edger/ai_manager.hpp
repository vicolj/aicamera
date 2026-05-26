#pragma once

#include "edger/ai_svc.hpp"

#include <atomic>
#include <cstdint>
#include "edger/alert_dispatcher.hpp"
#include "edger/alert_svc.hpp"
#include "edger/config.hpp"

namespace edger {

class AiManager {
 public:
  explicit AiManager(Config config);

  bool Start(int duration_sec = 0);
  void Stop();
  void Wait();

  std::uint64_t alerts_sent() const { return alerts_sent_; }

 private:
  Config config_;
  AlertService alert_service_;
  AlertDispatcher dispatcher_;
  AiService ai_service_;
  std::atomic<std::uint64_t> alerts_sent_{0};
};

}  // namespace edger
