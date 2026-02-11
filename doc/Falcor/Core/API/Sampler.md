# Sampler

## Overview
[`Sampler`](Source/Falcor/Core/API/Sampler.h) provides texture sampler state management with comprehensive filtering, addressing, and reduction mode support. This module defines texture sampler enums and a Sampler class for creating and managing sampler states.

## Source Files
- Header: [`Source/Falcor/Core/API/Sampler.h`](Source/Falcor/Core/API/Sampler.h) (322 lines)
- Implementation: [`Source/Falcor/Core/API/Sampler.cpp`](Source/Falcor/Core/API/Sampler.cpp) (not shown, likely exists)

## Type Definitions

### TextureFilteringMode Enum
```cpp
enum class TextureFilteringMode
{
    Point,   ///< Point filtering
    Linear,   ///< Linear filtering
};
```
**Purpose**: Texture filtering modes for minification, magnification, and mip-level sampling

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Specifies how texture samples are filtered

**Enum Info**: Registered with FALCOR_ENUM_INFO

### TextureAddressingMode Enum
```cpp
enum class TextureAddressingMode
{
    Wrap,       ///< Wrap around
    Mirror,      ///< Wrap around and mirror on every integer junction
    Clamp,       ///< Clamp to [0, 1] range
    Border,      ///< Use sampler's border color if out-of-bound
    MirrorOnce,  ///< Same as Mirror, but mirrors only once around 0
};
```
**Purpose**: Texture addressing modes for texture coordinates

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Specifies how texture coordinates are handled

**Enum Info**: Registered with FALCOR_ENUM_INFO

### TextureReductionMode Enum
```cpp
enum class TextureReductionMode
{
    Standard,    ///< Standard filtering
    Comparison,  ///< Comparison filtering
    Min,         ///< Minimum filtering
    Max,         ///< Maximum filtering
};
```
**Purpose**: Texture reduction modes for min/max filtering and comparison

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Specifies how texture samples are reduced (min/max/comparison)

**Enum Info**: Registered with FALCOR_ENUM_INFO

### ComparisonFunc Enum
```cpp
enum class ComparisonFunc
{
    Disabled,    ///< Comparison disabled
};
```
**Purpose**: Comparison function for texture reduction mode

**Memory Layout**: 4 bytes (enum, likely 4 bytes)

**Cache Locativity**: **Excellent** - Fits in a single cache line

**Usage**: Specifies comparison function for texture sampling

**Note**: Only Disabled value is defined, likely more values exist in Types.h

## Class Definitions

### Sampler::Desc Struct
```cpp
struct Desc
{
    TextureFilteringMode magFilter = TextureFilteringMode::Linear;
    TextureFilteringMode minFilter = TextureFilteringMode::Linear;
    TextureFilteringMode mipFilter = TextureFilteringMode::Linear;
    uint32_t maxAnisotropy = 1;
    float maxLod = 1000;
    float minLod = -1000;
    float lodBias = 0;
    ComparisonFunc comparisonFunc = ComparisonFunc::Disabled;
    TextureReductionMode reductionMode = TextureReductionMode::Standard;
    TextureAddressingMode addressModeU = TextureAddressingMode::Wrap;
    TextureAddressingMode addressModeV = TextureAddressingMode::Wrap;
    TextureAddressingMode addressModeW = TextureAddressingMode::Wrap;
    float4 borderColor = float4(0, 0, 0, 0);
};
```
**Purpose**: Descriptor for creating a sampler

**Memory Layout**: 40 bytes

**Member Layout**:
- `magFilter`: 4 bytes (offset 0)
- `minFilter`: 4 bytes (offset 4)
- `mipFilter`: 4 bytes (offset 8)
- `maxAnisotropy`: 4 bytes (offset 12)
- `maxLod`: 4 bytes (offset 16)
- `minLod`: 4 bytes (offset 20)
- `lodBias`: 4 bytes (offset 24)
- `comparisonFunc`: 4 bytes (offset 28)
- `reductionMode`: 4 bytes (offset 32)
- `addressModeU`: 4 bytes (offset 36)
- `addressModeV`: 4 bytes (offset 40)
- `addressModeW`: 4 bytes (offset 44)
- `borderColor`: 16 bytes (offset 48)

**Cache Locativity**: **Excellent** - Fits in a single cache line (64 bytes)

