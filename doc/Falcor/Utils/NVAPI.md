# NVAPI - NVIDIA API Conditional Compilation

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/Macros.h (FALCOR_NVAPI_AVAILABLE)
- nvapi/nvHLSLExtns.h (NVAPI HLSL extensions)
- nvapi/nvShaderExtnEnums.h (NVAPI shader extension enums)

### Dependent Modules

- Falcor/Utils (Internal utilities)
- Falcor/Rendering (Rendering utilities)
- Falcor/Shaders (Shader utilities)

## Module Overview

NVAPI provides conditional compilation support for NVIDIA API (NVAPI) extensions in Slang shaders. It includes NVAPI headers only when FALCOR_NVAPI_AVAILABLE is defined, allowing the codebase to work with and without NVAPI support.

### Key Characteristics

- **Conditional Compilation**: Only includes NVAPI headers when available
- **Slang Integration**: Provides NVAPI extensions for Slang shaders
- **Minimal Wrapper**: Simple wrapper with no additional functionality
- **Platform-Specific**: NVAPI is NVIDIA-specific
- **Optional Feature**: NVAPI support is optional

### Architecture Principles

1. **Conditional Compilation**: Conditional compilation based on availability
2. **Minimal Wrapper**: Simple wrapper with no additional functionality
3. **Slang Integration**: Provides NVAPI extensions for Slang shaders
4. **Platform-Specific**: NVAPI is NVIDIA-specific
5. **Optional Feature**: NVAPI support is optional

## Component Specifications

### NVAPI.slang File

**File**: `NVAPI.slang` (lines 1-30)

