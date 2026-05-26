#include "edger/ai_svc.hpp"
#include "edger/alert_svc.hpp"
#include "edger/config.hpp"
#include "edger/media_svc.hpp"
#include "edger/record_index.hpp"
#include "edger/record_manager.hpp"
#include "edger/record_svc.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

namespace {

std::atomic<bool> g_stop{false};

void HandleSignal(int) {
  g_stop = true;
}

void PrintUsage(const char* argv0) {
  std::cout
      << "EdgeRec Alert v0.1.0 (" << EDGER_BOARD << ")\n\n"
      << "Usage:\n"
      << "  " << argv0 << " [--config PATH] summary\n"
      << "  " << argv0 << " [--config PATH] probe [--channel ID]\n"
      << "  " << argv0 << " [--config PATH] record [--channel ID] [--duration SEC]\n"
      << "  " << argv0 << " [--config PATH] record-all [--duration SEC]\n"
      << "  " << argv0 << " [--config PATH] retention\n";
}

bool LoadConfig(int argc, char* argv[], edger::Config* config,
                std::string* config_path, std::string* command) {
  *config_path = "config/example.json";
  *command = "summary";

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return false;
    }
    if (arg == "--config" && i + 1 < argc) {
      *config_path = argv[++i];
      continue;
    }
    if (arg == "summary" || arg == "probe" || arg == "record" ||
        arg == "record-all" || arg == "retention") {
      *command = arg;
    }
  }

  if (!config->Load(*config_path)) {
    std::cerr << "error: failed to load config: " << *config_path << '\n';
    return false;
  }

  return true;
}

int ParseChannelId(int argc, char* argv[], int default_id) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--channel" && i + 1 < argc) {
      return std::stoi(argv[++i]);
    }
  }
  return default_id;
}

int ParseDuration(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--duration" && i + 1 < argc) {
      return std::stoi(argv[++i]);
    }
  }
  return 0;
}

const edger::ChannelConfig* SelectChannel(const edger::Config& config,
                                          int channel_id) {
  if (const auto* ch = config.FindChannel(channel_id)) {
    return ch;
  }
  return config.FirstEnabledChannel();
}

int CmdSummary(const edger::Config& config) {
  std::cout << "record_root: " << config.app().record_root << '\n'
            << "segment_sec: " << config.app().segment_sec << '\n'
            << "channels: " << config.channels().size() << '\n';
  for (const auto& ch : config.channels()) {
    std::cout << "  [" << ch.id << "] " << ch.name << " enabled=" << ch.enabled
              << " url=" << ch.rtsp_url << '\n';
  }
  return 0;
}

int CmdProbe(const edger::Config& config, int argc, char* argv[]) {
  const int channel_id = ParseChannelId(argc, argv, 0);
  const auto* ch = SelectChannel(config, channel_id);
  if (ch == nullptr) {
    std::cerr << "error: no enabled channel\n";
    return 1;
  }

  edger::MediaService media;
  const auto result = media.Probe(ch->rtsp_url);
  if (!result.ok) {
    std::cerr << "probe failed: " << result.error << '\n';
    return 1;
  }

  std::cout << "probe ok: " << ch->rtsp_url << '\n'
            << "video: " << result.video_codec << " " << result.width << "x"
            << result.height << " streams=" << result.video_streams << '\n'
            << "audio streams: " << result.audio_streams << '\n';
  return 0;
}

int CmdRecord(const edger::Config& config, int argc, char* argv[]) {
  const int channel_id = ParseChannelId(argc, argv, 0);
  const int duration = ParseDuration(argc, argv);
  const auto* ch = SelectChannel(config, channel_id);
  if (ch == nullptr) {
    std::cerr << "error: no enabled channel\n";
    return 1;
  }

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  edger::RecordOptions options;
  options.channel_id = ch->id;
  options.channel_name = ch->name.empty() ? "cam" : ch->name;
  options.rtsp_url = ch->rtsp_url;
  options.output_root = config.app().record_root;
  options.segment_sec = config.app().segment_sec;
  options.duration_sec = duration;

  edger::RecordService recorder;
  if (!recorder.Start(options)) {
    std::cerr << "record start failed: " << recorder.last_error() << '\n';
    return 1;
  }

  std::cout << "recording " << ch->rtsp_url << " -> "
            << config.app().record_root << '\n'
            << "press Ctrl+C to stop\n";

  while (!g_stop && recorder.running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  recorder.Stop();

  if (!recorder.last_error().empty() && recorder.packets_written() == 0) {
    std::cerr << "record failed: " << recorder.last_error() << '\n';
    return 1;
  }

  std::cout << "record stopped, packets=" << recorder.packets_written()
            << " last_file=" << recorder.current_file() << '\n';
  return 0;
}

int CmdRecordAll(const edger::Config& config, int argc, char* argv[]) {
  const int duration = ParseDuration(argc, argv);

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  edger::RecordManager manager(config);
  if (!manager.Start(duration)) {
    std::cerr << "record-all start failed\n";
    return 1;
  }

  std::cout << "recording " << manager.channel_count() << " channels\n";

  if (duration > 0) {
    manager.Wait();
  } else {
    while (!g_stop && manager.active_count() > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  }

  manager.Stop();

  edger::RecordIndex index;
  index.Load(edger::DefaultIndexPath(config.app().record_root));
  std::cout << "record-all stopped, index entries=" << index.size() << '\n';
  return index.size() > 0 ? 0 : 1;
}

int CmdRetention(const edger::Config& config) {
  edger::RecordIndex index;
  index.Load(edger::DefaultIndexPath(config.app().record_root));

  edger::RetentionPolicy policy;
  policy.retention_days = config.app().retention_days;
  const auto result =
      edger::ApplyRetention(config.app().record_root, policy, &index);

  std::cout << "retention removed files=" << result.removed_files
            << " index=" << result.removed_index_entries << '\n';
  return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string config_path;
  std::string command;
  edger::Config config;
  if (!LoadConfig(argc, argv, &config, &config_path, &command)) {
    return 1;
  }

  if (command == "summary") {
    return CmdSummary(config);
  }
  if (command == "probe") {
    return CmdProbe(config, argc, argv);
  }
  if (command == "record") {
    return CmdRecord(config, argc, argv);
  }
  if (command == "record-all") {
    return CmdRecordAll(config, argc, argv);
  }
  if (command == "retention") {
    return CmdRetention(config);
  }

  PrintUsage(argv[0]);
  return 1;
}
