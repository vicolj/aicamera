#include "edger/web_auth.hpp"

namespace edger::web {

namespace {

std::string Trim(const std::string& value) {
  const auto start = value.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(start, end - start + 1);
}

}  // namespace

bool TokenMatches(const std::string& provided, const std::string& expected) {
  if (expected.empty()) {
    return true;
  }
  return !provided.empty() && provided == expected;
}

std::string ExtractBearerToken(const std::string& authorization_header) {
  const std::string prefix = "Bearer ";
  if (authorization_header.rfind(prefix, 0) != 0) {
    return "";
  }
  return Trim(authorization_header.substr(prefix.size()));
}

std::string ExtractCookieValue(const std::string& cookie_header,
                               const std::string& name) {
  const std::string key = name + "=";
  size_t pos = 0;
  while (pos < cookie_header.size()) {
    const size_t end = cookie_header.find(';', pos);
    const std::string part = Trim(cookie_header.substr(
        pos, end == std::string::npos ? std::string::npos : end - pos));
    if (part.rfind(key, 0) == 0) {
      return part.substr(key.size());
    }
    if (end == std::string::npos) {
      break;
    }
    pos = end + 1;
  }
  return "";
}

bool IsAuthorized(const std::string& expected_token,
                  const std::string& authorization_header,
                  const std::string& cookie_header) {
  if (expected_token.empty()) {
    return true;
  }

  const std::string bearer = ExtractBearerToken(authorization_header);
  if (TokenMatches(bearer, expected_token)) {
    return true;
  }

  const std::string cookie = ExtractCookieValue(cookie_header, "edger_token");
  return TokenMatches(cookie, expected_token);
}

}  // namespace edger::web
