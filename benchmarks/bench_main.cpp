#include "syncbone/sync.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

using namespace syncbone;
namespace fs = std::filesystem;

static void write_random_file(const fs::path &p, size_t bytes, std::mt19937_64 &rng) {
    fs::create_directories(p.parent_path());
    std::ofstream o(p, std::ios::binary | std::ios::trunc);
    std::string buf; buf.resize(256*1024);
    size_t written = 0;
    while(written < bytes) {
        for(char &c: buf) c = static_cast<char>(rng() & 0xFF);
        size_t chunk = std::min(buf.size(), bytes - written);
        o.write(buf.data(), chunk);
        written += chunk;
    }
}

static void prepare_dataset(const fs::path &root, size_t files, size_t file_size) {
    std::mt19937_64 rng(123456);
    fs::remove_all(root);
    for(size_t i=0;i<files;++i) {
        fs::path p = root / ("dir" + std::to_string(i % 16)) / ("file_" + std::to_string(i) + ".bin");
        write_random_file(p, file_size, rng);
    }
}

struct RunResult { double seconds{}; SyncStats stats; };

static RunResult run_once(const fs::path &src, const fs::path &dst, const SyncOptions &opt) {
    SyncStats stats; auto t0 = std::chrono::high_resolution_clock::now();
    sync_directory(src, dst, stats, opt);
    auto t1 = std::chrono::high_resolution_clock::now();
    double sec = std::chrono::duration<double>(t1 - t0).count();
    return {sec, stats};
}

int main(int argc, char** argv) {
    size_t files = 200;            // default number of files
    size_t file_size = 128 * 1024; // 128 KB default size
    unsigned threads_multi = 4;    // default multi-thread run

    if(argc > 1) files = static_cast<size_t>(std::stoull(argv[1]));
    if(argc > 2) file_size = static_cast<size_t>(std::stoull(argv[2]));
    if(argc > 3) threads_multi = static_cast<unsigned>(std::stoul(argv[3]));

    fs::path src = "bench_src"; fs::path dst = "bench_dst"; fs::path dst2 = "bench_dst_resync";
    prepare_dataset(src, files, file_size);
    fs::remove_all(dst); fs::remove_all(dst2);

    const double total_mb = (files * file_size) / 1024.0 / 1024.0;
    std::cout << "Benchmark dataset: files="<<files
              << " size_each="<<file_size
              << " bytes total≈" << total_mb << " MB" << std::endl;
    std::cout << "Args: files file_size threads_multi" << std::endl;

    // First real copy single-thread
    SyncOptions opt1; opt1.threads = 1; opt1.verbose = false; opt1.dry_run=false;
    auto r1 = run_once(src,dst,opt1);
    double mbps1 = total_mb / r1.seconds;
    std::cout << "copy threads=1 first: "<< r1.seconds << "s copied="<<r1.stats.files_copied
              << " skipped="<<r1.stats.files_skipped
              << " throughput_MBps="<< mbps1 << std::endl;

    // Resync single-thread (should skip all)
    auto r2 = run_once(src,dst,opt1);
    std::cout << "resync threads=1 skip: "<< r2.seconds << "s copied="<<r2.stats.files_copied
              << " skipped="<<r2.stats.files_skipped << std::endl;

    // Multi-thread first copy (separate destination for fairness)
    SyncOptions optT; optT.threads = threads_multi; auto r3 = run_once(src,dst2,optT);
    double mbpsT = total_mb / r3.seconds;
    std::cout << "copy threads="<<threads_multi<<" first: "<< r3.seconds << "s copied="<<r3.stats.files_copied
              << " skipped="<<r3.stats.files_skipped
              << " throughput_MBps="<< mbpsT << std::endl;

    // Dry-run (hashing + planning only)
    SyncOptions optDry; optDry.dry_run = true; auto rDry = run_once(src,"bench_dry_out", optDry);
    std::cout << "dry-run estimate: "<< rDry.seconds << "s would_copy="<<rDry.stats.files_copied << std::endl;

    std::cout << "CSV: mode,threads,seconds,files_copied,files_skipped,throughput_MBps" << std::endl;
    std::cout << "CSV: first_copy,1,"<<r1.seconds<<","<<r1.stats.files_copied<<","<<r1.stats.files_skipped<<","<<mbps1<<"\n";
    std::cout << "CSV: resync,1,"<<r2.seconds<<","<<r2.stats.files_copied<<","<<r2.stats.files_skipped<<",0\n";
    std::cout << "CSV: first_copy,"<<threads_multi<<","<<r3.seconds<<","<<r3.stats.files_copied<<","<<r3.stats.files_skipped<<","<<mbpsT<<"\n";
    std::cout << "CSV: dry_run,0,"<<rDry.seconds<<","<<rDry.stats.files_copied<<","<<rDry.stats.files_skipped<<",0\n";
    return 0;
}
