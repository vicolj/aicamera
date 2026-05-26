#include "edger/record_svc.hpp"

#include "edger/record_index.hpp"
#include "edger/util/fs.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

#include <chrono>
#include <ctime>
#include <cstring>
#include <iostream>
#include <mutex>
#include <vector>

namespace edger {

namespace {

constexpr int kRtspTimeoutUs = 5 * 1000 * 1000;

std::string AvErrorString(int err) {
  char buf[AV_ERROR_MAX_STRING_SIZE];
  av_strerror(err, buf, sizeof(buf));
  return buf;
}

}  // namespace

struct RecordService::FfmpegContext {
  AVFormatContext* input = nullptr;
  AVFormatContext* output = nullptr;
  int64_t segment_start_pts = AV_NOPTS_VALUE;
  int video_stream_index = -1;
  std::vector<int> stream_map;
};

RecordService::RecordService() = default;

RecordService::~RecordService() {
  Stop();
}

bool RecordService::Start(const RecordOptions& options) {
  if (running_) {
    last_error_ = "recorder already running";
    return false;
  }

  options_ = options;
  if (options_.rtsp_url.empty()) {
    last_error_ = "empty rtsp url";
    return false;
  }

  stop_requested_ = false;
  packets_written_ = 0;
  worker_ = std::thread(&RecordService::RunLoop, this);
  return true;
}

void RecordService::Stop() {
  stop_requested_ = true;
  if (worker_.joinable()) {
    worker_.join();
  }
}

void RecordService::Wait() {
  if (worker_.joinable()) {
    worker_.join();
  }
}

bool RecordService::OpenOutputSegment() {
  CloseOutputSegment();

  const std::string channel_dir =
      "ch" + std::to_string(options_.channel_id) + "_" + options_.channel_name;
  const std::string day_dir =
      util::JoinPath(util::JoinPath(options_.output_root, channel_dir),
                     util::DateDirectoryName());

  if (!util::EnsureDirectory(day_dir)) {
    last_error_ = "failed to create directory: " + day_dir;
    return false;
  }

  current_file_ =
      util::JoinPath(day_dir, util::TimestampForFilename() + ".mp4");
  current_segment_date_ = util::DateDirectoryName();

  int ret = avformat_alloc_output_context2(&ffmpeg_->output, nullptr, "mp4",
                                           current_file_.c_str());
  if (ret < 0 || ffmpeg_->output == nullptr) {
    last_error_ = "alloc output failed: " + AvErrorString(ret);
    return false;
  }

  for (unsigned i = 0; i < ffmpeg_->input->nb_streams; ++i) {
    AVStream* in_stream = ffmpeg_->input->streams[i];
    if (in_stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
        in_stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
      continue;
    }

    AVStream* out_stream = avformat_new_stream(ffmpeg_->output, nullptr);
    if (out_stream == nullptr) {
      last_error_ = "new stream failed";
      return false;
    }

    ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
    if (ret < 0) {
      last_error_ = "copy codecpar failed: " + AvErrorString(ret);
      return false;
    }

    out_stream->codecpar->codec_tag = 0;
    out_stream->time_base = in_stream->time_base;
    ffmpeg_->stream_map[i] = out_stream->index;

    if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      ffmpeg_->video_stream_index = static_cast<int>(i);
    }
  }

  if (ffmpeg_->video_stream_index < 0) {
    last_error_ = "no video stream found";
    return false;
  }

  if (!(ffmpeg_->output->oformat->flags & AVFMT_NOFILE)) {
    ret = avio_open(&ffmpeg_->output->pb, current_file_.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      last_error_ = "open output failed: " + AvErrorString(ret);
      return false;
    }
  }

  ret = avformat_write_header(ffmpeg_->output, nullptr);
  if (ret < 0) {
    last_error_ = "write header failed: " + AvErrorString(ret);
    return false;
  }

  ffmpeg_->segment_start_pts = AV_NOPTS_VALUE;
  std::cout << "[record] segment opened: " << current_file_ << '\n';
  return true;
}

void RecordService::RegisterSegmentInIndex() {
  if (options_.index == nullptr || current_file_.empty()) {
    return;
  }

  RecordingEntry entry;
  entry.channel_id = options_.channel_id;
  entry.channel_name = options_.channel_name;
  entry.date = current_segment_date_;
  entry.path = current_file_;
  entry.created_at_unix = std::time(nullptr);
  entry.size_bytes = util::GetFileSize(current_file_);

  if (options_.index_mutex != nullptr) {
    std::lock_guard<std::mutex> lock(*options_.index_mutex);
    options_.index->Append(entry);
  } else {
    options_.index->Append(entry);
  }
}

