#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <cstdint>
#include <fstream>
#include <array>

namespace fs = std::filesystem;

namespace {
struct SyncStats {
    std::uintmax_t files_copied = 0;
    std::uintmax_t files_skipped = 0;
    std::uintmax_t dirs_created = 0;
};

// Minimal SHA-256 implementation (public-domain style).
namespace sha256 {
    using u32 = uint32_t;
    using u64 = uint64_t;
    static inline u32 rotr(u32 x, u32 n){ return (x>>n) | (x<<(32-n)); }
    struct Ctx {
        u64 bitlen = 0; // total bits processed
        std::array<u32,8> state { 0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19 };
        std::array<unsigned char,64> buffer{}; // data block buffer
        size_t buffer_len = 0;
    };
    static constexpr u32 K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };
    static void transform(Ctx &ctx, const unsigned char block[64]) {
        u32 w[64];
        for(int i=0;i<16;++i){
            w[i] = (u32(block[i*4])<<24)|(u32(block[i*4+1])<<16)|(u32(block[i*4+2])<<8)|u32(block[i*4+3]);
        }
        for(int i=16;i<64;++i){
            u32 s0 = rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3);
            u32 s1 = rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10);
            w[i] = w[i-16]+s0+w[i-7]+s1;
        }
        u32 a=ctx.state[0], b=ctx.state[1], c=ctx.state[2], d=ctx.state[3];
        u32 e=ctx.state[4], f=ctx.state[5], g=ctx.state[6], h=ctx.state[7];
        for(int i=0;i<64;++i){
            u32 S1 = rotr(e,6)^rotr(e,11)^rotr(e,25);
            u32 ch = (e & f) ^ ((~e) & g);
            u32 temp1 = h + S1 + ch + K[i] + w[i];
            u32 S0 = rotr(a,2)^rotr(a,13)^rotr(a,22);
            u32 maj = (a & b) ^ (a & c) ^ (b & c);
            u32 temp2 = S0 + maj;
            h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
        }
        ctx.state[0]+=a; ctx.state[1]+=b; ctx.state[2]+=c; ctx.state[3]+=d;
        ctx.state[4]+=e; ctx.state[5]+=f; ctx.state[6]+=g; ctx.state[7]+=h;
    }
    static void update(Ctx &ctx, const unsigned char *data, size_t len){
        while(len>0){
            size_t to_copy = std::min<size_t>(64-ctx.buffer_len, len);
            std::memcpy(ctx.buffer.data()+ctx.buffer_len, data, to_copy);
            ctx.buffer_len += to_copy; data += to_copy; len -= to_copy; ctx.bitlen += to_copy * 8ULL;
            if(ctx.buffer_len==64){ transform(ctx, ctx.buffer.data()); ctx.buffer_len=0; }
        }
    }
    static void finalize(Ctx &ctx, unsigned char out[32]){
        // append 0x80
        ctx.buffer[ctx.buffer_len++] = 0x80;
        if(ctx.buffer_len>56){
            while(ctx.buffer_len<64) ctx.buffer[ctx.buffer_len++] = 0;
            transform(ctx, ctx.buffer.data());
            ctx.buffer_len=0;
        }
        while(ctx.buffer_len<56) ctx.buffer[ctx.buffer_len++] = 0;
        // append length big-endian
        for(int i=7;i>=0;--i){ ctx.buffer[ctx.buffer_len++] = (unsigned char)((ctx.bitlen>>(i*8)) & 0xFF); }
        transform(ctx, ctx.buffer.data());
        for(int i=0;i<8;++i){
            out[i*4+0] = (unsigned char)((ctx.state[i]>>24)&0xFF);
            out[i*4+1] = (unsigned char)((ctx.state[i]>>16)&0xFF);
            out[i*4+2] = (unsigned char)((ctx.state[i]>>8)&0xFF);
            out[i*4+3] = (unsigned char)(ctx.state[i]&0xFF);
        }
    }
    static std::string hex(const unsigned char *digest){
        static const char* h = "0123456789abcdef";
        std::string s; s.reserve(64);
        for(int i=0;i<32;++i){ s.push_back(h[digest[i]>>4]); s.push_back(h[digest[i]&0xF]); }
        return s;
    }
}