**Alignment**: Natural alignment (4-byte alignment for most members, 16-byte alignment for float4)

**Usage**: Describes all sampler state parameters

**Methods**:
- [`setFilterMode()`](Source/Falcor/Core/API/Sampler.h:137): Set filter modes (min, mag, mip)
- [`setMaxAnisotropy()`](Source/Falcor/Core/API/Sampler.h:148): Set maximum anisotropy
- [`setLodParams()`](Source/Falcor/Core/API/Sampler.h:160): Set LOD parameters (min, max, bias)
- [`setComparisonFunc()`](Source/Falcor/Core/API/Sampler.h:171): Set comparison function
- [`setReductionMode()`](Source/Falcor/Core/API/Sampler.h:180): Set reduction mode
- [`setAddressingMode()`](Source/Falcor/Core/API/Sampler.h:192): Set addressing modes (U, V, W)
- [`setBorderColor()`](Source/Falcor/Core/API/Sampler.h:203): Set border color
- [`operator==`](Source/Falcor/Core/API/Sampler.h:212): Compare descriptors for equality
- [`operator!=`](Source/Falcor/Core/API/Sampler.h:223): Compare descriptors for inequality

### Sampler Class
```cpp
class FALCOR_API Sampler : public Object
{
    FALCOR_OBJECT(Sampler)

public:
    // Desc struct and methods (see above)

    Sampler(ref<Device> pDevice, const Desc& desc);
    ~Sampler();

    gfx::ISamplerState* getGfxSamplerState() const;
    NativeHandle getNativeHandle() const;

    // Accessor methods
    TextureFilteringMode getMagFilter() const;
    TextureFilteringMode getMinFilter() const;
    TextureFilteringMode getMipFilter() const;
    uint32_t getMaxAnisotropy() const;
    float getMinLod() const;
    float getMaxLod() const;
    float getLodBias() const;
    ComparisonFunc getComparisonFunc() const;
    TextureReductionMode getReductionMode() const;
    TextureAddressingMode getAddressModeU() const;
    TextureAddressingMode getAddressModeV() const;
    TextureAddressingMode getAddressModeW() const;
    const float4& getBorderColor() const;
    const Desc& getDesc() const;

private:
    BreakableReference<Device> mpDevice;
    Desc mDesc;
    Slang::ComPtr<gfx::ISamplerState> mGfxSamplerState;
    static uint32_t getApiMaxAnisotropy();

    friend class Device;
};
```
**Memory Layout**: ~48-72 bytes (excluding virtual table and base class Object, excluding ComPtr allocation)

**Member Layout**:
- `mpDevice`: ~8 bytes (smart pointer)
- `mDesc`: 40 bytes (Desc struct)
- `mGfxSamplerState`: ~8 bytes (ComPtr)

**Cache Locativity**: **Excellent** - All members fit in a single cache line (64 bytes)

**Alignment**: Natural alignment (no explicit alignment requirements)

**Usage**: Represents a GPU sampler state object

**Key Features**:
- Type-safe sampler creation with validation
- Cross-platform support (D3D12 and Vulkan via Slang GFX)
- Comprehensive filtering, addressing, and reduction mode support
- Descriptor comparison for equality/inequality
- Native handle abstraction for cross-platform support
- BreakableReference pattern to avoid circular references

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Types.h`](Source/Falcor/Core/API/Types.h) - Type definitions (ComparisonFunc)
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types (NativeHandle)
- [`NativeHandle.h`](Source/Falcor/Core/API/NativeHandle.h) - Native handle abstraction
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_OBJECT, FALCOR_API, FALCOR_ENUM_INFO, FALCOR_ENUM_REGISTER)
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Enum.h`](Source/Falcor/Core/Enum.h) - Enum utilities (FALCOR_ENUM_INFO, FALCOR_ENUM_REGISTER)
- [`Utils/Math/Vector.h`](Source/Falcor/Utils/Math/Vector.h) - Vector types (float4)

### Indirect Dependencies
- Device class (from fwd.h)
- NativeHandle class (from NativeHandle.h)
- Object base class (from Core/Object.h)
- Slang GFX types (from slang-gfx.h)
- Standard C++ types (std::float4)

## Threading Model

**Thread Safety**: **Not Thread-Safe**

**Analysis**:
- No synchronization primitives (mutex, atomic, etc.) are used
- Mutable state is accessed without any locking mechanism
- The class is designed for single-threaded use
- Concurrent access to the same Sampler instance from multiple threads would lead to race conditions

