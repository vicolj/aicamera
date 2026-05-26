#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace edger {

struct RecordingEntry {
  int channel_id = 0;
  std::string channel_name;
  std::string date;
  std::string path;
  std::int64_t created_at_unix = 0;
  std::int64_t size_bytes = 0;
};

class RecordIndex {
 public:
  bool Load(const std::string& index_path);
  bool Save() const;

  void SetPath(const std::string& index_path);
  const std::string& path() const { return index_path_; }

  bool Append(const RecordingEntry& entry);
  bool RemoveByPath(const std::string& path);
  std::vector<RecordingEntry> List(int channel_id,
                                   const std::string& date) const;
  std::vector<RecordingEntry> All() const { return entries_; }
  size_t size() const { return entries_.size(); }

 private:
  std::string index_path_;
  std::vector<RecordingEntry> entries_;
};

struct RetentionPolicy {
  int retention_days = 7;
};

struct RetentionResult {
  int removed_files = 0;
  int removed_index_entries = 0;
};

struct RecordingQuery {
  int channel_id = -1;
  std::int64_t from_unix = 0;
  std::int64_t to_unix = 0;
  std::string date;
};

std::vector<RecordingEntry> QueryRecordings(const RecordIndex& index,
                                            const RecordingQuery& query);

RetentionResult ApplyRetention(const std::string& record_root,
                               const RetentionPolicy& policy,
                               RecordIndex* index);

std::string DefaultIndexPath(const std::string& record_root);

}  // namespace edger
