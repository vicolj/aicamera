#pragma once

#include "edger/config.hpp"

#include <atomic>
#include <string>
#include <thread>

namespace httplib {
class Server;
}

#ifndef EDGER_WEB_STATIC_DIR
#define EDGER_WEB_STATIC_DIR "static"
#endif

namespace edger {

class WebServer {
 public:
  struct Options {
    std::string listen_host = "0.0.0.0";
    int port = 8080;
    std::string record_root;
    std::string index_path;
    std::string auth_token;
    std::string static_dir = EDGER_WEB_STATIC_DIR;
  };

  WebServer();
  ~WebServer();

  WebServer(const WebServer&) = delete;
  WebServer& operator=(const WebServer&) = delete;

  bool Start(const Options& options);
  void Stop();
  void Wait();

  bool running() const { return running_; }
  const std::string& last_error() const { return last_error_; }

 private:
  void Run();

  Options options_;
  std::thread worker_;
  httplib::Server* active_server_ = nullptr;
  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> running_{false};
  std::string last_error_;
};

WebServer::Options BuildWebOptions(const Config& config);

}  // namespace edger
