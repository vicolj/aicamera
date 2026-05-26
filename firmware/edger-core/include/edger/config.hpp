#pragma once

#include <string>
#include <utility>
#include <vector>

namespace edger {

struct ChannelConfig {
  int id = 0;
  std::string name;
  std::string rtsp_url;
  bool enabled = true;
};

struct AlertConfig {
  std::string webhook_url;
  int cooldown_sec = 10;
};

struct IntrusionConfig {
  bool enabled = false;
  int channel_id = 0;
  std::vector<std::pair<float, float>> polygon;
};

struct AiConfig {
  IntrusionConfig intrusion;
};

struct WebConfig {
  bool enabled = true;
  std::string listen_host = "0.0.0.0";
  int port = 8080;
  std::string auth_token;
};

struct AppConfig {
  std::string config_path = "/etc/edger-rec/config.json";
  std::string record_root = "/var/lib/edger-rec/recordings";
  int max_channels = 4;
  int retention_days = 7;
  int max_storage_gb = 0;
  int segment_sec = 300;
};

class Config {
 public:
  bool Load(const std::string& path);
  bool Save(const std::string& path) const;

  const AppConfig& app() const { return app_; }
  const std::vector<ChannelConfig>& channels() const { return channels_; }
  const AlertConfig& alert() const { return alert_; }
  const AiConfig& ai() const { return ai_; }
  const WebConfig& web() const { return web_; }

  AppConfig& mutable_app() { return app_; }
  std::vector<ChannelConfig>& mutable_channels() { return channels_; }
  AlertConfig& mutable_alert() { return alert_; }
  AiConfig& mutable_ai() { return ai_; }
  WebConfig& mutable_web() { return web_; }

  const ChannelConfig* FindChannel(int id) const;
  const ChannelConfig* FirstEnabledChannel() const;

 private:
  AppConfig app_;
  std::vector<ChannelConfig> channels_;
  AlertConfig alert_;
  AiConfig ai_;
  WebConfig web_;
};

}  // namespace edger
