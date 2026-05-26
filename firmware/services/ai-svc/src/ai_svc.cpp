#include "edger/ai_svc.hpp"

#include "edger/intrusion_detector.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

namespace edger {

namespace {

constexpr int kTargetWidth = 320;
constexpr int kTargetHeight = 180;

std::string AvErrorString(int err) {
  char buf[AV_ERROR_MAX_STRING_SIZE];
  av_strerror(err, buf, sizeof(buf));
  return buf;
}

std::int64_t NowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace

AiService::AiService() = default;

AiService::~AiService() {
  Stop();
}

bool AiService::Start(const AiOptions& options) {
  if (running_) {
    last_error_ = "ai service already running";
    return false;
  }

  if (options.source_url.empty()) {
    last_error_ = "empty source url";
    return false;
  }

  options_ = options;
  stop_requested_ = false;
  frames_analyzed_ = 0;
  alerts_triggered_ = 0;
  worker_ = std::thread(&AiService::RunLoop, this);
  return true;
}

void AiService::Stop() {
  stop_requested_ = true;
  if (worker_.joinable()) {
    worker_.join();
  }
}

void AiService::Wait() {
  if (worker_.joinable()) {
    worker_.join();
  }
}

void AiService::RunLoop() {
  running_ = true;

  AVDictionary* dict = nullptr;
  av_dict_set(&dict, "rtsp_transport", "tcp", 0);
  av_dict_set(&dict, "stimeout", "5000000", 0);

  AVFormatContext* input = nullptr;
  int ret = avformat_open_input(&input, options_.source_url.c_str(), nullptr, &dict);
  av_dict_free(&dict);
  if (ret < 0) {
    last_error_ = "open input failed: " + AvErrorString(ret);
    running_ = false;
    return;
  }

  ret = avformat_find_stream_info(input, nullptr);
  if (ret < 0) {
    last_error_ = "find stream info failed: " + AvErrorString(ret);
    avformat_close_input(&input);
    running_ = false;
    return;
  }

  int video_index = av_find_best_stream(input, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (video_index < 0) {
    last_error_ = "no video stream";
    avformat_close_input(&input);
    running_ = false;
    return;
  }

  AVCodecParameters* codecpar = input->streams[video_index]->codecpar;
  const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
  if (decoder == nullptr) {
    last_error_ = "decoder not found";
    avformat_close_input(&input);
    running_ = false;
    return;
  }

  AVCodecContext* codec = avcodec_alloc_context3(decoder);
  avcodec_parameters_to_context(codec, codecpar);
  ret = avcodec_open2(codec, decoder, nullptr);
  if (ret < 0) {
    last_error_ = "open decoder failed: " + AvErrorString(ret);
    avcodec_free_context(&codec);
    avformat_close_input(&input);
    running_ = false;
    return;
  }

  SwsContext* sws = sws_getContext(codec->width, codec->height, codec->pix_fmt,
                                     kTargetWidth, kTargetHeight, AV_PIX_FMT_GRAY8,
                                     SWS_BILINEAR, nullptr, nullptr, nullptr);

  AVFrame* frame = av_frame_alloc();
  AVFrame* gray = av_frame_alloc();
  gray->format = AV_PIX_FMT_GRAY8;
  gray->width = kTargetWidth;
  gray->height = kTargetHeight;
  av_frame_get_buffer(gray, 0);

  IntrusionDetector detector;
  detector.SetPolygon(options_.intrusion.polygon);

  const auto started = std::chrono::steady_clock::now();
  AVStream* vstream = input->streams[video_index];
  AVRational fps = av_guess_frame_rate(input, vstream, nullptr);
  int source_fps = 25;
  if (fps.num > 0 && fps.den > 0) {
    source_fps = std::max(1, static_cast<int>(fps.num / fps.den));
  }
  const int sample_every =
      std::max(1, source_fps / std::max(1, options_.analyze_fps));
  int decoded_frames = 0;

  AVPacket* packet = av_packet_alloc();
  while (!stop_requested_) {
    ret = av_read_frame(input, packet);
    if (ret == AVERROR_EOF) {
      break;
    }
    if (ret < 0) {
      last_error_ = "read frame failed: " + AvErrorString(ret);
      break;
    }

    if (packet->stream_index != video_index) {
      av_packet_unref(packet);
      continue;
    }

    ret = avcodec_send_packet(codec, packet);
    av_packet_unref(packet);
    if (ret < 0) {
      continue;
    }

    while (avcodec_receive_frame(codec, frame) == 0) {
      ++decoded_frames;
      if (decoded_frames % sample_every != 0) {
        continue;
      }

      sws_scale(sws, frame->data, frame->linesize, 0, codec->height, gray->data,
                gray->linesize);

      std::vector<uint8_t> gray_buf(kTargetWidth * kTargetHeight);
      for (int y = 0; y < kTargetHeight; ++y) {
        memcpy(gray_buf.data() + y * kTargetWidth, gray->data[0] + y * gray->linesize[0],
               kTargetWidth);
      }

      ++frames_analyzed_;
      if (!detector.ProcessGrayFrame(gray_buf, kTargetWidth, kTargetHeight)) {
        continue;
      }

      const float confidence = std::min(
          1.0f, static_cast<float>(detector.last_motion_pixels()) / 500.0f);
      AlertEvent event;
      event.type = "intrusion";
      event.channel_id = options_.channel_id;
      event.confidence = confidence;
      event.timestamp_ms = NowMs();

      ++alerts_triggered_;
      if (options_.on_alert) {
        options_.on_alert(event);
      }
    }

    if (options_.duration_sec > 0) {
      const auto elapsed = std::chrono::steady_clock::now() - started;
      if (elapsed >= std::chrono::seconds(options_.duration_sec)) {
        break;
      }
    }
  }

  av_packet_free(&packet);
  av_frame_free(&frame);
  av_frame_free(&gray);
  sws_freeContext(sws);
  avcodec_free_context(&codec);
  avformat_close_input(&input);
  running_ = false;
}

}  // namespace edger
