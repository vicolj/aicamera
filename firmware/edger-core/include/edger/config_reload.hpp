#pragma once

#include <cstdint>
#include <string>

namespace edger {

class ConfigReload {
 public:
  static std::string MarkerPath(const std::string& config_path);
  static bool Notify(const std::string& config_path);
  static std::uint64_t ReadGeneration(const std::string& config_path);
};

}  // namespace edger
