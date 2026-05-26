#pragma once

#include <string>

namespace edger::web {

bool TokenMatches(const std::string& provided, const std::string& expected);

std::string ExtractBearerToken(const std::string& authorization_header);

std::string ExtractCookieValue(const std::string& cookie_header,
                               const std::string& name);

bool IsAuthorized(const std::string& expected_token,
                  const std::string& authorization_header,
                  const std::string& cookie_header);

}  // namespace edger::web
