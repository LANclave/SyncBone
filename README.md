# SyncBone

**LAN & USB-first synchronization engine.**  
Offline-first, encrypted, part of the [LANclave](https://github.com/lanclave) ecosystem.

---

## Features (planned)
- Synchronization over **LAN** and **USB drives**  
- **End-to-end encryption** for transferred data  
- Local-first architecture: your files stay on your devices  
- Cross-platform support (Windows / Linux / macOS)

---

## Status
_Early prototyping stage. First commits will focus on core file operations and basic LAN discovery._

---

## Build
Planned toolchain:
- **C++20**
- **CMake**
- **[libsodium](https://github.com/jedisct1/libsodium)** for crypto

Sample build instructions will be added once the first prototype is available.

---

## Part of LANclave
SyncBone is the backbone of the LANclave ecosystem:  
- [NoteMancer](https://github.com/lanclave/notemancer) - local-first notes  
- [BoardForge](https://github.com/lanclave/boardforge) - offline task board  
- [TimeShard](https://github.com/lanclave/timeshard) - local calendar
