#include "edger/http_client.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <sstream>

namespace edger::http {

namespace {

struct ParsedUrl {
  std::string host;
  std::string path;
  int port = 80;
};

bool ParseUrl(const std::string& url, ParsedUrl* out) {
  const std::string prefix = "http://";
  if (url.rfind(prefix, 0) != 0) {
    return false;
  }

  const std::string rest = url.substr(prefix.size());
  const auto slash = rest.find('/');
  const std::string host_port =
      slash == std::string::npos ? rest : rest.substr(0, slash);
  out->path = slash == std::string::npos ? "/" : rest.substr(slash);

  const auto colon = host_port.find(':');
  if (colon == std::string::npos) {
    out->host = host_port;
    out->port = 80;
  } else {
    out->host = host_port.substr(0, colon);
    out->port = std::stoi(host_port.substr(colon + 1));
  }

  return !out->host.empty();
}

}  // namespace

bool PostJson(const std::string& url, const std::string& json_body,
              long* http_code) {
  if (http_code != nullptr) {
    *http_code = 0;
  }

  ParsedUrl parsed;
  if (!ParseUrl(url, &parsed)) {
    return false;
  }

  addrinfo hints {};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* result = nullptr;
  if (getaddrinfo(parsed.host.c_str(), std::to_string(parsed.port).c_str(),
                  &hints, &result) != 0) {
    return false;
  }

  int sock = -1;
  for (addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sock < 0) {
      continue;
    }
    if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    close(sock);
    sock = -1;
  }
  freeaddrinfo(result);

  if (sock < 0) {
    return false;
  }

  std::ostringstream req;
  req << "POST " << parsed.path << " HTTP/1.1\r\n"
      << "Host: " << parsed.host << "\r\n"
      << "Content-Type: application/json\r\n"
      << "Connection: close\r\n"
      << "Content-Length: " << json_body.size() << "\r\n\r\n"
      << json_body;

  const std::string request = req.str();
  if (send(sock, request.data(), request.size(), 0) < 0) {
    close(sock);
    return false;
  }

  char buffer[512];
  const ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
  close(sock);
  if (n <= 0) {
    return false;
  }

  buffer[n] = '\0';
  long code = 0;
  if (std::sscanf(buffer, "HTTP/1.%*d %ld", &code) == 1 && http_code != nullptr) {
    *http_code = code;
  }

  return code >= 200 && code < 300;
}

}  // namespace edger::http
