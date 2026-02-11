# D3D12DescriptorSetLayout

## Module Overview

The **D3D12DescriptorSetLayout** class provides D3D12 descriptor set layout management with support for multiple descriptor ranges and shader visibility. It defines the structure of descriptor sets, including descriptor types, register indices, descriptor counts, and register spaces. The layout uses a builder pattern for fluent interface construction.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12DescriptorSetLayout.h` (87 lines)
- **Implementation**: Header-only (no separate .cpp file)

## Enum Definition

### ShaderVisibility

```cpp
enum class ShaderVisibility
{
    None = 0,
    Vertex = (1 << (uint32_t)ShaderType::Vertex),
    Pixel = (1 << (uint32_t)ShaderType::Pixel),
    Hull = (1 << (uint32_t)ShaderType::Hull),
    Domain = (1 << (uint32_t)ShaderType::Domain),
    Geometry = (1 << (uint32_t)ShaderType::Geometry),
    Compute = (1 << (uint32_t)ShaderType::Compute),
    All = (1 << (uint32_t)ShaderType::Count) - 1,
};
```

## Class Definition

### D3D12DescriptorSetLayout

```cpp
class D3D12DescriptorSetLayout
{
public:
    struct Range
    {
        ShaderResourceType type;
        uint32_t baseRegIndex;
        uint32_t descCount;
        uint32_t regSpace;
    };

    D3D12DescriptorSetLayout(ShaderVisibility visibility = ShaderVisibility::All) : mVisibility(visibility) {}
    D3D12DescriptorSetLayout& addRange(ShaderResourceType type, uint32_t baseRegIndex, uint32_t descriptorCount, uint32_t regSpace = 0)
    {
        Range r;
        r.descCount = descriptorCount;
        r.baseRegIndex = baseRegIndex;
        r.regSpace = regSpace;
        r.type = type;

        mRanges.push_back(r);
        return *this;
    }
    size_t getRangeCount() const { return mRanges.size(); }
    const Range& getRange(size_t index) const { return mRanges[index]; }
    ShaderVisibility getVisibility() const { return mVisibility; }

private:
    std::vector<Range> mRanges;
    ShaderVisibility mVisibility;
};
```

## Dependencies

### Internal Dependencies

- **ShaderResourceType**: Required for descriptor type specification
- **ShaderType**: Required for shader visibility mapping

### External Dependencies

- **std::vector**: Standard library vector container
- **std::uint32_t**: Standard library 32-bit unsigned integer

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different descriptor layout management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### Layout Construction

```cpp
D3D12DescriptorSetLayout layout;
layout.addRange(D3D12DescriptorSetLayout::Type::TextureSrv, 0, 16)
      .addRange(D3D12DescriptorSetLayout::Type::Sampler, 0, 8)
      .addRange(D3D12DescriptorSetLayout::Type::Cbv, 0, 4);
```

### Layout Construction with Custom Visibility

```cpp
D3D12DescriptorSetLayout layout(ShaderVisibility::Vertex | ShaderVisibility::Pixel);
layout.addRange(D3D12DescriptorSetLayout::Type::TextureSrv, 0, 16);
```

### Layout Construction with Register Space

```cpp
D3D12DescriptorSetLayout layout;
layout.addRange(D3D12DescriptorSetLayout::Type::TextureSrv, 0, 16, 1);
```

### Accessing Layout Information

```cpp
size_t rangeCount = layout.getRangeCount();
const D3D12DescriptorSetLayout::Range& range = layout.getRange(0);
ShaderVisibility visibility = layout.getVisibility();
```

## Summary

**D3D12DescriptorSetLayout** is a lightweight D3D12 descriptor set layout manager that provides:

### D3D12DescriptorSetLayout
- Excellent cache locality (all members in single cache line, excluding vector elements)
- O(1) time complexity for access operations
- O(1) amortized time complexity for range addition
- Builder pattern with fluent interface
- Supports multiple descriptor ranges
- Supports shader visibility flags
- Value type (no reference counting)
- POD Range struct for efficient copying

### D3D12DescriptorSetLayout::Range
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- POD type (trivially copyable)
- 16 bytes total size
- No padding required

### ShaderVisibility
- Bit flags for shader visibility
- Maps to ShaderType enum
- Supports combinations
- All value (0x3F) for all shaders
- Enum operators support

The class implements a simple, efficient descriptor set layout system with builder pattern, shader visibility support, and multiple descriptor ranges, providing a flexible and efficient layout definition for D3D12 applications.
