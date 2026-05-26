#include "edger/record_index.hpp"
#include "edger/util/fs.hpp"

#include <gtest/gtest.h>

TEST(Week2Retention, DeletesOldDateDirectory) {
  const std::string root = "recordings-test/retention-case";
  const std::string old_date = edger::util::DateDirectoryOffset(-30);
  const std::string file = edger::util::JoinPath(
      edger::util::JoinPath(root, "ch0_cam/"), old_date + "/old.mp4");

  edger::util::EnsureDirectory(edger::util::JoinPath(root, "ch0_cam/" + old_date));
  {
    FILE* fp = fopen(file.c_str(), "wb");
    ASSERT_NE(fp, nullptr);
    fputs("test", fp);
    fclose(fp);
  }
  ASSERT_TRUE(edger::util::FileExists(file));

  edger::RecordIndex index;
  index.SetPath(edger::DefaultIndexPath(root));
  edger::RecordingEntry entry;
  entry.channel_id = 0;
  entry.channel_name = "cam";
  entry.date = old_date;
  entry.path = file;
  entry.created_at_unix = 1;
  entry.size_bytes = 4;
  ASSERT_TRUE(index.Append(entry));

  edger::RetentionPolicy policy;
  policy.retention_days = 7;
  const auto result = edger::ApplyRetention(root, policy, &index);
  EXPECT_EQ(result.removed_files, 1);
  EXPECT_FALSE(edger::util::FileExists(file));
  EXPECT_EQ(index.size(), 0u);
}
