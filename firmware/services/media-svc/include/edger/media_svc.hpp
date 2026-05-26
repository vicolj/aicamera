#pragma once

#include <string>

namespace edger {

class MediaService {
 public:
  bool Start(const std::string& rtsp_url);
  void Stop();
  bool running() const { return running_; }

 private:
  bool running_ = false;
};

}  // namespace edger
