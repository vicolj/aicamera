#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <mutex>
#include <thread>
#include <vector>

namespace edger {

class RecordIndex;

class RecordIndex;

struct RecordOptions {
  int channel_id = 0;
  std::string channel_name;
  std::string rtsp_url;
  std::string output_root;
  int segment_sec = 300;
  int duration_sec = 0;
  RecordIndex* index = nullptr;
  std::mutex* index_mutex = nullptr;
};

class RecordService {
 public:
  RecordService();
  ~RecordService();

  RecordService(const RecordService&) = delete;
  RecordService& operator=(const RecordService&) = delete;

  bool Start(const RecordOptions& options);
  void Stop();
  void Wait();

  bool running() const { return running_; }
  const std::string& last_error() const { return last_error_; }
  const std::string& current_file() const { return current_file_; }
  std::uint64_t packets_written() const { return packets_written_; }

 private:
  void RunLoop();
  bool OpenOutputSegment();
  void CloseOutputSegment();
  void RegisterSegmentInIndex();

  RecordOptions options_;
  std::thread worker_;
  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> running_{false};
  std::string last_error_;
  std::string current_file_;
  std::string current_segment_date_;
  std::atomic<std::uint64_t> packets_written_{0};

  struct FfmpegContext;
  FfmpegContext* ffmpeg_ = nullptr;
};

}  // namespace edger
