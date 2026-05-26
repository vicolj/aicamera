#pragma once

#include <string>

namespace edger {

class RecordService {
 public:
  bool Start(const std::string& output_dir);
  void Stop();
};

}  // namespace edger
