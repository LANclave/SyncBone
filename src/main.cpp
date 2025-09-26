#include <iostream>
#include <filesystem>
#include <string>
 #include <string_view>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: syncbone <source> <destination>" << std::endl;
        return 1;
    }

    auto strip_quotes = [](std::string_view v) -> std::string {
        if (v.size() >= 2) {
            char a = v.front();
            char b = v.back();
            if ((a == '"' && b == '"') || (a == '\'' && b == '\'')) {
                v.remove_prefix(1);
                v.remove_suffix(1);
            }
        }
        return std::string(v);
    };

    const fs::path source = strip_quotes(argv[1]);
    const fs::path dest   = strip_quotes(argv[2]);

    try {
        if (!fs::exists(source)) {
            std::cerr << "Error: source does not exist: " << source << "\n";
            return 1;
        }
        if (dest.has_parent_path() && !dest.parent_path().empty()) {
            std::error_code ec;
            fs::create_directories(dest.parent_path(), ec);
            if (ec) {
                std::cerr << "Error: cannot create destination directory: " << dest.parent_path() << " (" << ec.message() << ")\n";
                return 1;
            }
        }

        fs::copy_options opts = fs::copy_options::overwrite_existing;
        if (fs::is_directory(source)) {
            opts |= fs::copy_options::recursive;
        }

        fs::copy(source, dest, opts);
    std::cout << "Copied " << source << " -> " << dest << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}