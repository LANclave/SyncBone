# Changelog
All notable changes to this project will be documented in this file.

The format loosely follows Keep a Changelog (https://keepachangelog.com/en/1.1.0/) and Semantic Versioning.

## [0.0.2] – 2025-10-02
### Added
- CLI flags: `--dry-run / -n`, `--verbose / -v`, `--threads N` (parallel hashing & copy), `--color / -c` and `--no-color`.
- Parallel file processing (chunked over threads) with sequential fallback for small sets.
- Colored (ANSI) per-file and summary output (optional) including DRY-RUN highlighting.
- Extended statistics: error counter (`errors`) and reporting of partial failures.
- Structured exit codes (0 success, 1 usage, 2 source missing, 3 destination create error, 4 partial failures, 5 unexpected error).
- Improved copy reliability: fallback manual stream copy when `std::filesystem::copy_file` fails (esp. on Windows overwrite edge cases).
- Benchmark harness (`syncbone_bench`) with synthetic dataset generator and timing (single-thread vs multi-thread, resync, dry-run) + CSV-friendly output.

### Changed
- Refactored directory sync into option-driven API (`SyncOptions`) adding verbosity, threading, color control.
- Verbose mode now shows mkdir/copy/skip lines (with DRY-RUN prefix when applicable).
- Hashing / comparison unified for sequential and parallel paths.

### Fixed
- Windows sporadic overwrite failures by adding force copy fallback path.
- Ensured parent directories created before file copy in both sequential & parallel modes.

### Known / Not Yet Implemented
- No pruning of extra files in destination (planned future flag `--prune`).
- No include/exclude pattern filtering.
- No hash result caching across runs.
- Resync performance baseline shows higher overhead dominated by hashing; optimization TBD.

## [0.0.1] – 2025-09-29
### Added
- Initial project scaffolding (CMake, basic CLI, library separation planned)
- Recursive file & directory synchronization (one-way, no pruning)
- SHA-256 based change detection (size pre-check + hash compare)
- Robust Windows overwrite fallback (stream copy when copy_file fails)
- Sync statistics output (files_copied, files_skipped, dirs_created)
- Test suite: integration + unit + edge cases (Unicode, deep nesting, large file, resync logic)
- GitHub Actions CI (Windows & Linux matrix, Debug/Release)
- Doxygen configuration and public API documentation comments
- Test data tree with nested folders, Unicode filename, empty directory

[0.0.2]: https://github.com/LANclave/SyncBone/releases/tag/v0.0.2
[0.0.1]: https://github.com/LANclave/SyncBone/releases/tag/v0.0.1