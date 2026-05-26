#include "edger/config.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#include "edger/util/fs.hpp"

namespace edger {

namespace {

IntrusionConfig ParseIntrusion(const nlohmann::json& node) {
  IntrusionConfig cfg;
  cfg.enabled = node.value("enabled", false);
  cfg.channel_id = node.value("channel_id", 0);

  if (node.contains("polygon") && node["polygon"].is_array()) {
    for (const auto& point : node["polygon"]) {
      if (point.is_array() && point.size() >= 2) {
        cfg.polygon.emplace_back(point[0].get<float>(), point[1].get<float>());
      }
    }
  }

  return cfg;
}

}  // namespace

bool Config::Load(const std::string& path) {
  app_.config_path = path;

  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }

  nlohmann::json root;
  try {
    in >> root;
  } catch (const nlohmann::json::exception&) {
    return false;
  }

  if (root.contains("app")) {
    const auto& app = root["app"];
    app_.record_root = app.value("record_root", app_.record_root);
    app_.max_channels = app.value("max_channels", app_.max_channels);
    app_.retention_days = app.value("retention_days", app_.retention_days);
    app_.segment_sec = app.value("segment_sec", app_.segment_sec);
  }

  channels_.clear();
  if (root.contains("channels") && root["channels"].is_array()) {
    for (const auto& ch : root["channels"]) {
      ChannelConfig cfg;
      cfg.id = ch.value("id", 0);
      cfg.name = ch.value("name", "");
      cfg.rtsp_url = ch.value("rtsp_url", "");
      cfg.enabled = ch.value("enabled", true);
      channels_.push_back(cfg);
    }
  }

  if (root.contains("alert")) {
    const auto& alert = root["alert"];
    alert_.webhook_url = alert.value("webhook_url", "");
    alert_.cooldown_sec = alert.value("cooldown_sec", alert_.cooldown_sec);
  }

  if (root.contains("ai") && root["ai"].contains("intrusion")) {
    ai_.intrusion = ParseIntrusion(root["ai"]["intrusion"]);
  }

  return true;
}

bool Config::Save(const std::string& path) const {
  nlohmann::json root;
  root["app"] = {
      {"record_root", app_.record_root},
      {"max_channels", app_.max_channels},
      {"retention_days", app_.retention_days},
      {"segment_sec", app_.segment_sec},
  };

  nlohmann::json channels = nlohmann::json::array();
  for (const auto& ch : channels_) {
    channels.push_back({{"id", ch.id},
                        {"name", ch.name},
                        {"rtsp_url", ch.rtsp_url},
                        {"enabled", ch.enabled}});
  }
  root["channels"] = channels;

  root["alert"] = {{"webhook_url", alert_.webhook_url},
                   {"cooldown_sec", alert_.cooldown_sec}};

  nlohmann::json polygon = nlohmann::json::array();
  for (const auto& [x, y] : ai_.intrusion.polygon) {
    polygon.push_back({x, y});
  }
  root["ai"] = {{"intrusion",
                 {{"enabled", ai_.intrusion.enabled},
                  {"channel_id", ai_.intrusion.channel_id},
                  {"polygon", polygon}}}};

  const auto slash = path.find_last_of('/');
  if (slash != std::string::npos) {
    util::EnsureDirectory(path.substr(0, slash));
  }

  std::ofstream out(path);
  if (!out.is_open()) {
    return false;
  }

  out << root.dump(2) << '\n';
  return true;
}

const ChannelConfig* Config::FindChannel(int id) const {
  for (const auto& ch : channels_) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
}

const ChannelConfig* Config::FirstEnabledChannel() const {
  for (const auto& ch : channels_) {
    if (ch.enabled && !ch.rtsp_url.empty()) {
      return &ch;
    }
  }
  return nullptr;
}

}  // namespace edger
