#include "edger/web_auth.hpp"

#include <gtest/gtest.h>

TEST(Week5WebAuth, AcceptsBearerToken) {
  EXPECT_TRUE(edger::web::IsAuthorized("secret", "Bearer secret", ""));
  EXPECT_FALSE(edger::web::IsAuthorized("secret", "Bearer wrong", ""));
}

TEST(Week5WebAuth, AcceptsCookieToken) {
  EXPECT_TRUE(
      edger::web::IsAuthorized("secret", "", "edger_token=secret; Path=/"));
}

TEST(Week5WebAuth, AllowsEmptyExpectedToken) {
  EXPECT_TRUE(edger::web::IsAuthorized("", "", ""));
}
