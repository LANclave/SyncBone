// main.cpp - CLI entry for SyncBone
#include <iostream>
#include <filesystem>
#include <string_view>
#include <system_error>
#include "syncbone/sync.hpp"

namespace fs = std::filesystem;
using syncbone::SyncStats;
using syncbone::sync_directory;
using syncbone::strip_quotes;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: syncbone <source_path> <destination_path>\n"
                     "Synchronize a single file or an entire directory tree (one-way).\n";
        return 1;
    }

    const fs::path source = strip_quotes(argv[1]);
    const fs::path dest   = strip_quotes(argv[2]);

    try {
        if (!fs::exists(source)) {
            std::cerr << "Error: source does not exist: " << source << "\n";
            return 1;
        }

        if (fs::is_directory(source)) {
            if (!fs::exists(dest)) {
                std::error_code ec; fs::create_directories(dest, ec);
                if (ec) {
                    std::cerr << "Error: cannot create destination directory: " << dest
                              << " (" << ec.message() << ")\n"; return 1;
                }
            }
            SyncStats stats; sync_directory(source, dest, stats);
            std::cout << "Synced directory " << source << " -> " << dest
                      << " | copied: " << stats.files_copied
                      << ", skipped: " << stats.files_skipped
                      << ", new dirs: " << stats.dirs_created << "\n";
        } else {
            if (dest.has_parent_path() && !dest.parent_path().empty()) {
                std::error_code ec; fs::create_directories(dest.parent_path(), ec);
                if (ec) {
                    std::cerr << "Error: cannot create destination directory: " << dest.parent_path()
                              << " (" << ec.message() << ")\n"; return 1;
                }
            }
            std::error_code ec; fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);
            if (ec) { std::cerr << "Error: failed to copy file: " << ec.message() << "\n"; return 1; }
            std::cout << "Copied file " << source << " -> " << dest << "\n";
        }
    } catch(const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n"; return 1;
    }
    return 0;
}