std::string sha256_file(const fs::path &p) {
    std::ifstream in(p, std::ios::binary);
    if(!in) return {}; // empty indicates error
    sha256::Ctx ctx; std::array<unsigned char,8192> buf{};
    while(in){
        in.read(reinterpret_cast<char*>(buf.data()), buf.size());
        std::streamsize got = in.gcount();
        if(got>0) sha256::update(ctx, buf.data(), static_cast<size_t>(got));
    }
    unsigned char digest[32]; sha256::finalize(ctx, digest);
    return sha256::hex(digest);
}

bool should_copy_file(const fs::path &src, const fs::path &dst) {
    std::error_code ec;
    if (!fs::exists(dst, ec)) return true;
    if (ec) return true;
    // quick size check first
    auto src_size = fs::file_size(src, ec);
    if (ec) return true;
    auto dst_size = fs::file_size(dst, ec);
    if (ec) return true;
    if (src_size != dst_size) return true;
    // compute hashes only if sizes match
    auto h1 = sha256_file(src);
    auto h2 = sha256_file(dst);
    if (h1.empty() || h2.empty()) return true; // if failed to hash, be conservative
    return h1 != h2;
}

void sync_directory(const fs::path &source, const fs::path &dest, SyncStats &stats) {
    for (auto const &entry : fs::recursive_directory_iterator(source)) {
        const auto &src_path = entry.path();
        auto rel = fs::relative(src_path, source);
        fs::path dst_path = dest / rel;
        std::error_code ec;
        if (entry.is_directory()) {
            if (!fs::exists(dst_path)) {
                fs::create_directories(dst_path, ec);
                if (!ec) ++stats.dirs_created;
                else std::cerr << "Warning: cannot create directory " << dst_path << ": " << ec.message() << "\n";
            }
        } else if (entry.is_regular_file()) {
            if (should_copy_file(src_path, dst_path)) {
                fs::create_directories(dst_path.parent_path(), ec);
                ec.clear();
                fs::copy_file(src_path, dst_path, fs::copy_options::overwrite_existing, ec);
                if (ec) {
                    std::cerr << "Warning: failed to copy file " << src_path << " -> " << dst_path << ": " << ec.message() << "\n";
                } else {
                    ++stats.files_copied;
                }
            } else {
                ++stats.files_skipped;
            }
        } else {
            std::cerr << "Skipping non-regular file: " << src_path << "\n";
        }
    }
}

std::string strip_quotes(std::string_view v) {
    if (v.size() >= 2) {
        char a = v.front();
        char b = v.back();
        if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
            v.remove_prefix(1);
            v.remove_suffix(1);
        }
    }
    return std::string(v);
}
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: syncbone <source_path> <destination_path>" << std::endl;
        std::cout << "Synchronize a single file or an entire directory tree (one-way)." << std::endl;
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
                std::error_code ec;
                fs::create_directories(dest, ec);
                if (ec) {
                    std::cerr << "Error: cannot create destination directory: " << dest << " (" << ec.message() << ")\n";
                    return 1;
                }
            }
            SyncStats stats;
            sync_directory(source, dest, stats);
            std::cout << "Synced directory " << source << " -> " << dest
                      << " | copied: " << stats.files_copied
                      << ", skipped: " << stats.files_skipped
                      << ", new dirs: " << stats.dirs_created << "\n";
        } else {
            // Single file mode
            if (dest.has_parent_path() && !dest.parent_path().empty()) {
                std::error_code ec;
                fs::create_directories(dest.parent_path(), ec);
                if (ec) {
                    std::cerr << "Error: cannot create destination directory: " << dest.parent_path() << " (" << ec.message() << ")\n";
                    return 1;
                }
            }
            std::error_code ec;
            fs::copy_file(source, dest, fs::copy_options::overwrite_existing, ec);
            if (ec) {
                std::cerr << "Error: failed to copy file: " << ec.message() << "\n";
                return 1;
            }
            std::cout << "Copied file " << source << " -> " << dest << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}