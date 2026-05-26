#include "edger/record_manager.hpp"

#include "edger/config.hpp"

#include <chrono>
#include <iostream>

namespace edger {

std::vector<RecordOptions> BuildRecordOptions(const Config& config,
                                              RecordIndex* index,
                                              std::mutex* index_mutex) {
  std::vector<RecordOptions> options_list;
  int started = 0;

  for (const auto& ch : config.channels()) {
    if (!ch.enabled || ch.rtsp_url.empty()) {
      continue;
    }
    if (started >= config.app().max_channels) {
      break;
    }

    RecordOptions options;
    options.channel_id = ch.id;
    options.channel_name = ch.name.empty() ? "cam" : ch.name;
    options.rtsp_url = ch.rtsp_url;
    options.output_root = config.app().record_root;
    options.segment_sec = config.app().segment_sec;
    options.index = index;
    options.index_mutex = index_mutex;
    options_list.push_back(options);
    ++started;
  }

  return options_list;
}

RecordManager::RecordManager(Config config) : config_(std::move(config)) {}

RecordManager::~RecordManager() {
  Stop();
}

bool RecordManager::Start(int duration_sec) {
  if (running_) {
    return false;
  }

  const std::string index_path =
      DefaultIndexPath(config_.app().record_root);
  if (!index_.Load(index_path)) {
    std::cerr << "failed to load index: " << index_path << '\n';
    return false;
  }
  index_.SetPath(index_path);

  auto options_list =
      BuildRecordOptions(config_, &index_, &index_mutex_);
  if (options_list.empty()) {
    std::cerr << "no enabled channels\n";
    return false;
  }

  recorders_.clear();
  for (auto& options : options_list) {
    options.duration_sec = duration_sec;
    auto recorder = std::make_unique<RecordService>();
    if (!recorder->Start(options)) {
      std::cerr << "failed to start channel " << options.channel_id << ": "
                << recorder->last_error() << '\n';
      continue;
    }
    recorders_.push_back(std::move(recorder));
  }

  if (recorders_.empty()) {
    return false;
  }

  stop_requested_ = false;
  running_ = true;
  retention_thread_ = std::thread(&RecordManager::RetentionLoop, this);
  return true;
}

void RecordManager::Stop() {
  stop_requested_ = true;
  for (auto& recorder : recorders_) {
    recorder->Stop();
  }
  if (retention_thread_.joinable()) {
    retention_thread_.join();
  }
  recorders_.clear();
  running_ = false;
}

void RecordManager::Wait() {
  for (auto& recorder : recorders_) {
    recorder->Wait();
  }
}

size_t RecordManager::active_count() const {
  size_t count = 0;
  for (const auto& recorder : recorders_) {
    if (recorder->running()) {
      ++count;
    }
  }
  return count;
}

RetentionResult RecordManager::RunRetention() {
  RetentionPolicy policy;
  policy.retention_days = config_.app().retention_days;
  std::lock_guard<std::mutex> lock(index_mutex_);
  return ApplyRetention(config_.app().record_root, policy, &index_);
}

void RecordManager::RetentionLoop() {
  while (!stop_requested_) {
    const auto result = RunRetention();
    if (result.removed_files > 0) {
      std::cout << "[retention] removed files=" << result.removed_files
                << " index=" << result.removed_index_entries << '\n';
    }

    for (int i = 0; i < 60 && !stop_requested_; ++i) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

}  // namespace edger
