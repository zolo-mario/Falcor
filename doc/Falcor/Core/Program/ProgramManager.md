# ProgramManager

## Module Overview

The **ProgramManager** class provides centralized shader program management for Falcor engine. This class handles shader compilation, program versioning, hot reload, global defines, compiler flags, and compilation statistics, providing a unified interface for program management across different graphics APIs.

## Files

- **Header**: `Source/Falcor/Core/Program/ProgramManager.h` (163 lines)
- **Implementation**: `Source/Falcor/Core/Program/ProgramManager.cpp` (864 lines)

## Class Definition

```cpp
class FALCOR_API ProgramManager
{
public:
    ProgramManager(Device* pDevice);

    struct ForcedCompilerFlags
    {
        SlangCompilerFlags enabled = SlangCompilerFlags::None;
        SlangCompilerFlags disabled = SlangCompilerFlags::None;
    };

    struct CompilationStats
    {
        size_t programVersionCount = 0;
        size_t programKernelsCount = 0;
        double programVersionMaxTime = 0.0;
        double programKernelsMaxTime = 0.0;
        double programVersionTotalTime = 0.0;
        double programKernelsTotalTime = 0.0;
    };

    ProgramDesc applyForcedCompilerFlags(ProgramDesc desc) const;
    void registerProgramForReload(Program* program);
    void unregisterProgramForReload(Program* program);

    ref<const ProgramVersion> createProgramVersion(const Program& program, std::string& log) const;

    ref<const ProgramKernels> createProgramKernels(
        const Program& program,
        const ProgramVersion& programVersion,
        const ProgramVars& programVars,
        std::string& log
    ) const;

    ref<const EntryPointGroupKernels> createEntryPointGroupKernels(
        const std::vector<ref<EntryPointKernel>>& kernels,
        const ref<EntryPointBaseReflection>& pReflector
    ) const;

    std::string getHlslLanguagePrelude() const;
    void setHlslLanguagePrelude(const std::string& prelude);

    bool reloadAllPrograms(bool forceReload = false);

    void addGlobalDefines(const DefineList& defineList);
    void removeGlobalDefines(const DefineList& defineList);

    void setGlobalCompilerArguments(const std::vector<std::string>& args);
    const std::vector<std::string>& getGlobalCompilerArguments() const;

    void setGenerateDebugInfoEnabled(bool enabled);
    bool isGenerateDebugInfoEnabled();

    void setForcedCompilerFlags(ForcedCompilerFlags forcedCompilerFlags);
    ForcedCompilerFlags getForcedCompilerFlags();

    const CompilationStats& getCompilationStats();
    void resetCompilationStats();

private:
    SlangCompileRequest* createSlangCompileRequest(const Program& program) const;

    Device* mpDevice;
    std::vector<Program*> mLoadedPrograms;
    mutable CompilationStats mCompilationStats;
    DefineList mGlobalDefineList;
    std::vector<std::string> mGlobalCompilerArguments;
    bool mGenerateDebugInfo = false;
    ForcedCompilerFlags mForcedCompilerFlags;
    mutable uint32_t mHitGroupID = 0;
};
```

## Dependencies

### Internal Dependencies

- **Device**: Uses [`Device`](Source/Falcor/Core/API/Device.h) for GPU device
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h) for program management
- **ProgramVersion**: Uses [`ProgramVersion`](Source/Falcor/Core/Program/ProgramVersion.h) for program versioning
- **ProgramKernels**: Uses [`ProgramKernels`](Source/Falcor/Core/Program/ProgramVersion.h) for program kernels
- **EntryPointKernel**: Uses [`EntryPointKernel`](Source/Falcor/Core/Program/ProgramVersion.h) for entry point kernels
- **EntryPointGroupKernels**: Uses [`EntryPointGroupKernels`](Source/Falcor/Core/Program/ProgramVersion.h) for entry point group kernels
- **EntryPointBaseReflection**: Uses [`EntryPointBaseReflection`](Source/Falcor/Core/Program/ProgramReflection.h) for entry point reflection
- **ProgramReflection**: Uses [`ProgramReflection`](Source/Falcor/Core/Program/ProgramReflection.h) for program reflection

