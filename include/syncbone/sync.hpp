// sync.hpp - core synchronization interfaces
#pragma once
#include <filesystem>
#include <string>
#include <cstdint>

namespace syncbone {
namespace fs = std::filesystem;

struct SyncStats {
    std::uintmax_t files_copied = 0;
    std::uintmax_t files_skipped = 0;
    std::uintmax_t dirs_created = 0;
};

// Compute SHA-256 of a file. Returns empty string on error.
std::string sha256_file(const fs::path &p);

// Decide if src should be copied over dst (dst may not exist).
bool should_copy_file(const fs::path &src, const fs::path &dst);

// Synchronize directory recursively (copy new/changed files, create dirs). Does not delete extra files.
void sync_directory(const fs::path &source, const fs::path &dest, SyncStats &stats);

// Utility: strip matching leading+trailing quotes.
std::string strip_quotes(std::string_view v);
}
