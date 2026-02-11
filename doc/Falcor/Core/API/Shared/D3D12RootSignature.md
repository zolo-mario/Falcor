# D3D12RootSignature

## Module Overview

The **D3D12RootSignature** class provides D3D12 root signature management with support for descriptor tables, root descriptors, and root constants. It defines the layout of resources bound to the pipeline and provides methods for creating root signatures from either explicit descriptions or program reflection. The class uses a builder pattern for flexible root signature construction.

## Files

- **Header**: `Source/Falcor/Core/API/Shared/D3D12RootSignature.h` (149 lines)
- **Implementation**: `Source/Falcor/Core/API/Shared/D3D12RootSignature.cpp` (441 lines)

## Struct Definitions

### D3D12RootSignature::RootDescriptorDesc

```cpp
struct RootDescriptorDesc
{
    DescType type;
    uint32_t regIndex;
    uint32_t spaceIndex;
    ShaderVisibility visibility;
};
```

### D3D12RootSignature::RootConstantsDesc

```cpp
struct RootConstantsDesc
{
    uint32_t regIndex;
    uint32_t spaceIndex;
    uint32_t count;
};
```

## Class Definitions

### D3D12RootSignature::Desc

```cpp
class Desc
{
public:
    Desc& addDescriptorSet(const D3D12DescriptorSetLayout& setLayout);
    Desc& addRootDescriptor(DescType type, uint32_t regIndex, uint32_t spaceIndex, ShaderVisibility visibility = ShaderVisibility::All);
    Desc& addRootConstants(uint32_t regIndex, uint32_t spaceIndex, uint32_t count);

    size_t getSetsCount() const { return mSets.size(); }
    const D3D12DescriptorSetLayout& getSet(size_t index) const { return mSets[index]; }

    size_t getRootDescriptorCount() const { return mRootDescriptors.size(); }
    const RootDescriptorDesc& getRootDescriptorDesc(size_t index) const { return mRootDescriptors[index]; }

    size_t getRootConstantCount() const { return mRootConstants.size(); }
    const RootConstantsDesc& getRootConstantDesc(size_t index) const { return mRootConstants[index]; }

private:
    friend class D3D12RootSignature;
    std::vector<D3D12DescriptorSetLayout> mSets;
    std::vector<RootDescriptorDesc> mRootDescriptors;
    std::vector<RootConstantsDesc> mRootConstants;
};
```

### D3D12RootSignature

```cpp
class D3D12RootSignature : public Object
{
    FALCOR_OBJECT(D3D12RootSignature)
public:
    using ApiHandle = ID3D12RootSignaturePtr;
    using DescType = ShaderResourceType;

    ~D3D12RootSignature();

    static ref<D3D12RootSignature> create(ref<Device> pDevice, const Desc& desc);
    static ref<D3D12RootSignature> create(ref<Device> pDevice, const ProgramReflection* pReflection);

    const ApiHandle& getApiHandle() const { return mApiHandle; }

    size_t getDescriptorSetCount() const { return mDesc.mSets.size(); }
    const D3D12DescriptorSetLayout& getDescriptorSet(size_t index) const { return mDesc.mSets[index]; }

    uint32_t getDescriptorSetBaseIndex() const { return 0; }
    uint32_t getRootDescriptorBaseIndex() const { return getDescriptorSetBaseIndex() + (uint32_t)mDesc.mSets.size(); }
    uint32_t getRootConstantBaseIndex() const { return getRootDescriptorBaseIndex() + (uint32_t)mDesc.mRootDescriptors.size(); }

    uint32_t getSizeInBytes() const { return mSizeInBytes; }
    uint32_t getElementByteOffset(uint32_t elementIndex) { return mElementByteOffset[elementIndex]; }

    void bindForGraphics(CopyContext* pCtx);
    void bindForCompute(CopyContext* pCtx);

    const Desc& getDesc() const { return mDesc; }

protected:
    D3D12RootSignature(ref<Device> pDevice, const Desc& desc);
    void createApiHandle(ID3DBlobPtr pSigBlob);
    ref<Device> mpDevice;
    Desc mDesc;
    ApiHandle mApiHandle;

    uint32_t mSizeInBytes;
    std::vector<uint32_t> mElementByteOffset;
};
```