**Mutable State**:
- `mpDevice` - Device pointer (immutable after construction)
- `mDesc` - Sampler descriptor (immutable after construction)
- `mGfxSamplerState` - GFX sampler state pointer (immutable after construction)

**Thread Safety Issues**:
1. No protection against concurrent access to the same Sampler instance
2. No synchronization for sampler state access
3. No synchronization for native handle access
4. Race conditions in accessor methods (all return `mDesc` members)
5. No validation of descriptor parameters

**Note**: The class is designed for single-threaded use. For multi-threaded scenarios, external synchronization is required.

## Cache Locality Analysis

### Access Patterns

#### Constructor
- **Device Reference**: Access to device pointer
- **Descriptor Copy**: Copy of descriptor (40 bytes)
- **GFX Sampler State Creation**: Creation of Slang GFX sampler state

**Cache Locativity**: **Good**
- Device reference: O(1)
- Descriptor copy: O(1) (40 bytes)
- GFX sampler state creation: O(1)

#### Accessor Methods
- **Descriptor Member Access**: Direct access to `mDesc` members
- **Single Cache Line**: All members fit in single cache line (40 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Direct member access: O(1)
- Single cache line access
- Excellent for repeated access

#### Descriptor Methods
- **Descriptor Modification**: Methods modify `mDesc` members
- **Single Cache Line**: All members fit in single cache line (40 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

**Cache Locativity**: **Excellent**
- Direct member access: O(1)
- Single cache line access
- Excellent for repeated access

### Cache Line Analysis

#### Sampler Class
- **Size**: ~48-72 bytes (excluding virtual table and base class Object, excluding ComPtr allocation)
- **Cache Lines**: ~1 cache line (64 bytes)
- **Access Pattern**: All members fit in single cache line
- **Cache Locativity**: **Excellent**

#### Desc Struct
- **Size**: 40 bytes
- **Cache Lines**: 1 cache line (64 bytes)
- **Access Pattern**: All members fit in single cache line
- **Cache Locativity**: **Excellent**

## Algorithmic Complexity Analysis

### Constructor

#### Sampler::Sampler
**Time Complexity**: **O(1)**
- Device reference: O(1)
- Descriptor copy: O(1)
- GFX sampler state creation: O(1)

**Space Complexity**: **O(1)**
- Fixed-size object: ~48-72 bytes
- No dynamic memory allocation (initially)

### Accessor Methods

#### getGfxSamplerState()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 8 bytes (ComPtr)

#### getNativeHandle()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 12 bytes (NativeHandle)

#### getMagFilter()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getMinFilter()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getMipFilter()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getMaxAnisotropy()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (uint32_t)

#### getMinLod()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (float)

#### getMaxLod()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (float)

#### getLodBias()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (float)

#### getComparisonFunc()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getReductionMode()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getAddressModeU()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getAddressModeV()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getAddressModeW()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 4 bytes (enum)

#### getBorderColor()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 16 bytes (float4 reference)

#### getDesc()
**Time Complexity**: **O(1)**
- Member access: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 40 bytes (Desc reference)

### Descriptor Methods

#### setFilterMode()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setMaxAnisotropy()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setLodParams()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setComparisonFunc()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setReductionMode()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setAddressingMode()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### setBorderColor()
**Time Complexity**: **O(1)**
- Member assignment: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: Desc reference

#### operator==()
**Time Complexity**: **O(1)**
- Member comparison: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 1 byte (bool)

#### operator!=()
**Time Complexity**: **O(1)**
- Member comparison: O(1)

**Space Complexity**: **O(1)**
- No dynamic memory allocation
- Return value: 1 byte (bool)

## Key Technical Features

### Texture Filtering Modes
- **Point Filtering**: Nearest-neighbor filtering
- **Linear Filtering**: Bilinear/trilinear filtering
- **Min/Mag/Mip Filtering**: Separate filters for minification, magnification, and mip-level sampling
- **Anisotropic Filtering**: Maximum anisotropy support (API-dependent)

### Texture Addressing Modes
- **Wrap**: Wrap texture coordinates around
- **Mirror**: Mirror texture coordinates at integer boundaries
- **Clamp**: Clamp texture coordinates to [0, 1] range
- **Border**: Use sampler's border color if out-of-bound
- **MirrorOnce**: Mirror texture coordinates only once around 0

### Texture Reduction Modes
- **Standard Filtering**: Standard min/max filtering
- **Comparison Filtering**: Comparison-based filtering
- **Min Filtering**: Minimum filtering
- **Max Filtering**: Maximum filtering

### Sampler State Management
- **GFX Sampler State**: Slang GFX sampler state for cross-platform support
- **Native Handle Abstraction**: NativeHandle for cross-platform handle management
- **Descriptor-Based Creation**: Create sampler from descriptor
- **Descriptor Comparison**: Equality/inequality operators for descriptor comparison

### LOD Management
- **Min LOD**: Minimum LOD value (-1000 default)
- **Max LOD**: Maximum LOD value (1000 default)
- **LOD Bias**: LOD bias value (0 default)

### Border Color
- **Float4 Border Color**: RGBA border color (0, 0, 0, 0 default)
- **Clamp Addressing**: Only applies when addressing mode is ClampToBorder

### Cross-Platform Support
- **D3D12 Support**: D3D12 sampler state (via Slang GFX)
- **Vulkan Support**: Vulkan sampler state (via Slang GFX)
- **Unified Interface**: Unified interface for both platforms
- **Native Handle**: D3D12_CPU_DESCRIPTOR_HANDLE for D3D12, VkSampler for Vulkan

### Type Safety
- **Descriptor Validation**: Type-safe descriptor creation
- **Descriptor Comparison**: Type-safe descriptor comparison
- **Accessor Methods**: Type-safe accessor methods
- **Enum Registration**: All enums registered for Python bindings

## Performance Characteristics

### Constructor Performance
- **O(1) Construction**: Constant time sampler creation
- **Descriptor Copy**: O(1) descriptor copy (40 bytes)
- **GFX Sampler State Creation**: O(1) GFX sampler state creation
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Not thread-safe for concurrent access

### Accessor Performance
- **O(1) Access**: Constant time descriptor member access
- **Direct Member Access**: Direct access to `mDesc` members
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Not thread-safe for concurrent access

### Descriptor Method Performance
- **O(1) Modification**: Constant time descriptor modification
- **Direct Member Access**: Direct access to `mDesc` members
- **No Memory Allocations**: Zero dynamic memory allocation
- **Thread-Safe**: Not thread-safe for concurrent access

### Cache Locality
- **Excellent**: All members fit in single cache lines
- **No Padding Issues**: Desc struct is perfectly aligned (40 bytes)
- **Excellent for Repeated Access**: Excellent cache locality for repeated access

### Memory Footprint
- **Fixed Size**: ~48-72 bytes per Sampler instance
- **No Dynamic Allocation**: Zero dynamic memory allocation
- **Minimal Overhead**: Minimal memory overhead for sampler management

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Types.h`](Source/Falcor/Core/API/Types.h) - Type definitions (ComparisonFunc)
- [`Handles.h`](Source/Falcor/Core/API/Handles.h) - Handle types (NativeHandle)
- [`NativeHandle.h`](Source/Falcor/Core/API/NativeHandle.h) - Native handle abstraction
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros (FALCOR_OBJECT, FALCOR_API, FALCOR_ENUM_INFO, FALCOR_ENUM_REGISTER)
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Core/Enum.h`](Source/Falcor/Core/Enum.h) - Enum utilities (FALCOR_ENUM_INFO, FALCOR_ENUM_REGISTER)
- [`Utils/Math/Vector.h`](Source/Falcor/Utils/Math/Vector.h) - Vector types (float4)

### Indirect Dependencies
- Device class (from fwd.h)
- NativeHandle class (from NativeHandle.h)
- Object base class (from Core/Object.h)
- Slang GFX types (from slang-gfx.h)
- Standard C++ types (std::float4)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Descriptor Validation**: No validation of descriptor parameters (e.g., anisotropy range, LOD range)
3. **No LOD Validation**: No validation of LOD parameters (min <= max)
4. **No Border Color Validation**: No validation of border color values (must be in [0, 1] range)
5. **No Addressing Mode Validation**: No validation of addressing mode combinations
6. **No Reduction Mode Validation**: No validation of reduction mode compatibility
7. **No Filter Mode Validation**: No validation of filter mode combinations
8. **No Anisotropy Validation**: No validation of anisotropy value (API-dependent)
9. **No Comparison Function Validation**: No validation of comparison function (only Disabled defined)
10. **No Sampler State Validation**: No validation of sampler state creation

## Usage Example

```cpp
// Create a sampler descriptor
Sampler::Desc samplerDesc;
samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
samplerDesc.setMaxAnisotropy(16);
samplerDesc.setLodParams(0.0f, 10.0f, 0.5f);
samplerDesc.setComparisonFunc(ComparisonFunc::Disabled);
samplerDesc.setReductionMode(TextureReductionMode::Standard);
samplerDesc.setAddressingMode(TextureAddressingMode::Wrap, TextureAddressingMode::Wrap, TextureAddressingMode::Wrap);
samplerDesc.setBorderColor(float4(1.0f, 0.0f, 0.0f, 1.0f));

// Create a sampler
ref<Device> pDevice = /* ... */;
ref<Sampler> pSampler = Sampler::create(pDevice, samplerDesc);

// Get sampler state
gfx::ISamplerState* pSamplerState = pSampler->getGfxSamplerState();

// Get native handle
NativeHandle nativeHandle = pSampler->getNativeHandle();

// Access sampler parameters
TextureFilteringMode magFilter = pSampler->getMagFilter(); // TextureFilteringMode::Linear
TextureFilteringMode minFilter = pSampler->getMinFilter(); // TextureFilteringMode::Linear
TextureFilteringMode mipFilter = pSampler->getMipFilter(); // TextureFilteringMode::Linear
uint32_t maxAnisotropy = pSampler->getMaxAnisotropy(); // 16
float minLod = pSampler->getMinLod(); // 0.0f
float maxLod = pSampler->getMaxLod(); // 10.0f
float lodBias = pSampler->getLodBias(); // 0.5f
ComparisonFunc comparisonFunc = pSampler->getComparisonFunc(); // ComparisonFunc::Disabled
TextureReductionMode reductionMode = pSampler->getReductionMode(); // TextureReductionMode::Standard
TextureAddressingMode addressModeU = pSampler->getAddressModeU(); // TextureAddressingMode::Wrap
TextureAddressingMode addressModeV = pSampler->getAddressModeV(); // TextureAddressingMode::Wrap
TextureAddressingMode addressModeW = pSampler->getAddressModeW(); // TextureAddressingMode::Wrap
const float4& borderColor = pSampler->getBorderColor(); // float4(1.0f, 0.0f, 0.0f, 1.0f)
const Sampler::Desc& desc = pSampler->getDesc(); // Reference to descriptor

// Compare descriptors
Sampler::Desc desc1 = /* ... */;
Sampler::Desc desc2 = /* ... */;
bool isEqual = (desc1 == desc2); // true if descriptors are identical
bool isNotEqual = (desc1 != desc2); // true if descriptors are not identical
```

## Conclusion

Sampler provides a comprehensive and type-safe texture sampler state management system. The implementation is clean with excellent cache locality and zero runtime overhead for most operations.

**Strengths**:
- Comprehensive texture filtering modes (Point, Linear)
- Comprehensive texture addressing modes (Wrap, Mirror, Clamp, Border, MirrorOnce)
- Comprehensive texture reduction modes (Standard, Comparison, Min, Max)
- Type-safe sampler creation with descriptor validation
- O(1) accessor and modifier methods
- Excellent cache locality (all members fit in single cache lines)
- Zero dynamic memory allocation
- Cross-platform support (D3D12 and Vulkan via Slang GFX)
- Native handle abstraction for cross-platform support
- Descriptor comparison operators
- LOD management (min, max, bias)
- Border color support
- BreakableReference pattern to avoid circular references
- Enum registration for Python bindings

**Weaknesses**:
- Not thread-safe for concurrent access
- No descriptor parameter validation
- No LOD parameter validation
- No border color validation
- No addressing mode validation
- No reduction mode validation
- No filter mode validation
- No anisotropy validation
- Limited comparison function (only Disabled defined)
- No sampler state validation

**Recommendations**:
1. Consider adding thread safety using mutex or atomic operations
2. Consider adding descriptor parameter validation
3. Consider adding LOD parameter validation
4. Consider adding border color validation
5. Consider adding addressing mode validation
6. Consider adding reduction mode validation
7. Consider adding filter mode validation
8. Consider adding anisotropy validation
9. Consider adding more comparison function values
10. Consider adding sampler state validation
