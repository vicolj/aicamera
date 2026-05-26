#include "edger/config_reload.hpp"

#include "edger/util/fs.hpp"

#include <chrono>
#include <fstream>

namespace edger {

std::string ConfigReload::MarkerPath(const std::string& config_path) {
  return config_path + ".reload";
}

bool ConfigReload::Notify(const std::string& config_path) {
  const std::string marker = MarkerPath(config_path);
  const auto slash = marker.find_last_of('/');
  if (slash != std::string::npos) {
    util::EnsureDirectory(marker.substr(0, slash));
  }

  const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::system_clock::now().time_since_epoch())
                     .count();

  std::ofstream out(marker);
  if (!out.is_open()) {
    return false;
  }

  out << now << '\n';
  return true;
}

std::uint64_t ConfigReload::ReadGeneration(const std::string& config_path) {
  std::ifstream in(MarkerPath(config_path));
  if (!in.is_open()) {
    return 0;
  }

  std::uint64_t generation = 0;
  in >> generation;
  return generation;
}

}  // namespace edger
