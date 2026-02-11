---
modified: 2026-01-28T09:38:29+08:00
created: 2026-01-27T21:44:04+08:00
tags:
  - 2026/01/27
---
### [Index](../index.md) | [Usage](./index.md) | Environment Variables

--------

# Environment Variables

The following environment variables are used in Falcor:

| Variable Name          | Description                                                                                                                                                                                                                                                                                                                   |
| ---------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `FALCOR_DEVMODE`       | Set to `1` to enable development mode. In development mode, shader and data files are picked up from the `Source` folder instead of the binary output directory allowing for shader hot reloading (`F5`). Note that this environment variable is set by default when launching any of the Falcor projects from Visual Studio. |
| `FALCOR_MEDIA_FOLDERS` | Specifies a semi-colon (`;`) separated list of absolute path names containing Falcor scenes. Falcor will search in these paths when loading a scene from a relative path name.                                                                                                                                                |

| Environment Variable | Purpose | Usage/Notes |
|---------------------|---------|-------------|
| `FALCOR_MEDIA_FOLDERS` | Controls additional media/data directories for asset searching | Semicolon-separated list of paths [1](#1-0)  |
| `FALCOR_DEVMODE` | Enables development mode | Set to "1" to enable; affects shader directory search paths and development behaviors [2](#1-1)  |
| `FALCOR_EMBEDDED_PYTHON` | Indicates embedded Python interpreter usage | Automatically set to "1" by Falcor when using embedded Python [3](#1-2)  |
| `FALCOR_USE_SLANG_SPIRV_BACKEND` | Controls SPIR-V backend usage for shader compilation | Set to "1" to generate SPIRV directly [4](#1-3)  |
| `HOME` | User home directory (Linux only) | Used to determine location for `~/.falcor` application data [5](#1-4)  |

## Notes
- CMake automatically sets `FALCOR_DEVMODE=1` for debugging purposes in Visual Studio builds [6](#1-5) 
- Falcor provides a cross-platform `getEnvironmentVariable()` function for safe environment variable access [7](#1-6) 
- The testing infrastructure uses separate JSON-based environment configuration, which is distinct from these runtime environment variables [8](#1-7)

### Citations

**File:** Source/Falcor/Core/Platform/OS.cpp (L85-89)
```cpp
    if (auto mediaFolders = getEnvironmentVariable("FALCOR_MEDIA_FOLDERS"))
    {
        auto folders = splitString(*mediaFolders, ";");
        directories.insert(directories.end(), folders.begin(), folders.end());
    }
```

**File:** Source/Falcor/Core/Platform/OS.cpp (L98-105)
```cpp
    static bool devMode = []()
    {
        auto value = getEnvironmentVariable("FALCOR_DEVMODE");
        return value && *value == "1";
    }();

    return devMode;
}
```

**File:** Source/Falcor/Core/Program/ProgramManager.cpp (L679-686)
```cpp
    if (getEnvironmentVariable("FALCOR_USE_SLANG_SPIRV_BACKEND") == "1" || program.mDesc.useSPIRVBackend)
    {
        targetDesc.flags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    }
    else
    {
        targetDesc.flags &= ~SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
    }
```

**File:** Source/Falcor/Core/Platform/Linux/Linux.cpp (L238-243)
```cpp
            const char* homeDir;
            if ((homeDir = getenv("HOME")) == nullptr)
            {
                homeDir = getpwuid(getuid())->pw_dir;
            }
            return std::filesystem::path(homeDir) / ".falcor";
```

**File:** Source/Falcor/Core/Platform/Linux/Linux.cpp (L262-266)
```cpp
std::optional<std::string> getEnvironmentVariable(const std::string& varName)
{
    const char* val = ::getenv(varName.c_str());
    return val != nullptr ? std::string(val) : std::optional<std::string>{};
}
```

**File:** tests/testing/core/environment.py (L52-67)
```python
class Environment:
    '''
    Holds a bunch of variables necessary to run the testing infrastructure.
    '''

    def __init__(self, json_file, build_config):
        '''
        Loads the environment from the JSON file and sets up derived variables.
        '''

        self.project_dir = Path(__file__).parents[3].resolve()
        self.temp_dir = self.project_dir / "tests/temp"

        if json_file == None:
            json_file = self.project_dir / config.DEFAULT_ENVIRONMENT

```
