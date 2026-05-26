#pragma once

#include "edger/alert_event.hpp"
#include "edger/config.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace edger {

struct AiOptions {
  int channel_id = 0;
  std::string source_url;
  IntrusionConfig intrusion;
  AlertConfig alert;
  int analyze_fps = 2;
  int duration_sec = 0;
  std::function<void(const AlertEvent&)> on_alert;
};

class AiService {
 public:
  AiService();
  ~AiService();

  AiService(const AiService&) = delete;
  AiService& operator=(const AiService&) = delete;

  bool Start(const AiOptions& options);
  void Stop();
  void Wait();

  bool running() const { return running_; }
  const std::string& last_error() const { return last_error_; }
  std::uint64_t frames_analyzed() const { return frames_analyzed_; }
  std::uint64_t alerts_triggered() const { return alerts_triggered_; }

 private:
  void RunLoop();

  AiOptions options_;
  std::thread worker_;
  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> running_{false};
  std::string last_error_;
  std::atomic<std::uint64_t> frames_analyzed_{0};
  std::atomic<std::uint64_t> alerts_triggered_{0};
};

}  // namespace edger
