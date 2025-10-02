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
    bool verbose = false;
    unsigned threads = 1;
    bool color = false;
    fs::path source;
    fs::path dest;
    for(int i=1;i<argc;++i){
        std::string_view a = argv[i];
    if(a == "--dry-run" || a == "-n") { dry_run = true; continue; }
        if(a == "--verbose" || a == "-v") { verbose = true; continue; }
        if(a == "--threads") {
            if(i+1>=argc) { std::cerr << "Error: --threads requires a number" << "\n"; return 1; }
            std::string val = argv[++i];
            try { int n = std::stoi(val); if(n<=0){ std::cerr << "Error: threads must be >0"<<"\n"; return 1;} threads = static_cast<unsigned>(n); }
            catch(...) { std::cerr << "Error: invalid threads value"<<"\n"; return 1; }
            continue;
        }
        if(a == "--color" || a == "-c") { color = true; continue; }
        if(a == "--no-color") { color = false; continue; }
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
            syncbone::SyncOptions opts; opts.dry_run=dry_run; opts.verbose=verbose; opts.threads=threads; opts.color=color;
            SyncStats stats; sync_directory(source, dest, stats, opts);
            const char* C_RESET = (color?"\x1b[0m":"");
            const char* C_HDR   = (color?"\x1b[1m":"");
            const char* C_NUM   = (color?"\x1b[32m":"");
            const char* C_SKIP  = (color?"\x1b[33m":"");
            const char* C_DIR   = (color?"\x1b[36m":"");
            std::cout << (dry_run?(color?"\x1b[35mDRY-RUN:\x1b[0m ":"DRY-RUN: "):"")
                      << C_HDR << "Synced directory" << C_RESET << " " << source << " -> " << dest
                      << " | copied: " << C_NUM << stats.files_copied << C_RESET
                      << ", skipped: " << C_SKIP << stats.files_skipped << C_RESET
                      << ", new dirs: " << C_DIR << stats.dirs_created << C_RESET << "\n";
        } else {
            if(dry_run) {
                if(color) std::cout << "\x1b[35mDRY-RUN:\x1b[0m " << (verbose?"\x1b[32mcopy file\x1b[0m ":"Copied file ") << source << " -> " << dest << " (simulated)\n"; else
                    std::cout << (verbose?"DRY-RUN: copy file ":"DRY-RUN: Copied file ") << source << " -> " << dest << " (simulated)\n";
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