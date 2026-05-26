#include "edger/record_index.hpp"

#include "edger/util/fs.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

namespace edger {

std::string DefaultIndexPath(const std::string& record_root) {
  return util::JoinPath(record_root, "index/recordings.json");
}

void RecordIndex::SetPath(const std::string& index_path) {
  index_path_ = index_path;
}

bool RecordIndex::Load(const std::string& index_path) {
  index_path_ = index_path;
  entries_.clear();

  std::ifstream in(index_path);
  if (!in.is_open()) {
    return true;
  }

  nlohmann::json root;
  try {
    in >> root;
  } catch (const nlohmann::json::exception&) {
    return false;
  }

  if (!root.contains("entries") || !root["entries"].is_array()) {
    return true;
  }

  for (const auto& item : root["entries"]) {
    RecordingEntry entry;
    entry.channel_id = item.value("channel_id", 0);
    entry.channel_name = item.value("channel_name", "");
    entry.date = item.value("date", "");
    entry.path = item.value("path", "");
    entry.created_at_unix = item.value("created_at_unix", 0);
    entry.size_bytes = item.value("size_bytes", 0);
    entries_.push_back(entry);
  }

  return true;
}

bool RecordIndex::Save() const {
  if (index_path_.empty()) {
    return false;
  }

  const auto parent = index_path_.substr(0, index_path_.find_last_of('/'));
  if (!parent.empty() && !util::EnsureDirectory(parent)) {
    return false;
  }

  nlohmann::json entries = nlohmann::json::array();
  for (const auto& entry : entries_) {
    entries.push_back({{"channel_id", entry.channel_id},
                       {"channel_name", entry.channel_name},
                       {"date", entry.date},
                       {"path", entry.path},
                       {"created_at_unix", entry.created_at_unix},
                       {"size_bytes", entry.size_bytes}});
  }

  nlohmann::json root;
  root["version"] = 1;
  root["entries"] = entries;

  std::ofstream out(index_path_);
  if (!out.is_open()) {
    return false;
  }

  out << root.dump(2) << '\n';
  return true;
}

bool RecordIndex::Append(const RecordingEntry& entry) {
  entries_.push_back(entry);
  return Save();
}

bool RecordIndex::RemoveByPath(const std::string& path) {
  const auto old_size = entries_.size();
  entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
                                [&](const RecordingEntry& item) {
                                  return item.path == path;
                                }),
                 entries_.end());
  if (entries_.size() == old_size) {
    return true;
  }
  return Save();
}

std::vector<RecordingEntry> RecordIndex::List(int channel_id,
                                              const std::string& date) const {
  std::vector<RecordingEntry> result;
  for (const auto& entry : entries_) {
    if (entry.channel_id == channel_id &&
        (date.empty() || entry.date == date)) {
      result.push_back(entry);
    }
  }
  return result;
}

std::vector<RecordingEntry> QueryRecordings(const RecordIndex& index,
                                            const RecordingQuery& query) {
  std::vector<RecordingEntry> result;
  for (const auto& entry : index.All()) {
    if (query.channel_id >= 0 && entry.channel_id != query.channel_id) {
      continue;
    }
    if (!query.date.empty() && entry.date != query.date) {
      continue;
    }
    if (query.from_unix > 0 && entry.created_at_unix < query.from_unix) {
      continue;
    }
    if (query.to_unix > 0 && entry.created_at_unix > query.to_unix) {
      continue;
    }
    result.push_back(entry);
  }

  std::sort(result.begin(), result.end(),
            [](const RecordingEntry& a, const RecordingEntry& b) {
              return a.created_at_unix > b.created_at_unix;
            });
  return result;
}

RetentionResult ApplyRetention(const std::string& record_root,
                               const RetentionPolicy& policy,
                               RecordIndex* index) {
  RetentionResult result;

  if (policy.retention_days <= 0) {
    return result;
  }

  const std::string cutoff = util::DateDirectoryOffset(-policy.retention_days);
  const auto files = util::ListFilesRecursive(record_root, ".mp4");

  for (const auto& file : files) {
    const auto pos = file.find_last_of('/');
    if (pos == std::string::npos) {
      continue;
    }

    const auto prev = file.find_last_of('/', pos - 1);
    if (prev == std::string::npos) {
      continue;
    }

    const std::string date_dir = file.substr(prev + 1, pos - prev - 1);
    if (date_dir.size() != 8) {
      continue;
    }

    if (date_dir < cutoff) {
      if (util::RemoveFile(file)) {
        ++result.removed_files;
        if (index != nullptr) {
          if (index->RemoveByPath(file)) {
            ++result.removed_index_entries;
          }
        }
      }
    }
  }

  return result;
}

}  // namespace edger
