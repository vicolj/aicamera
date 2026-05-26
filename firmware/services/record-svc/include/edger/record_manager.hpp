#pragma once

#include "edger/config.hpp"
#include "edger/record_index.hpp"
#include "edger/record_svc.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace edger {

std::vector<RecordOptions> BuildRecordOptions(const Config& config,
                                              RecordIndex* index,
                                              std::mutex* index_mutex);

class RecordManager {
 public:
  explicit RecordManager(Config config);
  ~RecordManager();

  RecordManager(const RecordManager&) = delete;
  RecordManager& operator=(const RecordManager&) = delete;

  bool Start(int duration_sec = 0);
  void Stop();
  void Wait();

  bool running() const { return running_; }
  size_t active_count() const;
  size_t channel_count() const { return recorders_.size(); }

  RecordIndex* index() { return &index_; }
  RetentionResult RunRetention();

 private:
  void RetentionLoop();

  Config config_;
  RecordIndex index_;
  std::mutex index_mutex_;
  std::vector<std::unique_ptr<RecordService>> recorders_;
  std::thread retention_thread_;
  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> running_{false};
};

}  // namespace edger
