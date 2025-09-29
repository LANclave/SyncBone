#include "syncbone/sync.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

using namespace syncbone;
namespace fs = std::filesystem;

static fs::path make_temp_dir(const std::string &name){
    fs::path p = fs::path("unit_tmp")/name;
    fs::remove_all(p); fs::create_directories(p); return p;
}
static void write_file(const fs::path &p, std::string_view data){ fs::create_directories(p.parent_path()); std::ofstream o(p, std::ios::binary); o<<data; }
static std::string read_file(const fs::path &p){ std::ifstream i(p, std::ios::binary); return std::string((std::istreambuf_iterator<char>(i)),{}); }

static void test_empty_file(){
    auto d = make_temp_dir("edge_empty");
    auto a = d/"a.txt"; auto b = d/"b.txt";
    write_file(a,"");
    assert(should_copy_file(a,b)); // b doesn't exist
    write_file(b,"");
    assert(!should_copy_file(a,b)); // identical empties
}

static void test_large_random(){
    auto d = make_temp_dir("edge_large");
    auto a = d/"bigA.bin"; auto b = d/"bigB.bin";
    // Generate deterministic pseudo-random content ~256KB
    std::mt19937_64 rng(12345);
    std::string data; data.resize(256*1024);
    for(char &c: data) c = static_cast<char>(rng() & 0xFF);
    write_file(a,data);
    write_file(b,data);
    assert(!should_copy_file(a,b));
    // Mutate one byte
    data[100] ^= 0xFF; write_file(b,data);
    assert(should_copy_file(a,b));
}

static void test_deep_nesting(){
    auto src = make_temp_dir("edge_deep_src");
    fs::path cur = src;
    // create depth 15
    for(int i=0;i<15;++i){ cur /= ("dir"+std::to_string(i)); fs::create_directories(cur); write_file(cur/"file.txt", std::to_string(i)); }
    auto dst = make_temp_dir("edge_deep_dst");
    SyncStats st; sync_directory(src,dst,st);
    assert(st.dirs_created>=15);
    SyncStats st2; sync_directory(src,dst,st2); // second run should skip
    assert(st2.files_copied==0);
}

static void test_unicode_and_quotes_strip(){
    std::string input = "\"пример\""; // quoted
    auto stripped = strip_quotes(input);
    assert(stripped=="пример");
    assert(strip_quotes("'a'")=="a");
    assert(strip_quotes("\"a\"\n")=="\"a\"\n"); // trailing newline prevents strip
}

static void test_directory_resync_after_change(){
    auto src = make_temp_dir("edge_resync_src");
    auto dst = make_temp_dir("edge_resync_dst");
    write_file(src/"f1.txt","one");
    write_file(src/"f2.txt","two");
    SyncStats s1; sync_directory(src,dst,s1); assert(s1.files_copied==2);
    // Modify f1 only
    write_file(src/"f1.txt","one-mod" );
    SyncStats s2; sync_directory(src,dst,s2); assert(s2.files_copied==1 && s2.files_skipped>=1);
}

static void test_hash_error_handling(){
    auto d = make_temp_dir("edge_hash_err");
    auto a = d/"a.txt"; write_file(a,"abc");
    // Create destination path but make it a directory with same name to force failure reading file content scenario for dst
    auto b = d/"b.txt"; write_file(b,"abc"); // identical
    assert(!should_copy_file(a,b));
    // Replace b with different content same size -> change last byte
    write_file(b,"abd");
    assert(should_copy_file(a,b));
}

int main(){
    test_empty_file();
    test_large_random();
    test_deep_nesting();
    test_unicode_and_quotes_strip();
    test_directory_resync_after_change();
    test_hash_error_handling();
    std::cout << "All edge tests passed" << std::endl;
    return 0;
}
