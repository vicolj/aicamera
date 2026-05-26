#include "edger/ai_manager.hpp"

#include <iostream>

namespace edger {

AiManager::AiManager(Config config)
    : config_(std::move(config)), dispatcher_(config_.alert()) {}

bool AiManager::Start(int duration_sec) {
  if (!config_.ai().intrusion.enabled) {
    std::cerr << "intrusion detection disabled\n";
    return false;
  }

  const auto* channel = config_.FindChannel(config_.ai().intrusion.channel_id);
  if (channel == nullptr) {
    channel = config_.FirstEnabledChannel();
  }
  if (channel == nullptr) {
    std::cerr << "no channel for ai\n";
    return false;
  }

  AiOptions options;
  options.channel_id = channel->id;
  options.source_url = channel->rtsp_url;
  options.intrusion = config_.ai().intrusion;
  options.alert = config_.alert();
  options.duration_sec = duration_sec;
  options.analyze_fps = 5;
  options.on_alert = [this](const AlertEvent& event) {
    if (!dispatcher_.ShouldDispatch(event)) {
      return;
    }
    if (alert_service_.SendWebhook(config_.alert().webhook_url, event)) {
      dispatcher_.MarkDispatched(event);
      ++alerts_sent_;
      std::cout << "[alert] sent type=" << event.type
                << " channel=" << event.channel_id
                << " confidence=" << event.confidence << '\n';
    }
  };

  return ai_service_.Start(options);
}

void AiManager::Stop() {
  ai_service_.Stop();
}

void AiManager::Wait() {
  ai_service_.Wait();
}

}  // namespace edger
