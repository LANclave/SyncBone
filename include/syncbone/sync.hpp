/**
 * \file sync.hpp
 * \brief Core synchronization interfaces for SyncBone.
 *
 * Provides functions for one-way recursive directory synchronization based on
 * SHA-256 content comparison (with a quick size pre-check). New files and
 * directories are created as needed; extra entries present only in the
 * destination are NOT removed (no pruning phase yet).
 */
#pragma once
#include <filesystem>
#include <string>
#include <cstdint>

namespace syncbone {
namespace fs = std::filesystem;

/**
 * \struct SyncStats
 * \brief Accumulated statistics for a synchronization run.
 */
struct SyncStats {
    std::uintmax_t files_copied = 0;  //!< Number of files actually copied (new or changed)
    std::uintmax_t files_skipped = 0; //!< Number of files skipped because they were identical
    std::uintmax_t dirs_created = 0;  //!< Number of subdirectories created
};

/** \brief Options controlling synchronization behavior. */
struct SyncOptions {
    bool dry_run = false;   //!< If true, do not modify filesystem; statistics show intended actions.
    bool verbose = false;   //!< If true, log per-file / per-directory actions (or intended actions in dry-run).
    unsigned threads = 1;   //!< >1 enables parallel file processing (hash + copy). 1 = sequential.
};

/**
 * \brief Compute SHA-256 of a file.
 * \param p Path to the file.
 * \return 64-character lowercase hexadecimal string. Empty string on read or IO failure.
 */
std::string sha256_file(const fs::path &p);

/**
 * \brief Decide whether the file at src should be copied over dst.
 * \details Rules:
 *  - If dst does not exist -> copy.
 *  - If sizes differ -> copy.
 *  - Otherwise compute SHA-256 for both; copy if hashes differ or any hash fails to compute.
 */
bool should_copy_file(const fs::path &src, const fs::path &dst);

/**
 * \brief Recursively perform a one-way synchronization of directories.
 * \param source Source directory (must exist).
 * \param dest Destination directory (created if necessary unless dry_run).
 * \param stats Statistics accumulator to update.
 * \param dry_run If true, no filesystem changes are performed; statistics reflect what WOULD happen.
 * \note Files that exist only in dest are not deleted (no pruning).
 */
// Legacy convenience overload (kept for compatibility)
void sync_directory(const fs::path &source, const fs::path &dest, SyncStats &stats, bool dry_run = false);

/**
 * \brief Synchonize directories with explicit options structure.
 */
void sync_directory(const fs::path &source, const fs::path &dest, SyncStats &stats, const SyncOptions &options);

/**
 * \brief Remove surrounding symmetric quotes ("..." or '...') if present.
 * \param v Input string view.
 * \return A copy without outer quotes if both ends match and are quotes; otherwise the original content.
 */
std::string strip_quotes(std::string_view v);
} // namespace syncbone
