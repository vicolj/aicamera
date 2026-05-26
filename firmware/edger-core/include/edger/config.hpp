#pragma once

#include <string>

namespace edger {

struct ChannelConfig {
  int id = 0;
  std::string name;
  std::string rtsp_url;
  bool enabled = true;
};

struct AppConfig {
  std::string config_path = "/etc/edger-rec/config.json";
  std::string record_root = "/var/lib/edger-rec/recordings";
  int max_channels = 4;
  int retention_days = 7;
};

class Config {
 public:
  bool Load(const std::string& path);
  bool Save(const std::string& path) const;

  const AppConfig& app() const { return app_; }

 private:
  AppConfig app_;
};

}  // namespace edger
