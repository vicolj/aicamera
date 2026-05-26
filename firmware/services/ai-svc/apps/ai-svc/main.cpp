#include "edger/ai_manager.hpp"
#include "edger/config.hpp"

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
  int duration = 0;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if ((arg == "--config" || arg == "-c") && i + 1 < argc) {
      config_path = argv[++i];
    } else if (arg == "--duration" && i + 1 < argc) {
      duration = std::stoi(argv[++i]);
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: edger-ai-svc [--config PATH] [--duration SEC]\n";
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

  edger::AiManager manager(std::move(config));
  if (!config.ai().intrusion.enabled) {
    std::cout << "edger-ai-svc idle (intrusion disabled)\n";
    while (!g_stop) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return 0;
  }

  if (!manager.Start(duration)) {
    return 1;
  }

  if (duration > 0) {
    manager.Wait();
  } else {
    while (!g_stop) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  manager.Stop();
  std::cout << "alerts_sent=" << manager.alerts_sent() << '\n';
  return 0;
}
