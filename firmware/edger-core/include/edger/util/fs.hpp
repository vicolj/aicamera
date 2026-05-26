#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace edger::util {

bool EnsureDirectory(const std::string& path);
bool RemoveFile(const std::string& path);
bool FileExists(const std::string& path);
std::int64_t GetFileSize(const std::string& path);
std::string JoinPath(const std::string& a, const std::string& b);
std::string TimestampForFilename();
std::string DateDirectoryName();
std::string DateDirectoryOffset(int day_offset);
std::vector<std::string> ListFilesRecursive(const std::string& root,
                                            const std::string& extension);
bool IsSubPath(const std::string& root, const std::string& path);
std::string RelativePath(const std::string& root, const std::string& path);

}  // namespace edger::util
