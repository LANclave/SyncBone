#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <cstdint>

namespace fs = std::filesystem;

namespace {
struct SyncStats {
    std::uintmax_t files_copied = 0;
    std::uintmax_t files_skipped = 0;
    std::uintmax_t dirs_created = 0;
};

bool should_copy_file(const fs::path &src, const fs::path &dst) {
    std::error_code ec;
    if (!fs::exists(dst, ec)) return true;
    if (ec) return true; // conservative
    auto src_size = fs::file_size(src, ec);
    if (ec) return true;
    auto dst_size = fs::file_size(dst, ec);
    if (ec) return true;
    if (src_size != dst_size) return true;
    auto src_time = fs::last_write_time(src, ec);
    if (ec) return true;
    auto dst_time = fs::last_write_time(dst, ec);
    if (ec) return true;
    return src_time > dst_time; // copy if source newer
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