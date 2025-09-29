#include "syncbone/sync.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace syncbone;
namespace fs = std::filesystem;

static fs::path make_temp_dir(const char* name){
    fs::path p = fs::path("unit_tmp")/name;
    fs::remove_all(p); fs::create_directories(p); return p;
}

static void write_file(const fs::path &p, const std::string &content){
    fs::create_directories(p.parent_path());
    std::ofstream o(p, std::ios::binary); o<<content; o.close();
}

static std::string read_file(const fs::path &p){ std::ifstream i(p, std::ios::binary); return std::string((std::istreambuf_iterator<char>(i)),{}); }

static void test_strip_quotes(){
    assert(strip_quotes("\"abc\"")=="abc");
    assert(strip_quotes("'abc'")=="abc");
    assert(strip_quotes("abc")=="abc");
}

static void test_sha_and_should_copy(){
    auto d = make_temp_dir("hash");
    auto a = d/"a.txt"; auto b = d/"b.txt"; write_file(a,"hello"); write_file(b,"hello");
    assert(!should_copy_file(a,b)); // same content no copy
    write_file(b,"HELLO");
    assert(should_copy_file(a,b)); // different
}

static void test_sync_directory(){
    auto src = make_temp_dir("src");
    write_file(src/"dir1"/"f1.txt","one");
    write_file(src/"dir1"/"f2.txt","two");
    write_file(src/"dir2"/"f3.txt","three");
    auto dst = make_temp_dir("dst");
    SyncStats stats; sync_directory(src,dst,stats);
    assert(stats.files_copied==3);
    assert(fs::exists(dst/"dir1"/"f1.txt"));
    assert(fs::exists(dst/"dir2"/"f3.txt"));

    // Modify one file and resync
    write_file(src/"dir1"/"f2.txt","two-mod");
    SyncStats stats2; sync_directory(src,dst,stats2);
    assert(stats2.files_copied==1);
    assert(stats2.files_skipped>=2);
}

int main(){
    test_strip_quotes();
    test_sha_and_should_copy();
    test_sync_directory();
    std::cout << "All unit tests passed" << std::endl;
    return 0;
}
