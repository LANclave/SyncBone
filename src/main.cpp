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
        std::cout << "Usage: syncbone [options] <source_path> <destination_path>\n"
                     "Options:\n"
                     "  --dry-run, -n   Show what would change without modifying anything\n"
                     "Synchronize a single file or an entire directory tree (one-way).\n";
        return 1;
    }

    bool dry_run = false;
    fs::path source;
    fs::path dest;
    for(int i=1;i<argc;++i){
        std::string_view a = argv[i];
        if(a == "--dry-run" || a == "-n") { dry_run = true; continue; }
        if(source.empty()) source = strip_quotes(argv[i]);
        else if(dest.empty()) dest = strip_quotes(argv[i]);
        else {
            std::cerr << "Unexpected extra argument: " << a << "\n";
            return 1;
        }
    }
    if(source.empty() || dest.empty()) {
        std::cerr << "Error: source and destination required.\n";
        return 1;
    }

    try {
        if (!fs::exists(source)) {
            std::cerr << "Error: source does not exist: " << source << "\n";
            return 1;
        }

        if (fs::is_directory(source)) {
            if (!dry_run && !fs::exists(dest)) {
                std::error_code ec; fs::create_directories(dest, ec);
                if (ec) {
                    std::cerr << "Error: cannot create destination directory: " << dest
                              << " (" << ec.message() << ")\n"; return 1;
                }
            }
            SyncStats stats; sync_directory(source, dest, stats, dry_run);
            std::cout << (dry_run?"DRY-RUN: ":"")
                      << "Synced directory " << source << " -> " << dest
                      << " | copied: " << stats.files_copied
                      << ", skipped: " << stats.files_skipped
                      << ", new dirs: " << stats.dirs_created << "\n";
        } else {
            if(dry_run) {
                bool need_copy = true; // single-file path, treat as would copy (no hashing logic reuse here)
                std::cout << "DRY-RUN: Copied file " << source << " -> " << dest << " (simulated)\n";
                return 0;
            }
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