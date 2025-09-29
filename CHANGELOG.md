# Changelog
All notable changes to this project will be documented in this file.

The format loosely follows Keep a Changelog (https://keepachangelog.com/en/1.1.0/) and Semantic Versioning.

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

[0.0.1]: https://github.com/LANclave/SyncBone/releases/tag/v0.0.1