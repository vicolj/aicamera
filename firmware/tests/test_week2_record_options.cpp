#include "edger/record_manager.hpp"

#include <gtest/gtest.h>
#include <mutex>

TEST(Week2RecordOptions, BuildFourChannels) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/test-4ch.json"));

  edger::RecordIndex index;
  std::mutex mutex;
  const auto options =
      edger::BuildRecordOptions(config, &index, &mutex);

  EXPECT_EQ(options.size(), 4u);
  EXPECT_EQ(options[0].channel_id, 0);
  EXPECT_EQ(options[3].channel_id, 3);
  EXPECT_EQ(options[0].output_root, config.app().record_root);
  EXPECT_EQ(options[0].index, &index);
}

TEST(Week2RecordOptions, SkipDisabledChannels) {
  edger::Config config;
  ASSERT_TRUE(config.Load("config/test-4ch.json"));

  edger::RecordIndex index;
  std::mutex mutex;
  const auto options =
      edger::BuildRecordOptions(config, &index, &mutex);
  EXPECT_EQ(options.size(), 4u);
}
