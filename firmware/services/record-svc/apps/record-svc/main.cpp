#include "edger/config.hpp"
#include "edger/record_manager.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

namespace {

std::atomic<bool> g_stop{false};

void HandleSignal(int) {
  g_stop = true;
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string config_path = "/etc/edger-rec/config.json";
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if ((arg == "--config" || arg == "-c") && i + 1 < argc) {
      config_path = argv[++i];
    }
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: edger-record-svc [--config PATH]\n";
      return 0;
    }
  }

  edger::Config config;
  if (!config.Load(config_path)) {
    std::cerr << "failed to load config: " << config_path << '\n';
    return 1;
  }

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  edger::RecordManager manager(std::move(config));
  if (!manager.Start(0)) {
    std::cerr << "failed to start record manager\n";
    return 1;
  }

  std::cout << "edger-record-svc started, channels=" << manager.channel_count()
            << " active=" << manager.active_count() << '\n';

  while (!g_stop) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  manager.Stop();
  std::cout << "edger-record-svc stopped\n";
  return 0;
}