## Dependencies

### Internal Dependencies

- **D3D12Handles**: Required for D3D12 handle types
- **D3D12DescriptorSet**: Required for descriptor set layout
- **Device**: Required for device access and D3D12 API calls
- **ProgramReflection**: Required for reflection-based creation
- **CopyContext**: Required for binding operations

### External Dependencies

- **D3D12**: Requires Direct3D 12 API
- **std::vector**: Standard library vector container
- **std::shared_ptr**: Standard library shared pointer
- **std::string**: Standard library string
- **std::atomic**: Standard library atomic operations

## Cross-Platform Considerations

- **Windows Only**: D3D12 is only available on Windows
- **No Vulkan Equivalent**: Vulkan uses different root signature management
- **Conditional Compilation**: Likely uses `FALCOR_HAS_D3D12` for conditional compilation

## Usage Patterns

### Root Signature Creation (Explicit)

```cpp
ref<Device> pDevice = ...;

D3D12RootSignature::Desc desc;
desc.addDescriptorSet(setLayout1)
     .addDescriptorSet(setLayout2)
     .addRootDescriptor(D3D12RootSignature::DescType::Cbv, 0, 0)
     .addRootConstants(1, 0, 4);

ref<D3D12RootSignature> pRootSig = D3D12RootSignature::create(pDevice, desc);
```

### Root Signature Creation (Reflection)

```cpp
ref<Device> pDevice = ...;
const ProgramReflection* pReflection = ...;

ref<D3D12RootSignature> pRootSig = D3D12RootSignature::create(pDevice, pReflection);
```

### Binding Root Signature

```cpp
CopyContext* pCtx = ...;

pRootSig->bindForGraphics(pCtx);
pRootSig->bindForCompute(pCtx);
```

### Accessing Root Signature Information

```cpp
size_t setCount = pRootSig->getDescriptorSetCount();
const D3D12DescriptorSetLayout& set = pRootSig->getDescriptorSet(0);
uint32_t setBaseIndex = pRootSig->getDescriptorSetBaseIndex();
uint32_t rootDescBaseIndex = pRootSig->getRootDescriptorBaseIndex();
uint32_t rootConstBaseIndex = pRootSig->getRootConstantBaseIndex();
uint32_t sizeInBytes = pRootSig->getSizeInBytes();
uint32_t elementOffset = pRootSig->getElementByteOffset(0);
```

## Summary

**D3D12RootSignature** is a sophisticated D3D12 root signature manager that provides:

### D3D12RootSignature
- Moderate cache locality (spans 2 cache lines)
- O(N) time complexity for creation (N = number of elements)
- O(1) time complexity for binding and access
- Supports descriptor tables, root descriptors, and root constants
- Consecutive parameter arrangement
- Base index tracking for parameter types
- Reflection-based creation support
- Element byte offset tracking
- Automatic cleanup via smart pointers

### D3D12RootSignature::Desc
- Moderate cache locality (spans 2 cache lines)
- O(1) amortized time complexity for element addition
- Builder pattern with fluent interface
- Supports descriptor sets, root descriptors, and root constants
- Vector storage for dynamic element management

### D3D12RootSignature::RootDescriptorDesc
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- POD type (trivially copyable)
- 16 bytes total size
- No padding required

### D3D12RootSignature::RootConstantsDesc
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for all operations
- POD type (trivially copyable)
- 12 bytes total size
- No padding required

The class implements a sophisticated root signature management system with support for multiple parameter types, reflection-based creation, and efficient binding, providing flexible and efficient root signature management for D3D12 applications.