void RecordService::CloseOutputSegment() {
  if (ffmpeg_ == nullptr || ffmpeg_->output == nullptr) {
    return;
  }

  av_write_trailer(ffmpeg_->output);
  RegisterSegmentInIndex();

  if (!(ffmpeg_->output->oformat->flags & AVFMT_NOFILE) &&
      ffmpeg_->output->pb != nullptr) {
    avio_closep(&ffmpeg_->output->pb);
  }

  avformat_free_context(ffmpeg_->output);
  ffmpeg_->output = nullptr;
}

void RecordService::RunLoop() {
  running_ = true;
  ffmpeg_ = new FfmpegContext();

  AVDictionary* options = nullptr;
  av_dict_set(&options, "rtsp_transport", "tcp", 0);
  av_dict_set(&options, "stimeout", std::to_string(kRtspTimeoutUs).c_str(), 0);

  int ret = avformat_open_input(&ffmpeg_->input, options_.rtsp_url.c_str(),
                                nullptr, &options);
  av_dict_free(&options);

  if (ret < 0) {
    last_error_ = "open input failed: " + AvErrorString(ret);
    running_ = false;
    delete ffmpeg_;
    ffmpeg_ = nullptr;
    return;
  }

  ret = avformat_find_stream_info(ffmpeg_->input, nullptr);
  if (ret < 0) {
    last_error_ = "find stream info failed: " + AvErrorString(ret);
    avformat_close_input(&ffmpeg_->input);
    running_ = false;
    delete ffmpeg_;
    ffmpeg_ = nullptr;
    return;
  }

  ffmpeg_->stream_map.assign(ffmpeg_->input->nb_streams, -1);

  if (!OpenOutputSegment()) {
    avformat_close_input(&ffmpeg_->input);
    running_ = false;
    delete ffmpeg_;
    ffmpeg_ = nullptr;
    return;
  }

  const auto started = std::chrono::steady_clock::now();
  AVPacket* packet = av_packet_alloc();

  while (!stop_requested_) {
    ret = av_read_frame(ffmpeg_->input, packet);
    if (ret == AVERROR_EOF) {
      std::cout << "[record] input eof\n";
      break;
    }
    if (ret < 0) {
      last_error_ = "read frame failed: " + AvErrorString(ret);
      break;
    }

    if (packet->stream_index < 0 ||
        packet->stream_index >= static_cast<int>(ffmpeg_->stream_map.size())) {
      av_packet_unref(packet);
      continue;
    }

    const int out_index = ffmpeg_->stream_map[packet->stream_index];
    if (out_index < 0) {
      av_packet_unref(packet);
      continue;
    }

    AVStream* in_stream = ffmpeg_->input->streams[packet->stream_index];

    if (ffmpeg_->video_stream_index >= 0 &&
        packet->stream_index == ffmpeg_->video_stream_index &&
        packet->pts != AV_NOPTS_VALUE) {
      if (ffmpeg_->segment_start_pts == AV_NOPTS_VALUE) {
        ffmpeg_->segment_start_pts = packet->pts;
      } else {
        AVStream* vstream = ffmpeg_->input->streams[ffmpeg_->video_stream_index];
        const int64_t elapsed =
            av_rescale_q(packet->pts - ffmpeg_->segment_start_pts,
                         vstream->time_base, {1, 1});
        if (elapsed >= options_.segment_sec) {
          CloseOutputSegment();
          if (!OpenOutputSegment()) {
            av_packet_unref(packet);
            break;
          }
        }
      }
    }

    AVStream* out_stream = ffmpeg_->output->streams[out_index];
    packet->stream_index = out_index;
    av_packet_rescale_ts(packet, in_stream->time_base, out_stream->time_base);
    packet->pos = -1;

    ret = av_interleaved_write_frame(ffmpeg_->output, packet);
    av_packet_unref(packet);
    if (ret < 0) {
      last_error_ = "write frame failed: " + AvErrorString(ret);
      break;
    }

    ++packets_written_;

    if (options_.duration_sec > 0) {
      const auto elapsed = std::chrono::steady_clock::now() - started;
      if (elapsed >= std::chrono::seconds(options_.duration_sec)) {
        std::cout << "[record] duration reached\n";
        break;
      }
    }
  }

  av_packet_free(&packet);
  CloseOutputSegment();
  avformat_close_input(&ffmpeg_->input);

  delete ffmpeg_;
  ffmpeg_ = nullptr;
  running_ = false;
}

}  // namespace edger
