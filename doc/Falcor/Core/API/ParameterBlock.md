# ParameterBlock

## Overview
[`ParameterBlock`](Source/Falcor/Core/API/ParameterBlock.h) provides a comprehensive shader parameter block management system for binding resources to shader parameters. This module manages uniform variables, buffers, textures, SRVs, UAVs, samplers, parameter blocks, and acceleration structures.

## Source Files
- Header: [`Source/Falcor/Core/API/ParameterBlock.h`](Source/Falcor/Core/API/ParameterBlock.h) (415 lines)
- Implementation: [`Source/Falcor/Core/API/ParameterBlock.cpp`](Source/Falcor/Core/API/ParameterBlock.cpp) (not shown, likely exists)

## Class Hierarchy
```
Object (base class)
  └── ParameterBlock
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Buffer.h`](Source/Falcor/Core/API/Buffer.h) - Buffer type
- [`Texture.h`](Source/Falcor/Core/API/Texture.h) - Texture type
- [`Sampler.h`](Source/Falcor/Core/API/Sampler.h) - Sampler type
- [`RtAccelerationStructure.h`](Source/Falcor/Core/API/RtAccelerationStructure.h) - Acceleration structure type
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_OBJECT, FALCOR_API, FALCOR_UNREACHABLE)
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Program/ProgramReflection.h`](Source/Falcor/Core/Program/ProgramReflection.h) - Program reflection types
- [`Core/Program/ShaderVar.h`](Source/Falcor/Core/Program/ShaderVar.h) - Shader variable types
- [`Utils/UI/Gui.h`](Source/Falcor/Utils/UI/Gui.h) - UI types
- [`slang.h`](Source/Falcor/Core/API/slang.h) - Slang API
- [`memory`](Source/Falcor/Core/API/memory) - Memory utilities (std::memory)
- [`string`](Source/Falcor/Core/API/string) - String type (std::string)
- [`string_view`](Source/Falcor/Core/API/string_view) - String view type
- [`utility`](Source/Falcor/Core/API/utility) - Utility types (std::utility)
- [`variant`](Source/Falcor/Core/API/variant) - Variant type (std::variant)
- [`vector`](Source/Falcor/Core/API/vector) - Vector type (std::vector)
- [`cstddef`](Source/Falcor/Core/API/cstddef) - Standard types (std::size_t)

### Indirect Dependencies
- Buffer class (from Buffer.h)
- Texture class (from Texture.h)
- Sampler class (from Sampler.h)
- Acceleration structure class (from RtAccelerationStructure.h)
- Object base class (from Core/Object.h)
- Program reflection types (from Program/ProgramReflection.h)
- Shader variable types (from Core/Program/ShaderVar.h)
- Slang types (from slang.h)
- Standard C++ types (std::memory, std::string, std::string_view, std::utility, std::variant, std::vector, std::size_t)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **Poor Cache Locality**: Members scattered across multiple cache lines
3. **No Resource Validation**: No validation of resource values (only type validation)
4. **No Bind Flags Validation**: No validation of bind flags values
5. **No Resource Lifecycle Management**: No automatic resource lifecycle management
6. **No Resource Cleanup**: No automatic resource cleanup
7. **No Resource Unbinding**: No automatic resource unbinding
8. **No Resource Rebinding**: No automatic resource rebinding
9. **No Resource Pooling**: No automatic resource pooling
10. **No Resource Caching**: No automatic resource caching

## Usage Example

```cpp
// Create a parameter block from type
ref<Device> pDevice = /* ... */;
ref<const ProgramVersion> pProgramVersion = /* ... */;
ref<const ReflectionType> pType = /* ... */;
ref<ParameterBlock> pParameterBlock = ParameterBlock::create(pDevice, pProgramVersion, pType);

// Create a parameter block from reflection
ref<const ParameterBlockReflection> pReflection = /* ... */;
ref<ParameterBlock> pParameterBlock = ParameterBlock::create(pDevice, pReflection);

// Create a parameter block from type name
std::string typeName = "MyType";
ref<ParameterBlock> pParameterBlock = ParameterBlock::create(pDevice, pProgramVersion, typeName);

// Set uniform variables
pParameterBlock->setVariable("gUniformValue", 42.0f);
pParameterBlock->setVariable(bindLocation, 42.0f);

// Set buffers
ref<Buffer> pBuffer = /* ... */;
pParameterBlock->setBuffer("gBuffer", pBuffer);
pParameterBlock->setBuffer(bindLocation, pBuffer);

