# ShaderTable

## Overview
[`ShaderTable`](Source/Falcor/Core/API/ShaderTable.h) is a GPU shader table wrapper class for raytracing programs. It provides a mechanism to manage GPU shader tables with deferred resource release using the [`ShaderTablePtr`](Source/Falcor/Core/API/ShaderTable.h:67) wrapper class.

## Source Files
- Header: [`Source/Falcor/Core/API/ShaderTable.h`](Source/Falcor/Core/API/ShaderTable.h) (88 lines)
- Implementation: Header-only (no separate .cpp file)

## Class Hierarchy
```
ShaderTablePtr (wrapper class)
  └── gfx::IShaderTable (via Slang ComPtr)
```

## Dependencies

### Direct Dependencies
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device type
- [`slang-gfx.h`](Source/Falcor/Core/API/slang-gfx.h) - Slang gfx types
- [`Scene.h`](Source/Falcor/Core/Scene/Scene.h) - Scene type (forward declaration)
- [`Program.h`](Source/Falcor/Core/Program/Program.h) - Program type (forward declaration)
- [`RtStateObject.h`](Source/Falcor/Core/API/RtStateObject.h) - RtStateObject type (forward declaration)
- [`RtProgramVars.h`](Source/Falcor/Core/API/RtProgramVars.h) - RtProgramVars type (forward declaration)
- [`RenderContext.h`](Source/Falcor/Core/API/RenderContext.h) - RenderContext type (forward declaration)

### Indirect Dependencies
- Device class (from Device.h)
- Slang gfx interfaces (gfx::IShaderTable)
- ComPtr smart pointer (from Slang)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Validation**: No validation of shader table creation
3. **No Error Handling**: No error handling for shader table access failures
4. **No Custom Root Signatures**: User provided local root signatures are not supported
5. **No Shader Table Layout Validation**: No validation of shader table layout
6. **No Bounds Checking**: No bounds checking for shader table access
7. **No Shader Table Size Control**: No control over shader table size
8. **No Shader Table Update Support**: No support for updating shader table
9. **No Shader Table Query Support**: No support for querying shader table properties
10. **No Shader Table Serialization**: No support for serializing shader table

## Usage Example

```cpp
// Create a shader table
ShaderTablePtr pShaderTable = ShaderTablePtr(pDevice);

// Access gfx shader table
gfx::IShaderTable* pGfxShaderTable = pShaderTable.get();

// Get gfx shader table pointer
gfx::IShaderTable* pGfxShaderTable = pShaderTable->operator->();

// Get gfx shader table reference
gfx::IShaderTable& refGfxShaderTable = pShaderTable->operator*();

// Get write reference for gfx API operations
gfx::IShaderTable** ppGfxShaderTable = pShaderTable->writeRef();

// Get root signature (static function)
RootSignature* pRootSig = ShaderTable::getRootSignature();

// Shader table is automatically released when pShaderTable goes out of scope
```

## Conclusion

ShaderTable provides a straightforward and efficient wrapper for GPU shader tables. The implementation is clean with excellent cache locality but lacks thread safety and advanced features.

**Strengths**:
- Clean and simple API for shader table management
- Excellent cache locality (all members fit in a single cache line)
- Lightweight construction and access
- Automatic resource management using smart pointers
- Cross-platform support (D3D12 and Vulkan)
- Deferred release mechanism via destructor
- Direct access to gfx shader table via multiple operators

**Weaknesses**:
- Not thread-safe for concurrent access
- No validation of shader table creation
- No error handling for shader table access failures
- User provided local root signatures are not supported
- No shader table layout validation
- No bounds checking for shader table access
- No control over shader table size
- No support for updating shader table
- No support for querying shader table properties
- No support for serializing shader table

**Recommendations**:
1. Add thread safety using mutex or atomic operations
2. Add validation for shader table creation
3. Add error handling for shader table access failures
4. Consider adding support for user provided local root signatures
5. Add shader table layout validation
6. Add bounds checking for shader table access
7. Consider adding control over shader table size
8. Consider adding support for updating shader table
9. Consider adding support for querying shader table properties
10. Consider adding support for serializing shader table
11. Consider aligning `mTable` to cache line boundary for better performance
