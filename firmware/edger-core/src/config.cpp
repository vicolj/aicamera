#include "edger/config.hpp"

#include <fstream>

namespace edger {

bool Config::Load(const std::string& path) {
  app_.config_path = path;
  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }
  // TODO: JSON parse (nlohmann/json in Week 1)
  return true;
}

bool Config::Save(const std::string& path) const {
  std::ofstream out(path);
  if (!out.is_open()) {
    return false;
  }
  // TODO: JSON serialize
  out << "{}\n";
  return true;
}

}  // namespace edger