// Set textures
ref<Texture> pTexture = /* ... */;
pParameterBlock->setTexture("gTexture", pTexture);
pParameterBlock->setTexture(bindLocation, pTexture);

// Set SRVs
ref<ShaderResourceView> pSrv = /* ... */;
pParameterBlock->setSrv(bindLocation, pSrv);

// Set UAVs
ref<UnorderedAccessView> pUav = /* ... */;
pParameterBlock->setUav(bindLocation, pUav);

// Set samplers
ref<Sampler> pSampler = /* ... */;
pParameterBlock->setSampler("gSampler", pSampler);
pParameterBlock->setSampler(bindLocation, pSampler);

// Set parameter blocks
ref<ParameterBlock> pBlock = /* ... */;
pParameterBlock->setParameterBlock("gBlock", pBlock);
pParameterBlock->setParameterBlock(bindLocation, pBlock);

// Set acceleration structures
ref<RtAccelerationStructure> pAccel = /* ... */;
pParameterBlock->setAccelerationStructure("gAccel", pAccel);
pParameterBlock->setAccelerationStructure(bindLocation, pAccel);

// Get resources
ref<Buffer> pBuffer = pParameterBlock->getBuffer("gBuffer");
ref<Texture> pTexture = pParameterBlock->getTexture("gTexture");
ref<ShaderResourceView> pSrv = pParameterBlock->getSrv(bindLocation);
ref<UnorderedAccessView> pUav = pParameterBlock->getUav(bindLocation);
ref<Sampler> pSampler = pParameterBlock->getSampler("gSampler");
ref<ParameterBlock> pBlock = pParameterBlock->getParameterBlock("gBlock");
ref<ParameterBlock> pBlock = pParameterBlock->getParameterBlock(bindLocation);
ref<RtAccelerationStructure> pAccel = pParameterBlock->getAccelerationStructure(bindLocation);

// Get reflection
ref<const ParameterBlockReflection> pReflection = pParameterBlock->getReflection();
ref<const ReflectionType> pElementType = pParameterBlock->getElementType();
size_t elementSize = pParameterBlock->getElementSize();
TypedShaderVarOffset offset = pParameterBlock->getVariableOffset("gUniformValue");
ShaderVar rootVar = pParameterBlock->getRootVar();
ShaderVar member = pParameterBlock->findMember("gMember");
ShaderVar member = pParameterBlock->findMember(0);
size_t size = pParameterBlock->getSize();

// Prepare descriptor sets
CopyContext* pCopyContext = /* ... */;
bool prepared = pParameterBlock->prepareDescriptorSets(pCopyContext);

// Collect specialization args
ParameterBlock::SpecializationArgs args;
pParameterBlock->collectSpecializationArgs(args);

// Get raw data
const void* pRawData = pParameterBlock->getRawData();
```

## Conclusion

ParameterBlock provides a comprehensive and type-safe shader parameter block management system. The implementation is complex with poor cache locality due to scattered member layout, but provides extensive resource binding capabilities.

**Strengths**:
- Comprehensive resource binding support (buffers, textures, SRVs, UAVs, samplers, parameter blocks, acceleration structures)
- Type-safe resource binding with validation
- Support for both name-based and bind location-based operations
- Exception throwing on errors
- Reflection integration
- Reference counting for resources
- Blob setting support
- Descriptor set preparation
- Specialization argument collection
- Multiple creation methods (type-based, reflection-based, name-based)

**Weaknesses**:
- Not thread-safe for concurrent access
- Poor cache locality (members scattered across multiple cache lines)
- No resource validation (only type validation)
- No bind flags validation
- No automatic resource lifecycle management
- No automatic resource cleanup
- No automatic resource unbinding
- No automatic resource rebinding
- No automatic resource pooling
- No automatic resource caching
- Complex implementation (415 lines)
- Many map lookups (O(log N) complexity)

**Recommendations**:
1. Consider adding thread safety using mutex or atomic operations
2. Consider grouping frequently accessed members to improve cache locality
3. Consider aligning members to cache line boundaries
4. Consider using arrays instead of maps for better cache locality
5. Consider adding resource validation
6. Consider adding bind flags validation
7. Consider adding automatic resource lifecycle management
8. Consider adding automatic resource cleanup
9. Consider adding automatic resource unbinding
10. Consider adding automatic resource rebinding