### External Dependencies

- **Slang**: Uses Slang compiler for shader compilation
- **std::vector**: Uses std::vector for container storage
- **std::string**: Uses std::string for string storage
- **DefineList**: Uses [`DefineList`](Source/Falcor/Core/Program/DefineList.h) for define management
- **ProgramDesc**: Uses [`ProgramDesc`](Source/Falcor/Core/Program/Program.h:156) for program description

## Usage Patterns

### Creating a ProgramManager

```cpp
// Create a program manager (typically done by Device)
auto programManager = device->getProgramManager();
```

### Setting Global Defines

```cpp
// Set global defines
programManager->addGlobalDefines(DefineList()
    .add("USE_LIGHTING", "1")
    .add("MAX_LIGHTS", "16")
);
```

### Setting Global Compiler Arguments

```cpp
// Set global compiler arguments
programManager->setGlobalCompilerArguments({"-O3", "-Wall"});
```

### Setting Forced Compiler Flags

```cpp
// Set forced compiler flags
ProgramManager::ForcedCompilerFlags flags;
flags.enabled = SlangCompilerFlags::GenerateDebugInfo;
flags.disabled = SlangCompilerFlags::TreatWarningsAsErrors;
programManager->setForcedCompilerFlags(flags);
```

### Enabling Debug Info

```cpp
// Enable debug info generation
programManager->setGenerateDebugInfoEnabled(true);
```

### Getting Compilation Statistics

```cpp
// Get compilation statistics
auto stats = programManager->getCompilationStats();
std::cout << "Program versions compiled: " << stats.programVersionCount << std::endl;
std::cout << "Program kernels compiled: " << stats.programKernelsCount << std::endl;
std::cout << "Max program version time: " << stats.programVersionMaxTime << " s" << std::endl;
std::cout << "Max program kernels time: " << stats.programKernelsMaxTime << " s" << std::endl;
```

### Resetting Compilation Statistics

```cpp
// Reset compilation statistics
programManager->resetCompilationStats();
```

### Reloading All Programs

```cpp
// Reload all programs
bool reloaded = programManager->reloadAllPrograms();
if (reloaded)
{
    std::cout << "Programs reloaded" << std::endl;
}
```

### Setting HLSL Language Prelude

```cpp
// Set HLSL language prelude
programManager->setHlslLanguagePrelude("#define MY_DEFINE 1\n");
```

### Getting HLSL Language Prelude

```cpp
// Get HLSL language prelude
auto prelude = programManager->getHlslLanguagePrelude();
```

## Summary

**ProgramManager** is a complex class that provides centralized shader program management for Falcor engine:

### ForcedCompilerFlags Struct
- **Memory Layout**: 8 bytes
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Excellent (fits within a single cache line)
- **Algorithmic Complexity**: O(1) for all operations

### CompilationStats Struct
- **Memory Layout**: 48 bytes
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Excellent (fits within a single cache line)
- **Algorithmic Complexity**: O(1) for all operations

### ProgramManager Class
- **Memory Layout**: ~216-232 bytes + heap allocations
- **Threading Model**: Not thread-safe for concurrent writes
- **Cache Locality**: Moderate (some members are heap-allocated)
- **Algorithmic Complexity**: O(N + M + K + C + R) for createProgramVersion, O(G × E × T + C + R + K) for createProgramKernels, O(N × F) for reloadAllPrograms

### Module Characteristics
- Singleton pattern (one ProgramManager per device)
- Factory pattern (creates ProgramVersion and ProgramKernels objects)
- Observer pattern (registers programs for hot reload notifications)
- Statistics tracking (tracks compilation statistics for performance monitoring)
- Global configuration (provides global defines, compiler arguments, and compiler flags)
- Hot reload support (file time tracking enables hot reload)
- Slang integration (integrates with Slang compiler for shader compilation)
- Moderate cache locality (some members are heap-allocated)
- Not thread-safe for concurrent writes
- Mutable state requires external synchronization

The class provides a comprehensive shader program management system with hot reload support, global configuration, and statistics tracking, but suffers from moderate cache locality and is not thread-safe for concurrent writes.
