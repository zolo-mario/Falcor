# Platform

## Module Overview

The **Platform** module provides platform-specific code and abstractions for the Falcor engine. This module encapsulates platform-dependent functionality including OS operations, file I/O, monitor information, and progress bars, providing a unified interface across different platforms (Windows and Linux).

## Dependency Graph

### Sub-Modules

- [x] LockFile - File locking mechanism
- [x] MemoryMappedFile - Memory-mapped file I/O
- [x] MonitorInfo - Monitor and display information
- [x] OS - Operating system abstractions
- [x] ProgressBar - Progress bar UI
- [x] PlatformHandles - Platform-specific handle types

### External Dependencies

- **Core/API**: Depends on API module for graphics abstraction
- **Core/State**: Depends on State module for state management
- **Core/Resource**: Depends on Resource module for GPU resources

### Internal Dependencies

- **LockFile**: Uses OS module for file operations
- **MemoryMappedFile**: Uses OS module for file operations
- **MonitorInfo**: Uses OS module for monitor queries
- **ProgressBar**: Uses OS module for console output
- **PlatformHandles**: Provides platform-specific handle types

### Platform-Specific Dependencies

- **Windows**: Windows-specific implementations (Windows.cpp, ProgressBarWin.cpp)
- **Linux**: Linux-specific implementations (Linux.cpp, ProgressBarLinux.cpp)

## State Machine

### Current State

- **Status**: Complete
- **Total Components**: 6
- **Completed Components**: 6
- **Current Depth**: 2
- **Max Depth Reached**: 2

### Progress Log

- **2026-01-07T17:56:00.000Z**: Module created, awaiting analysis
- **2026-01-07T17:58:00.000Z**: LockFile component analyzed - 16 bytes (Windows) / 8 bytes (Linux), O(1) operations, cross-platform file locking
- **2026-01-07T17:59:00.000Z**: MemoryMappedFile component analyzed - 88 bytes (Windows) / 80 bytes (Linux), O(1) operations, cross-platform memory mapping
- **2026-01-07T18:01:00.000Z**: MonitorInfo component analyzed - 56 bytes, O(N×M) enumeration, cross-platform monitor info
- **2026-01-07T18:05:00.000Z**: OS component analyzed - comprehensive OS abstraction, 50+ functions, cross-platform support
- **2026-01-07T18:07:00.000Z**: ProgressBar component analyzed - 8 bytes, O(1) operations, RAII pattern, thread management
- **2026-01-07T18:09:00.000Z**: PlatformHandles component analyzed - 16 bytes (Linux WindowHandle), 8 bytes (type aliases), zero-cost abstraction, cross-platform handles

### Next Action

All Platform components complete. Proceed to analyze the next Core sub-module: **Program** or **State**.
