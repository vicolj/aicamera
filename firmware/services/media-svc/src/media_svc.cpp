#include "edger/media_svc.hpp"

namespace edger {

bool MediaService::Start(const std::string& /*rtsp_url*/) {
  running_ = true;
  // TODO Week 1: FFmpeg avformat_open_input
  return true;
}

void MediaService::Stop() {
  running_ = false;
}

}  // namespace edger
