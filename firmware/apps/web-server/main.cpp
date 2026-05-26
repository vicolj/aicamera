#include "web_server.hpp"

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
  std::string config_path = "config/example.json";
  int port_override = 0;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if ((arg == "--config" || arg == "-c") && i + 1 < argc) {
      config_path = argv[++i];
    } else if (arg == "--port" && i + 1 < argc) {
      port_override = std::stoi(argv[++i]);
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: edger-web-server [--config PATH] [--port PORT]\n";
      return 0;
    }
  }

  edger::Config config;
  if (!config.Load(config_path)) {
    std::cerr << "failed to load config: " << config_path << '\n';
    return 1;
  }

  edger::WebServer::Options options = edger::BuildWebOptions(config);
  if (port_override > 0) {
    options.port = port_override;
  }

  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);

  if (!config.web().enabled) {
    std::cout << "edger-web-server idle (web disabled)\n";
    while (!g_stop) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return 0;
  }

  edger::WebServer server;
  if (!server.Start(options)) {
    std::cerr << "web server start failed: " << server.last_error() << '\n';
    return 1;
  }

  std::cout << "web server listening on " << options.listen_host << ':'
            << options.port << '\n';

  while (!g_stop && server.running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  server.Stop();
  return 0;
}
