#pragma once

#include <string>

namespace edger::http {

bool PostJson(const std::string& url, const std::string& json_body, long* http_code);

}  // namespace edger::http
