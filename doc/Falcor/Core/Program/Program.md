# Program

## Module Overview

The **Program** module provides shader program management for Falcor engine. This module handles shader compilation, reflection, variable binding, and program versioning, providing a unified interface for shader program management across different graphics APIs.

## Dependency Graph

### Sub-Modules

- [x] DefineList - Define list management
- [x] Program - Shader program class
- [x] ProgramManager - Program manager
- [x] ProgramReflection - Program reflection
- [x] ProgramVars - Program variables
- [x] ProgramVersion - Program version
- [x] RtBindingTable - Raytracing binding table
- [x] ShaderVar - Shader variable

### External Dependencies

- **Core/API**: Depends on API module for graphics abstraction
- **Core/State**: Depends on State module for state management
- **Core/Resource**: Depends on Resource module for GPU resources

### Internal Dependencies

- **Program**: Uses ProgramManager for program management
- **ProgramManager**: Uses ProgramReflection for reflection
- **ProgramVars**: Uses ShaderVar for variable management
- **RtBindingTable**: Uses Program for raytracing programs
- **ShaderVar**: Used by ProgramVars for variable management

## State Machine

### Current State

- **Status**: Complete
- **Total Components**: 8
- **Completed Components**: 8
- **Current Depth**: 2
- **Max Depth Reached**: 2

### Progress Log

- **2026-01-07T18:12:00.000Z**: Module created, awaiting analysis
- **2026-01-07T18:13:00.000Z**: DefineList component analyzed - inherits from std::map, O(log N) operations, poor cache locality
- **2026-01-07T18:16:00.000Z**: Program component analyzed - 768-1024 bytes, lazy compilation, version caching, poor cache locality
- **2026-01-07T18:18:00.000Z**: ProgramManager component analyzed - 216-232 bytes, hot reload, statistics tracking, moderate cache locality
- **2026-01-07T18:22:00.000Z**: ProgramReflection component analyzed - 10 types, offset types, reflection types, excellent cache locality
- **2026-01-07T18:24:00.000Z**: ProgramVars component analyzed - 168-208 bytes, high-level abstraction, shader table management
- **2026-01-07T18:26:00.000Z**: ProgramVersion component analyzed - 4 classes (EntryPointKernel, EntryPointGroupKernels, ProgramKernels, ProgramVersion), immutable after construction, excellent cache locality
- **2026-01-07T18:28:00.000Z**: RtBindingTable component analyzed - 32-40 bytes, flat array layout, O(1) lookup, excellent cache locality
- **2026-01-07T18:29:00.000Z**: ShaderVar component analyzed - 16-24 bytes, pointer-like interface, unowned pointer, excellent cache locality

### Next Action

Proceed to analyze the **State** sub-module of the Core module.
