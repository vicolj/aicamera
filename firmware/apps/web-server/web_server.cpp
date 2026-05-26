#include "web_server.hpp"

#include "edger/record_index.hpp"
#include "edger/util/fs.hpp"
#include "edger/web_auth.hpp"

#include <httplib.h>

#include <nlohmann/json.hpp>

#include <cctype>
#include <iostream>
#include <memory>

namespace edger {

namespace {

std::string UrlEncode(const std::string& value) {
  static const char* kHex = "0123456789ABCDEF";
  std::string encoded;
  encoded.reserve(value.size() * 3);

  for (unsigned char ch : value) {
    if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.') {
      encoded.push_back(static_cast<char>(ch));
      continue;
    }
    encoded.push_back('%');
    encoded.push_back(kHex[ch >> 4]);
    encoded.push_back(kHex[ch & 0x0F]);
  }

  return encoded;
}

RecordingQuery ParseRecordingQuery(const httplib::Request& req) {
  RecordingQuery query;
  if (req.has_param("channel_id")) {
    query.channel_id = std::stoi(req.get_param_value("channel_id"));
  }
  if (req.has_param("from")) {
    query.from_unix = std::stoll(req.get_param_value("from"));
  }
  if (req.has_param("to")) {
    query.to_unix = std::stoll(req.get_param_value("to"));
  }
  if (req.has_param("date")) {
    query.date = req.get_param_value("date");
  }
  return query;
}

bool AuthorizeRequest(const WebServer::Options& options,
                      const httplib::Request& req) {
  return web::IsAuthorized(options.auth_token,
                           req.get_header_value("Authorization"),
                           req.get_header_value("Cookie"));
}

nlohmann::json EntryToJson(const RecordingEntry& entry,
                           const std::string& record_root) {
  const std::string rel = util::RelativePath(record_root, entry.path);
  nlohmann::json item = {{"channel_id", entry.channel_id},
                         {"channel_name", entry.channel_name},
                         {"date", entry.date},
                         {"path", entry.path},
                         {"created_at_unix", entry.created_at_unix},
                         {"size_bytes", entry.size_bytes}};
  if (!rel.empty()) {
    item["media_rel"] = rel;
    item["play_url"] = "/api/media?rel=" + UrlEncode(rel);
  }
  return item;
}

}  // namespace

WebServer::Options BuildWebOptions(const Config& config) {
  WebServer::Options options;
  options.listen_host = config.web().listen_host;
  options.port = config.web().port;
  options.record_root = config.app().record_root;
  options.index_path = DefaultIndexPath(config.app().record_root);
  options.auth_token = config.web().auth_token;
  return options;
}

WebServer::WebServer() = default;

WebServer::~WebServer() {
  Stop();
}

bool WebServer::Start(const Options& options) {
  if (running_) {
    last_error_ = "web server already running";
    return false;
  }

  options_ = options;
  stop_requested_ = false;
  worker_ = std::thread(&WebServer::Run, this);
  return true;
}

void WebServer::Stop() {
  stop_requested_ = true;
  if (active_server_ != nullptr) {
    active_server_->stop();
  }
  if (worker_.joinable()) {
    worker_.join();
  }
  active_server_ = nullptr;
}

void WebServer::Wait() {
  if (worker_.joinable()) {
    worker_.join();
  }
}

void WebServer::Run() {
  running_ = true;

  auto server = std::make_unique<httplib::Server>();
  active_server_ = server.get();
  server->set_read_timeout(30, 0);
  server->set_write_timeout(30, 0);

  if (!options_.static_dir.empty()) {
    server->set_mount_point("/", options_.static_dir.c_str());
  }

  server->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
    res.set_content(R"({"ok":true})", "application/json");
  });

  server->Post("/api/login",
               [this](const httplib::Request& req, httplib::Response& res) {
                 if (options_.auth_token.empty()) {
                   res.set_content(R"({"ok":true,"auth":"disabled"})",
                                   "application/json");
                   return;
                 }

                 nlohmann::json body;
                 try {
                   body = nlohmann::json::parse(req.body);
                 } catch (const nlohmann::json::exception&) {
                   res.status = 400;
                   res.set_content(R"({"ok":false,"error":"invalid json"})",
                                   "application/json");
                   return;
                 }

                 const std::string token = body.value("token", "");
                 if (!web::TokenMatches(token, options_.auth_token)) {
                   res.status = 401;
                   res.set_content(R"({"ok":false,"error":"invalid token"})",
                                   "application/json");
                   return;
                 }

                 res.set_header("Set-Cookie",
                                "edger_token=" + token + "; Path=/; HttpOnly");
                 res.set_content(R"({"ok":true})", "application/json");
               });

  server->Get("/api/recordings",
              [this](const httplib::Request& req, httplib::Response& res) {
                if (!AuthorizeRequest(options_, req)) {
                  res.status = 401;
                  res.set_content(R"({"ok":false,"error":"unauthorized"})",
                                  "application/json");
                  return;
                }

                RecordIndex index;
                index.Load(options_.index_path);

                const RecordingQuery query = ParseRecordingQuery(req);
                const auto entries = QueryRecordings(index, query);

                nlohmann::json payload;
                payload["ok"] = true;
                payload["count"] = entries.size();
                payload["entries"] = nlohmann::json::array();
                for (const auto& entry : entries) {
                  payload["entries"].push_back(
                      EntryToJson(entry, options_.record_root));
                }

                res.set_content(payload.dump(), "application/json");
              });

  server->Get("/api/media",
              [this](const httplib::Request& req, httplib::Response& res) {
                if (!AuthorizeRequest(options_, req)) {
                  res.status = 401;
                  res.set_content("unauthorized", "text/plain");
                  return;
                }

                if (!req.has_param("rel")) {
                  res.status = 400;
                  res.set_content("missing rel", "text/plain");
                  return;
                }

                const std::string rel = req.get_param_value("rel");
                const std::string abs_path =
                    util::JoinPath(options_.record_root, rel);
                if (!util::IsSubPath(options_.record_root, abs_path) ||
                    !util::FileExists(abs_path)) {
                  res.status = 404;
                  res.set_content("not found", "text/plain");
                  return;
                }

                res.set_file_content(abs_path, "video/mp4");
              });

  const std::string host = options_.listen_host + ":" +
                           std::to_string(options_.port);
  if (!server->listen(options_.listen_host.c_str(), options_.port)) {
    last_error_ = "failed to listen on " + host;
  }

  active_server_ = nullptr;
  running_ = false;
}

}  // namespace edger
