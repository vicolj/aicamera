#include "edger/config_reload.hpp"

#include <cstdio>

#include <gtest/gtest.h>

TEST(Week4ConfigReload, WritesAndReadsGeneration) {
  const std::string config_path = "recordings-test/week4-reload-config.json";
  const std::string marker_path = edger::ConfigReload::MarkerPath(config_path);

  std::remove(marker_path.c_str());

  EXPECT_EQ(edger::ConfigReload::ReadGeneration(config_path), 0u);
  ASSERT_TRUE(edger::ConfigReload::Notify(config_path));

  const auto generation = edger::ConfigReload::ReadGeneration(config_path);
  EXPECT_GT(generation, 0u);
}
