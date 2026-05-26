#include "edger/config.hpp"

#include <gtest/gtest.h>

TEST(Week1Config, LoadExampleJson) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/example.json"));
  EXPECT_EQ(config.app().max_channels, 4);
  EXPECT_EQ(config.app().segment_sec, 300);
  EXPECT_FALSE(config.app().record_root.empty());
  ASSERT_GE(config.channels().size(), 1u);

  const auto* ch = config.FirstEnabledChannel();
  ASSERT_NE(ch, nullptr);
  EXPECT_FALSE(ch->rtsp_url.empty());
}

TEST(Week1Config, SaveAndReload) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/example.json"));

  const std::string temp_path = "recordings-test/tmp-config.json";
  ASSERT_TRUE(config.Save(temp_path));

  edger::Config reloaded;
  ASSERT_TRUE(reloaded.Load(temp_path));
  EXPECT_EQ(reloaded.app().record_root, config.app().record_root);
  EXPECT_EQ(reloaded.channels().size(), config.channels().size());
}
