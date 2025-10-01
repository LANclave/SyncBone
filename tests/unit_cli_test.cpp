// unit_cli_test.cpp - tests for CLI related features (flags like --dry-run)
// Future additions: --fast, --prune, --verbose, invalid flag handling, usage output

#include "syncbone/sync.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace syncbone;
namespace fs = std::filesystem;

static fs::path make_temp_dir(const char* name){
    fs::path p = fs::path("unit_tmp")/name;
    fs::remove_all(p); fs::create_directories(p); return p;
}
static void write_file(const fs::path &p, const std::string &c){ fs::create_directories(p.parent_path()); std::ofstream o(p, std::ios::binary); o<<c; }

static void test_dry_run_directory(){
    auto src_root = make_temp_dir("cli_dry_src");
    write_file(src_root/"a.txt","AAA");
    write_file(src_root/"sub"/"b.txt","BBB");
    fs::path dst_root = fs::path("unit_tmp")/"cli_dry_dst";
    fs::remove_all(dst_root);
    // Determine expected number of files
    std::size_t expected_files = 0; for(auto &e: fs::recursive_directory_iterator(src_root)) if(e.is_regular_file()) ++expected_files;
    SyncStats stats; sync_directory(src_root, dst_root, stats, true); // dry run
    assert(stats.files_copied == expected_files);
    assert(stats.files_skipped == 0);
    assert(!fs::exists(dst_root));
    // Real run should copy exactly those files
    SyncStats real_stats; sync_directory(src_root, dst_root, real_stats, false);
    assert(real_stats.files_copied == expected_files);
    assert(fs::exists(dst_root/"a.txt"));
    assert(fs::exists(dst_root/"sub"/"b.txt"));
}

int main(){
    test_dry_run_directory();
    std::cout << "CLI flag tests passed" << std::endl;
    return 0;
}