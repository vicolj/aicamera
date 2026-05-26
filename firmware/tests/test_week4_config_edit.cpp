#include "edger/config.hpp"

#include <gtest/gtest.h>

TEST(Week4ConfigEdit, MutatesChannelsAndRetention) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/example.json"));

  config.mutable_app().retention_days = 14;
  config.mutable_app().max_storage_gb = 128;
  config.mutable_channels().push_back(
      {1, "cam-2", "rtsp://127.0.0.1:8554/live2", true});

  const std::string temp_path = "recordings-test/week4-config-edit.json";
  ASSERT_TRUE(config.Save(temp_path));

  edger::Config reloaded;
  ASSERT_TRUE(reloaded.Load(temp_path));
  EXPECT_EQ(reloaded.app().retention_days, 14);
  EXPECT_EQ(reloaded.app().max_storage_gb, 128);
  EXPECT_EQ(reloaded.channels().size(), config.channels().size());
}

TEST(Week4ConfigEdit, RoundTripsIntrusionPolygon) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/example.json"));

  config.mutable_ai().intrusion.polygon = {
      {0.2f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.2f, 0.8f}};

  const std::string temp_path = "recordings-test/week4-polygon.json";
  ASSERT_TRUE(config.Save(temp_path));

  edger::Config reloaded;
  ASSERT_TRUE(reloaded.Load(temp_path));
  ASSERT_EQ(reloaded.ai().intrusion.polygon.size(), 4u);
  EXPECT_FLOAT_EQ(reloaded.ai().intrusion.polygon[0].first, 0.2f);
  EXPECT_FLOAT_EQ(reloaded.ai().intrusion.polygon[2].second, 0.8f);
}
