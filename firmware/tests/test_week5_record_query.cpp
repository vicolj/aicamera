#include "edger/record_index.hpp"

#include <gtest/gtest.h>

namespace {

edger::RecordIndex BuildSampleIndex() {
  edger::RecordIndex index;
  index.SetPath("recordings-test/week5-index.json");

  edger::RecordingEntry a;
  a.channel_id = 0;
  a.channel_name = "cam-1";
  a.date = "20250501";
  a.path = "/tmp/ch0/20250501/a.mp4";
  a.created_at_unix = 1000;
  a.size_bytes = 100;

  edger::RecordingEntry b = a;
  b.channel_id = 1;
  b.channel_name = "cam-2";
  b.date = "20250502";
  b.path = "/tmp/ch1/20250502/b.mp4";
  b.created_at_unix = 2000;

  edger::RecordingEntry c = a;
  c.date = "20250502";
  c.path = "/tmp/ch0/20250502/c.mp4";
  c.created_at_unix = 3000;

  index.Append(a);
  index.Append(b);
  index.Append(c);
  return index;
}

}  // namespace

TEST(Week5RecordQuery, FiltersByChannelAndDate) {
  const edger::RecordIndex index = BuildSampleIndex();

  edger::RecordingQuery query;
  query.channel_id = 0;
  query.date = "20250502";
  const auto entries = edger::QueryRecordings(index, query);

  ASSERT_EQ(entries.size(), 1u);
  EXPECT_EQ(entries[0].created_at_unix, 3000);
}

TEST(Week5RecordQuery, FiltersByTimeRange) {
  const edger::RecordIndex index = BuildSampleIndex();

  edger::RecordingQuery query;
  query.from_unix = 1500;
  query.to_unix = 2500;
  const auto entries = edger::QueryRecordings(index, query);

  ASSERT_EQ(entries.size(), 1u);
  EXPECT_EQ(entries[0].channel_id, 1);
}
