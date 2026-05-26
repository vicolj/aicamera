#include "edger/alert_dispatcher.hpp"
#include "edger/alert_svc.hpp"

#include <gtest/gtest.h>

TEST(Week3AlertCooldown, SuppressesDuplicateWithinWindow) {
  edger::AlertConfig config;
  config.cooldown_sec = 10;

  edger::AlertDispatcher dispatcher(config);
  edger::AlertEvent event;
  event.type = "intrusion";
  event.channel_id = 0;

  EXPECT_TRUE(dispatcher.ShouldDispatch(event));
  dispatcher.MarkDispatched(event);
  EXPECT_FALSE(dispatcher.ShouldDispatch(event));
}

TEST(Week3AlertPayload, BuildJson) {
  edger::AlertService service;
  edger::AlertEvent event;
  event.type = "intrusion";
  event.channel_id = 2;
  event.confidence = 0.88f;
  event.timestamp_ms = 123456;

  const std::string payload = service.BuildPayload(event);
  EXPECT_NE(payload.find("\"intrusion\""), std::string::npos);
  EXPECT_NE(payload.find("\"channel_id\":2"), std::string::npos);
}