```slang
/***************************************************************************
 # Copyright (c) 2015-22, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer
 #    in the documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

**Purpose**: Conditional compilation wrapper for NVAPI HLSL extensions

**Behavior**:
1. If FALCOR_NVAPI_AVAILABLE is defined:
   - Include nvapi/nvHLSLExtns.h
2. Otherwise:
   - Skip include

**Characteristics**:
- Conditional compilation
- NVAPI HLSL extensions
- Minimal wrapper
- No additional functionality

### NVAPI.slangh File

**File**: `NVAPI.slangh` (lines 1-32)

```slang
/***************************************************************************
 # Copyright (c) 2015-22, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer
 #    in the documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvShaderExtnEnums.h"
#endif
import Utils.NVAPI;
```

**Purpose**: Conditional compilation wrapper for NVAPI shader extension enums

**Behavior**:
1. If FALCOR_NVAPI_AVAILABLE is defined:
   - Include nvapi/nvShaderExtnEnums.h
2. Otherwise:
   - Skip include
3. Import Utils.NVAPI module

**Characteristics**:
- Conditional compilation
- NVAPI shader extension enums
- Minimal wrapper
- Imports NVAPI module

## Technical Details

### Conditional Compilation

Conditional compilation based on FALCOR_NVAPI_AVAILABLE:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

**Characteristics**:
- Preprocessor conditional
- Only includes when available
- Allows codebase to work without NVAPI
- Platform-specific

### NVAPI HLSL Extensions

NVAPI HLSL extensions for Slang shaders:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

**Characteristics**:
- NVIDIA-specific HLSL extensions
- Additional shader functionality
- Vendor-specific features
- Optional feature

### NVAPI Shader Extension Enums

NVAPI shader extension enums for Slang shaders:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvShaderExtnEnums.h"
#endif
```

**Characteristics**:
- NVIDIA-specific shader extension enums
- Additional shader functionality
- Vendor-specific features
- Optional feature

### Module Import

Imports Utils.NVAPI module:

```slang
import Utils.NVAPI;
```

**Characteristics**:
- Slang module import
- Imports NVAPI functionality
- Allows use in Slang shaders
- Conditional compilation

## Integration Points

### Falcor Core Integration

- **Falcor/Core/Macros.h**: FALCOR_NVAPI_AVAILABLE

### NVAPI Integration

- **nvapi/nvHLSLExtns.h**: NVAPI HLSL extensions
- **nvapi/nvShaderExtnEnums.h**: NVAPI shader extension enums

### Internal Falcor Usage

- **Falcor/Utils**: Internal utilities
- **Falcor/Rendering**: Rendering utilities
- **Falcor/Shaders**: Shader utilities

## Architecture Patterns

### Conditional Compilation Pattern

Conditional compilation based on availability:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

Benefits:
- Allows codebase to work without NVAPI
- Platform-specific features
- Optional feature support
- Clean code

### Minimal Wrapper Pattern

Simple wrapper with no additional functionality:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

Benefits:
- Simple implementation
- No additional functionality
- Easy to maintain
- Minimal overhead

### Module Import Pattern

Imports NVAPI module for Slang shaders:

```slang
import Utils.NVAPI;
```

Benefits:
- Allows use in Slang shaders
- Provides NVAPI functionality
- Conditional compilation
- Clean interface

## Code Patterns

### Conditional Compilation Pattern

Conditional compilation based on availability:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

Pattern:
- Check preprocessor macro
- Include header if available
- Skip include if not available
- Allows codebase to work without feature

### Minimal Wrapper Pattern

Simple wrapper with no additional functionality:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

Pattern:
- Check preprocessor macro
- Include header if available
- No additional functionality
- Minimal implementation

### Module Import Pattern

Imports NVAPI module for Slang shaders:

```slang
import Utils.NVAPI;
```

Pattern:
- Import module
- Provides functionality
- Conditional compilation
- Clean interface

## Use Cases

### Conditional NVAPI Support

```slang
// In Slang shader
#if FALCOR_NVAPI_AVAILABLE
// Use NVAPI-specific features
#include "nvapi/nvHLSLExtns.h"
#endif

// Use standard features
// ...
```

### NVAPI HLSL Extensions

```slang
// In Slang shader
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"

// Use NVAPI HLSL extensions
// ...
#endif
```

### NVAPI Shader Extension Enums

```slang
// In Slang shader
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvShaderExtnEnums.h"

// Use NVAPI shader extension enums
// ...
#endif
```

### Module Import

```slang
// In Slang shader
import Utils.NVAPI;

// Use NVAPI functionality
// ...
```

## Performance Considerations

### Compilation Overhead

- **Preprocessor Check**: Preprocessor conditional check
- **Include Overhead**: Include overhead (minimal)
- **Module Import**: Module import overhead (minimal)

### Runtime Overhead

- **No Runtime Overhead**: No runtime overhead
- **Conditional Compilation**: Compile-time only
- **No Branching**: No runtime branching

### Compiler Optimizations

- **Conditional Compilation**: Conditional compilation eliminates unused code
- **Dead Code Elimination**: Unused code eliminated
- **Include Optimization**: Include optimization

### Comparison with Alternatives

**NVAPI vs Direct Include**:
- NVAPI: Conditional compilation, optional feature
- Direct Include: Always includes, no optional support

## Limitations

### Feature Limitations

- **No Additional Functionality**: No additional functionality beyond wrapper
- **No Error Handling**: No error handling
- **No Validation**: No validation of NVAPI availability
- **No Fallback**: No fallback for missing NVAPI

### API Limitations

- **No NVAPI Abstraction**: No NVAPI abstraction layer
- **No Feature Detection**: No feature detection
- **No Version Check**: No version checking
- **No Error Handling**: No error handling

### Platform Limitations

- **NVIDIA Only**: NVAPI is NVIDIA-specific
- **NVAPI Required**: Requires NVAPI installation
- **Windows Only**: NVAPI is Windows-only

### Performance Limitations

- **Include Overhead**: Include overhead (minimal)
- **Module Import Overhead**: Module import overhead (minimal)

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Conditional NVAPI support in Slang shaders
- Optional NVAPI features
- Platform-specific NVAPI features
- Vendor-specific shader extensions

**Inappropriate Use Cases**:
- Cross-vendor code (use standard features)
- Non-NVIDIA hardware (use standard features)
- Non-Windows platforms (use standard features)

### Usage Patterns

**Conditional NVAPI Support**:
```slang
#if FALCOR_NVAPI_AVAILABLE
// Use NVAPI-specific features
#endif
```

**Module Import**:
```slang
import Utils.NVAPI;
```

### Platform Considerations

- **NVIDIA Only**: NVAPI is NVIDIA-specific
- **Windows Only**: NVAPI is Windows-only
- **Optional Feature**: NVAPI support is optional

### Cross-Vendor Considerations

- **Fallback**: Provide fallback for non-NVIDIA hardware
- **Standard Features**: Use standard features for cross-vendor code
- **Feature Detection**: Detect NVAPI availability

## Implementation Notes

### Conditional Compilation Implementation

Conditional compilation based on FALCOR_NVAPI_AVAILABLE:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

**Characteristics**:
- Preprocessor conditional
- Only includes when available
- Allows codebase to work without NVAPI
- Platform-specific

### Minimal Wrapper Implementation

Simple wrapper with no additional functionality:

```slang
#if FALCOR_NVAPI_AVAILABLE
#include "nvapi/nvHLSLExtns.h"
#endif
```

**Characteristics**:
- Simple implementation
- No additional functionality
- Easy to maintain
- Minimal overhead

### Module Import Implementation

Imports NVAPI module for Slang shaders:

```slang
import Utils.NVAPI;
```

**Characteristics**:
- Slang module import
- Imports NVAPI functionality
- Conditional compilation
- Clean interface

## Future Enhancements

### Potential Improvements

1. **Feature Detection**: Add feature detection
2. **Version Check**: Add version checking
3. **Fallback**: Add fallback for missing NVAPI
4. **Error Handling**: Add error handling
5. **Validation**: Add validation of NVAPI availability
6. **Cross-Platform**: Add cross-platform support
7. **More NVAPI Features**: Add more NVAPI features

### API Extensions

1. **Feature Detection**: Add feature detection
2. **Version Check**: Add version checking
3. **Fallback**: Add fallback for missing NVAPI
4. **Error Handling**: Add error handling
5. **Validation**: Add validation of NVAPI availability
6. **Cross-Platform**: Add cross-platform support
7. **More NVAPI Features**: Add more NVAPI features

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **NVAPI Documentation**: Add NVAPI documentation
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from direct include
5. **API Reference**: Add complete API reference

## References

### NVAPI Documentation

- **nvapi/nvHLSLExtns.h**: NVAPI HLSL extensions documentation
- **nvapi/nvShaderExtnEnums.h**: NVAPI shader extension enums documentation

### Falcor Documentation

- **Falcor Utils**: Utils module documentation
- **Falcor Shaders**: Shaders module documentation

### Related Technologies

- **NVAPI**: NVIDIA API documentation
- **Slang**: Slang shader language documentation
- **Conditional Compilation**: Conditional compilation techniques
- **Vendor Extensions**: Vendor extension techniques
