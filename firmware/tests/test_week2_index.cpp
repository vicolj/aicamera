#include "edger/record_index.hpp"
#include "edger/util/fs.hpp"

#include <gtest/gtest.h>

TEST(Week2Index, AppendListSaveLoad) {
  const std::string root = "recordings-test/index-case";
  const std::string index_path = edger::DefaultIndexPath(root);
  edger::util::EnsureDirectory(root + "/index");

  edger::RecordIndex index;
  index.SetPath(index_path);

  edger::RecordingEntry entry;
  entry.channel_id = 1;
  entry.channel_name = "cam-2";
  entry.date = "20260526";
  entry.path = root + "/ch1_cam-2/20260526/120000.mp4";
  entry.created_at_unix = 1710000000;
  entry.size_bytes = 1234;
  ASSERT_TRUE(index.Append(entry));

  const auto listed = index.List(1, "20260526");
  ASSERT_EQ(listed.size(), 1u);
  EXPECT_EQ(listed[0].path, entry.path);

  edger::RecordIndex loaded;
  ASSERT_TRUE(loaded.Load(index_path));
  EXPECT_EQ(loaded.size(), 1u);
}
