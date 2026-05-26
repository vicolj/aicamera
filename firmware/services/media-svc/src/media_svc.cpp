#include "edger/media_svc.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace edger {

namespace {

std::string AvErrorString(int err) {
  char buf[AV_ERROR_MAX_STRING_SIZE];
  av_strerror(err, buf, sizeof(buf));
  return buf;
}

}  // namespace

RtspProbeResult MediaService::Probe(const std::string& rtsp_url) const {
  RtspProbeResult result;

  AVDictionary* options = nullptr;
  av_dict_set(&options, "rtsp_transport", "tcp", 0);
  av_dict_set(&options, "stimeout", "5000000", 0);

  AVFormatContext* input = nullptr;
  int ret = avformat_open_input(&input, rtsp_url.c_str(), nullptr, &options);
  av_dict_free(&options);

  if (ret < 0) {
    result.error = AvErrorString(ret);
    return result;
  }

  ret = avformat_find_stream_info(input, nullptr);
  if (ret < 0) {
    result.error = AvErrorString(ret);
    avformat_close_input(&input);
    return result;
  }

  for (unsigned i = 0; i < input->nb_streams; ++i) {
    const AVStream* stream = input->streams[i];
    const AVCodecParameters* par = stream->codecpar;
    if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
      ++result.video_streams;
      result.width = par->width;
      result.height = par->height;
      if (const AVCodec* codec = avcodec_find_decoder(par->codec_id)) {
        result.video_codec = codec->name;
      }
    } else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
      ++result.audio_streams;
    }
  }

  avformat_close_input(&input);
  result.ok = result.video_streams > 0;
  if (!result.ok) {
    result.error = "no video stream";
  }

  return result;
}

}  // namespace edger
