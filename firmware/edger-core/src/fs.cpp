#include "edger/util/fs.hpp"

#include <chrono>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>

namespace edger::util {

bool EnsureDirectory(const std::string& path) {
  if (path.empty()) {
    return false;
  }

  std::string current;
  current.reserve(path.size());

  for (size_t i = 0; i < path.size(); ++i) {
    current.push_back(path[i]);
    if (path[i] == '/' || i + 1 == path.size()) {
      if (current.size() <= 1) {
        continue;
      }
      if (mkdir(current.c_str(), 0755) != 0) {
        struct stat st {};
        if (stat(current.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
          return false;
        }
      }
    }
  }

  return true;
}

bool RemoveFile(const std::string& path) {
  return ::remove(path.c_str()) == 0;
}

bool FileExists(const std::string& path) {
  struct stat st {};
  return stat(path.c_str(), &st) == 0;
}

std::int64_t GetFileSize(const std::string& path) {
  struct stat st {};
  if (stat(path.c_str(), &st) != 0) {
    return -1;
  }
  return static_cast<std::int64_t>(st.st_size);
}

std::string JoinPath(const std::string& a, const std::string& b) {
  if (a.empty()) {
    return b;
  }
  if (b.empty()) {
    return a;
  }
  if (a.back() == '/') {
    return a + b;
  }
  return a + "/" + b;
}

namespace {

std::tm LocalTmWithOffset(int day_offset) {
  const auto now = std::chrono::system_clock::now();
  const auto shifted = now + std::chrono::hours(24 * day_offset);
  const std::time_t t = std::chrono::system_clock::to_time_t(shifted);
  std::tm tm {};
  localtime_r(&t, &tm);
  return tm;
}

void AppendFilesRecursive(const std::string& root, const std::string& extension,
                          std::vector<std::string>* out) {
  DIR* dir = opendir(root.c_str());
  if (dir == nullptr) {
    return;
  }

  while (const dirent* entry = readdir(dir)) {
    if (entry->d_name[0] == '.') {
      continue;
    }

    const std::string path = JoinPath(root, entry->d_name);
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) {
      continue;
    }

    if (S_ISDIR(st.st_mode)) {
      AppendFilesRecursive(path, extension, out);
      continue;
    }

    if (extension.empty()) {
      out->push_back(path);
      continue;
    }

    if (path.size() >= extension.size() &&
        path.compare(path.size() - extension.size(), extension.size(),
                     extension) == 0) {
      out->push_back(path);
    }
  }

  closedir(dir);
}

}  // namespace

std::string TimestampForFilename() {
  const std::tm tm = LocalTmWithOffset(0);
  char buf[32];
  std::strftime(buf, sizeof(buf), "%H%M%S", &tm);
  return buf;
}

std::string DateDirectoryName() {
  return DateDirectoryOffset(0);
}

std::string DateDirectoryOffset(int day_offset) {
  const std::tm tm = LocalTmWithOffset(day_offset);
  char buf[16];
  std::strftime(buf, sizeof(buf), "%Y%m%d", &tm);
  return buf;
}

std::vector<std::string> ListFilesRecursive(const std::string& root,
                                            const std::string& extension) {
  std::vector<std::string> files;
  if (!FileExists(root)) {
    return files;
  }
  AppendFilesRecursive(root, extension, &files);
  std::sort(files.begin(), files.end());
  return files;
}

}  // namespace edger::util
