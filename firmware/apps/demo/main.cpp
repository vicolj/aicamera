#include "edger/ai_svc.hpp"
#include "edger/alert_svc.hpp"
#include "edger/config.hpp"
#include "edger/media_svc.hpp"
#include "edger/record_svc.hpp"

#include <iostream>
#include <string>

namespace {

void PrintUsage(const char* argv0) {
  std::cout << "EdgeRec Alert demo\n"
            << "Usage: " << argv0 << " [--help] [--config PATH]\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string config_path = "/etc/edger-rec/config.json";

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return 0;
    }
    if (arg == "--config" && i + 1 < argc) {
      config_path = argv[++i];
    }
  }

  edger::Config config;
  if (!config.Load(config_path)) {
    std::cerr << "warn: config not found, using defaults: " << config_path
              << '\n';
  }

  edger::MediaService media;
  edger::RecordService record;
  edger::AiService ai;
  edger::AlertService alert;

  (void)media;
  (void)record;
  (void)ai;
  (void)alert;

  std::cout << "EdgeRec Alert v0.1.0 (" << EDGER_BOARD << ")\n"
            << "record_root: " << config.app().record_root << '\n'
            << "services: media record ai alert [stubs]\n";

  return 0;
}
