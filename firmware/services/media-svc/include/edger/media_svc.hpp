#pragma once

#include <string>

namespace edger {

struct RtspProbeResult {
  bool ok = false;
  std::string error;
  int video_streams = 0;
  int audio_streams = 0;
  std::string video_codec;
  int width = 0;
  int height = 0;
};

class MediaService {
 public:
  RtspProbeResult Probe(const std::string& rtsp_url) const;
};

}  // namespace edger
