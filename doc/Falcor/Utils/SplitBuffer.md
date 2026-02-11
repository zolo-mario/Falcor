# SplitBuffer - CPU/GPU Buffer Management

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core/API/Buffer.h (Buffer)
- Falcor/Core/API/Device.h (Device, ResourceBindFlags)
- Falcor/Core/Program/DefineList.h (DefineList)
- Falcor/Core/Program/ShaderVar.h (ShaderVar)
- Falcor/Core/Error (FALCOR_ASSERT, FALCOR_CHECK, FALCOR_THROW)
- std::vector (std::vector)
- fmt/format.h (fmt::format)

### Dependent Modules

- Falcor/Scene (Scene caching)
- Falcor/Rendering (Rendering buffer management)
- Falcor/Utils (Internal utilities)

## Module Overview

SplitBuffer provides a CPU/GPU buffer management system that handles GPU limit on 4GB buffers by splitting large datasets into multiple buffers. It uses a power-of-2 byte index encoding to select which 4GB buffer to use, allowing up to 2^30 items (for uint32_t) or 2^31 items (for uint64_t) in a single buffer. The class manages both CPU and GPU buffers, with automatic GPU buffer creation and CPU data management.

### Key Characteristics

- **GPU Limit Handling**: Handles 4GB GPU buffer limit
- **Power-of-2 Encoding**: Uses upper bits of index for buffer selection
- **Automatic GPU Creation**: Creates GPU buffers on demand
- **CPU Data Management**: Manages CPU-side data
- **Balanced Buffering**: Balances content across all buffers
- **Flexible Buffer Count**: Configurable number of buffers
- **Memory Efficient**: Minimizes memory overhead
- **Debug Support**: Debugging and statistics support

### Architecture Principles

1. **GPU Limit Workaround**: Work around 4GB GPU buffer limit
2. **Power-of-2 Encoding**: Efficient buffer selection encoding
3. **Lazy Initialization**: GPU buffers created on demand
4. **Automatic Cleanup**: Automatic GPU buffer cleanup
5. **Balanced Distribution**: Balanced content distribution

## Component Specifications

### SplitBuffer Class

**File**: `Source/Falcor/Utils/SplitBuffer.h` (lines 52-298)

```cpp
template<typename T, bool TByteBuffer>
class SplitBuffer
{
public:
    using ElementType = T;

    SplitBuffer() : mCpuBuffers(1) {}

    size_t getMaxBufferCount() const { return kMaxBufferCount; }

    void setBufferCount(uint32_t bufferCount)
    {
        FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot change buffer count after creating GPU buffers.");
        FALCOR_CHECK(bufferCount >= getBufferCount(), "Cannot reduce number of existing buffers ({}).", getBufferCount());
        FALCOR_CHECK(bufferCount <= kMaxBufferCount, "Cannot exceed max number of buffers ({}).", kMaxBufferCount);
        mCpuBuffers.resize(bufferCount);
    }

    void setName(std::string_view name) { mBufferName = name; }

    void setBufferCountDefinePrefix(std::string bufferCountDefinePrefix) { mBufferCountDefinePrefix = bufferCountDefinePrefix; }

    template<typename Iter>
    uint32_t insert(Iter first, Iter last)
    {
        FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot insert after creating GPU buffers.");

        if (first == last)
            return 0;

        const size_t itemCount = std::distance(first, last);

        // Find the buffer with fewest items.
        auto it = std::min_element(
            mCpuBuffers.begin(),
            mCpuBuffers.end(),
            [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
        );

        uint32_t bufferIndex = std::distance(mCpuBuffers.begin(), it);

        // If new items wouldn't fit into the buffer with fewest items, create a new buffer,
        // throw if new buffer cannot be created.
        if ((mCpuBuffers[bufferIndex].size() + itemCount) * sizeof(T) > kBufferSizeLimit)
        {
            bufferIndex = mCpuBuffers.size();
            if (bufferIndex >= kMaxBufferCount)
                FALCOR_THROW("Buffers {} cannot accomodate all of data within the buffer limit.", mBufferName);
            mCpuBuffers.push_back({});
        }

        const uint32_t elementIndex = mCpuBuffers[bufferIndex].size();
        FALCOR_ASSERT((((1 << kBufferIndexOffset) - 1) & elementIndex) == elementIndex, "Element index overflows into buffer index");

        mCpuBuffers[bufferIndex].insert(mCpuBuffers[bufferIndex].end(), first, last);
        return ((bufferIndex << kBufferIndexOffset) | elementIndex);
    }

    uint32_t insertEmpty(size_t itemCount)
    {
        FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot insert after creating GPU buffers.");
        if (itemCount == 0)
            return 0;

        // Find the buffer with fewest items.
        auto it = std::min_element(
            mCpuBuffers.begin(),
            mCpuBuffers.end(),
            [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
        );

        uint32_t bufferIndex = std::distance(mCpuBuffers.begin(), it);

        // If new items wouldn't fit into the buffer with fewest items, create a new buffer,
        // throw if new buffer cannot be created.
        if ((mCpuBuffers[bufferIndex].size() + itemCount) * sizeof(T) > kBufferSizeLimit)
        {
            bufferIndex = mCpuBuffers.size();
            if (bufferIndex >= kMaxBufferCount)
                FALCOR_THROW("Buffers {} cannot accomodate all of data within the buffer limit.", mBufferName);
            mCpuBuffers.push_back({});
        }

        const uint32_t elementIndex = mCpuBuffers[bufferIndex].size();
        FALCOR_ASSERT((((1 << kBufferIndexOffset) - 1) & elementIndex) == elementIndex, "Element index overflows into buffer index");

        mCpuBuffers[bufferIndex].resize(mCpuBuffers[bufferIndex].size() + itemCount);
        return ((bufferIndex << kBufferIndexOffset) | elementIndex);
    }

    void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
    {
        mGpuBuffers.clear();
        mGpuBuffers.reserve(mCpuBuffers.size());
        for (size_t i = 0; i < mCpuBuffers.size(); ++i)
        {
            if (mCpuBuffers[i].empty())
            {
                mGpuBuffers.push_back({});
                continue;
            }

            ref<Buffer> buffer = mpDevice->createStructuredBuffer(
                sizeof(T), mCpuBuffers[i].size(), bindFlags, MemoryType::DeviceLocal, mCpuBuffers[i].data(), false
            );
            buffer->setName(fmt::format("SplitBuffer:{}:[{}]", mBufferName, i));
            mGpuBuffers.push_back(std::move(buffer));
        }
    }

    bool empty() const
    {
        // We check if all CPU buffers are empty. If so, we also check GPU buffers, as CPU buffers
        // maybe have been dropped.
        for (auto& it : mCpuBuffers)
            if (!it.empty())
                return false;
        for (auto& it : mGpuBuffers)
            if (it)
                return false;
        return true;
    }

    size_t getBufferCount() const
    {
        // We check both CPU and GPU buffers, to get correct answer even before `createGpuBuffers`
        // and after `dropCpuData`.
        return std::max(mCpuBuffers.size(), mGpuBuffers.size());
    }

    size_t getByteSize() const
    {
        size_t result = 0;
        if (!mCpuBuffers.empty())
        {
            for (auto& it : mCpuBuffers)
                result += it.size() * sizeof(T);
        }
        else
        {
            for (auto& it : mGpuBuffers)
                result += it->getSize();
        }
        return result;
    }

    uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }

    uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }

    const T& operator[](uint32_t index) const
    {
        FALCOR_ASSERT(!mCpuBuffers.empty());
        const uint32_t bufferIndex = getBufferIndex(index);
        const uint32_t elementIndex = getElementIndex(index);
        return mCpuBuffers[bufferIndex][elementIndex];
    }

    T& operator[](uint32_t index)
    {
        FALCOR_ASSERT(!mCpuBuffers.empty());
        const uint32_t bufferIndex = getBufferIndex(index);
        const uint32_t elementIndex = getElementIndex(index);
        return mCpuBuffers[bufferIndex][elementIndex];
    }

    void dropCpuData() { mCpuBuffers.clear(); }

    bool hasCpuData() const { return !mCpuBuffers.empty(); }

    ref<Buffer> getGpuBuffer(uint32_t bufferIndex) const { return mGpuBuffers[bufferIndex]; }

    const std::vector<T>& getCpuBuffer(uint32_t bufferIndex) const { return mCpuBuffers[bufferIndex]; }

    uint64_t getGpuAddress(uint32_t index) const
    {
        const uint32_t bufferIndex = getBufferIndex(index);
        const uint32_t elementIndex = getElementIndex(index);
        return getGpuBuffer(bufferIndex)->getGpuAddress() + size_t(elementIndex) * sizeof(T);
    }

    void getShaderDefines(DefineList& defines) const
    {
        FALCOR_ASSERT(!mBufferCountDefinePrefix.empty());
        defines.add(mBufferCountDefinePrefix + "_BUFFER_COUNT", std::to_string(mGpuBuffers.size()));
        defines.add(mBufferCountDefinePrefix + "_BUFFER_INDEX_BITS", std::to_string(kBufferIndexBits));
    }

    void bindShaderData(const ShaderVar& var) const
    {
        static const std::string kDataStr = "data";
        for (size_t i = 0; i < mGpuBuffers.size(); ++i)
            var[kDataStr][i] = mGpuBuffers[i];
    }

private:
    static constexpr size_t k4GBSizeLimit = (UINT64_C(1) << UINT64_C(32)) - UINT64_C(1024);
    static constexpr size_t k2GBSizeLimit = (UINT64_C(1) << UINT64_C(31));
    static constexpr size_t kBufferSizeLimit = (sizeof(T) < 16 || TByteBuffer) ? k2GBSizeLimit : k4GBSizeLimit;
    static constexpr size_t kMaxElementCount = (kBufferSizeLimit + 1 - sizeof(T)) / sizeof(T);
    static_assert(kMaxElementCount * sizeof(T) <= kBufferSizeLimit);

    static constexpr size_t kElementIndexBits = bitCount(kMaxElementCount - 1);
    static constexpr size_t kBufferIndexBits = 32 - kElementIndexBits;
    static constexpr size_t kMaxBufferCount = (1u << kBufferIndexBits);
    static_assert(kMaxBufferCount * sizeof(T) <= kBufferSizeLimit);

    static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
    static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);

    std::string mBufferName;
    std::string mBufferCountDefinePrefix;
    std::vector<std::vector<T>> mCpuBuffers;
    std::vector<ref<Buffer>> mGpuBuffers;

    friend class SceneCache;
};
```

**Purpose**: Represents a CPU/GPU buffer that handles GPU limit on 4GB buffers

**Template Parameters**:
- **T**: Type of object stored in the buffer
- **TByteBuffer**: Boolean flag indicating if T is a byte buffer type (default: false)

**Type Definitions**:
```cpp
using ElementType = T;
```

**ElementType**: Alias for element type

#### Constructor

**Default Constructor**:
```cpp
SplitBuffer() : mCpuBuffers(1) {}
```

**Purpose**: Construct SplitBuffer with one buffer

**Behavior**:
- Initializes mCpuBuffers with one empty vector
- Creates one buffer slot
- Default buffer count is 1

#### Configuration Methods

**getMaxBufferCount()**:
```cpp
size_t getMaxBufferCount() const { return kMaxBufferCount; }
```

**Purpose**: Returns maximum number of buffers supported

**Returns**: Maximum buffer count

**setBufferCount()**:
```cpp
void setBufferCount(uint32_t bufferCount)
{
    FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot change buffer count after creating GPU buffers.");
    FALCOR_CHECK(bufferCount >= getBufferCount(), "Cannot reduce number of existing buffers ({}).", getBufferCount());
    FALCOR_CHECK(bufferCount <= kMaxBufferCount, "Cannot exceed max number of buffers ({}).", kMaxBufferCount);
    mCpuBuffers.resize(bufferCount);
}
```

**Purpose**: Sets number of buffers

**Parameters**:
- bufferCount: Number of buffers

**Behavior**:
- Asserts that CPU buffers are empty
- Asserts that bufferCount >= current count
- Asserts that bufferCount <= max count
- Resizes mCpuBuffers to new count

**setName()**:
```cpp
void setName(std::string_view name) { mBufferName = name; }
```

**Purpose**: Sets name used for CPU reporting and GPU buffer names

**Parameters**:
- name: Buffer name

**Behavior**: Sets mBufferName

**setBufferCountDefinePrefix()**:
```cpp
void setBufferCountDefinePrefix(std::string bufferCountDefinePrefix) { mBufferCountDefinePrefix = bufferCountDefinePrefix; }
```

**Purpose**: Sets the name of the define that governs the number of buffers in GPU code

**Parameters**:
- bufferCountDefinePrefix: Prefix for buffer count define

**Behavior**: Sets mBufferCountDefinePrefix

#### Insertion Methods

**insert()**:
```cpp
template<typename Iter>
uint32_t insert(Iter first, Iter last)
{
    FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot insert after creating GPU buffers.");

    if (first == last)
        return 0;

    const size_t itemCount = std::distance(first, last);

    // Find the buffer with fewest items.
    auto it = std::min_element(
        mCpuBuffers.begin(),
        mCpuBuffers.end(),
        [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
    );

    uint32_t bufferIndex = std::distance(mCpuBuffers.begin(), it);

    // If new items wouldn't fit into the buffer with fewest items, create a new buffer,
    // throw if new buffer cannot be created.
    if ((mCpuBuffers[bufferIndex].size() + itemCount) * sizeof(T) > kBufferSizeLimit)
    {
        bufferIndex = mCpuBuffers.size();
        if (bufferIndex >= kMaxBufferCount)
            FALCOR_THROW("Buffers {} cannot accomodate all of data within the buffer limit.", mBufferName);
        mCpuBuffers.push_back({});
    }

    const uint32_t elementIndex = mCpuBuffers[bufferIndex].size();
    FALCOR_ASSERT((((1 << kBufferIndexOffset) - 1) & elementIndex) == elementIndex, "Element index overflows into buffer index");

    mCpuBuffers[bufferIndex].insert(mCpuBuffers[bufferIndex].end(), first, last);
    return ((bufferIndex << kBufferIndexOffset) | elementIndex);
}
```

**Purpose**: Insert range of items into the split buffer

**Parameters**:
- first: Start iterator
- last: End iterator

**Returns**: Index of first inserted item

**Behavior**:
1. Assert that CPU buffers are empty
2. If first == last, return 0
3. Calculate item count
4. Find buffer with fewest items
5. Calculate buffer index
6. If items wouldn't fit, create new buffer
7. Throw if buffer count exceeds max
8. Calculate element index
9. Assert element index doesn't overflow
10. Insert items into buffer
11. Return combined index

**insertEmpty()**:
```cpp
uint32_t insertEmpty(size_t itemCount)
{
    FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot insert after creating GPU buffers.");
    if (itemCount == 0)
        return 0;

    // Find the buffer with fewest items.
    auto it = std::min_element(
        mCpuBuffers.begin(),
        mCpuBuffers.end(),
        [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
    );

    uint32_t bufferIndex = std::distance(mCpuBuffers.begin(), it);

    // If new items wouldn't fit into the buffer with fewest items, create a new buffer,
    // throw if new buffer cannot be created.
    if ((mCpuBuffers[bufferIndex].size() + itemCount) * sizeof(T) > kBufferSizeLimit)
    {
        bufferIndex = mCpuBuffers.size();
        if (bufferIndex >= kMaxBufferCount)
            FALCOR_THROW("Buffers {} cannot accomodate all of data within the buffer limit.", mBufferName);
        mCpuBuffers.push_back({});
    }

    const uint32_t elementIndex = mCpuBuffers[bufferIndex].size();
    FALCOR_ASSERT((((1 << kBufferIndexOffset) - 1) & elementIndex) == elementIndex, "Element index overflows into buffer index");

    mCpuBuffers[bufferIndex].resize(mCpuBuffers[bufferIndex].size() + itemCount);
    return ((bufferIndex << kBufferIndexOffset) | elementIndex);
}
```

**Purpose**: Insert empty range into the split buffer

**Parameters**:
- itemCount: Number of items to insert

**Returns**: Index of first inserted item

**Behavior**:
1. Assert that CPU buffers are empty
2. If itemCount == 0, return 0
3. Find buffer with fewest items
4. Calculate buffer index
5. If items wouldn't fit, create new buffer
6. Throw if buffer count exceeds max
7. Calculate element index
8. Assert element index doesn't overflow
9. Resize buffer to accommodate items
10. Return combined index

#### GPU Buffer Creation

**createGpuBuffers()**:
```cpp
void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
{
    mGpuBuffers.clear();
    mGpuBuffers.reserve(mCpuBuffers.size());
    for (size_t i = 0; i < mCpuBuffers.size(); ++i)
    {
        if (mCpuBuffers[i].empty())
        {
            mGpuBuffers.push_back({});
            continue;
        }

        ref<Buffer> buffer = mpDevice->createStructuredBuffer(
            sizeof(T), mCpuBuffers[i].size(), bindFlags, MemoryType::DeviceLocal, mCpuBuffers[i].data(), false
        );
        buffer->setName(fmt::format("SplitBuffer:{}:[{}]", mBufferName, i));
        mGpuBuffers.push_back(std::move(buffer));
    }
}
```

**Purpose**: Creates GPU buffers, locking further inserts

**Parameters**:
- mpDevice: Device pointer
- bindFlags: Resource bind flags

**Behavior**:
1. Clear existing GPU buffers
2. Reserve space for GPU buffers
3. For each CPU buffer:
   - If empty, create empty GPU buffer
   - Create structured buffer from CPU data
   - Set buffer name
   - Add to GPU buffers list

#### Query Methods

**empty()**:
```cpp
bool empty() const
{
    // We check if all CPU buffers are empty. If so, we also check GPU buffers, as CPU buffers
    // maybe have been dropped.
    for (auto& it : mCpuBuffers)
        if (!it.empty())
            return false;
    for (auto& it : mGpuBuffers)
        if (it)
            return false;
    return true;
}
```

**Purpose**: Return true when SplitBuffer is empty

**Returns**: true if empty, false otherwise

**Behavior**:
- Checks all CPU buffers
- Checks all GPU buffers
- Returns true only if all are empty

**getBufferCount()**:
```cpp
size_t getBufferCount() const
{
    // We check both CPU and GPU buffers, to get correct answer even before `createGpuBuffers`
    // and after `dropCpuData`.
    return std::max(mCpuBuffers.size(), mGpuBuffers.size());
}
```

**Purpose**: Returns number of buffers

**Returns**: Number of buffers

**Behavior**:
- Returns max of CPU and GPU buffer counts
- Works before and after GPU buffer creation

**getByteSize()**:
```cpp
size_t getByteSize() const
{
    size_t result = 0;
    if (!mCpuBuffers.empty())
    {
        for (auto& it : mCpuBuffers)
            result += it.size() * sizeof(T);
    }
    else
        {
            for (auto& it : mGpuBuffers)
                result += it->getSize();
        }
    return result;
}
```

**Purpose**: Returns total byte size used by buffers

**Returns**: Total byte size

**Behavior**:
- If CPU buffers exist, sum their sizes
- Otherwise, sum GPU buffer sizes

#### Index Conversion Methods

**getBufferIndex()**:
```cpp
uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }
```

**Purpose**: Extracts buffer index from combined index

**Parameters**:
- index: Combined index

**Returns**: Buffer index

**Behavior**: Shifts right by kBufferIndexOffset bits

**getElementIndex()**:
```cpp
uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }
```

**Purpose**: Extracts element index from combined index

**Parameters**:
- index: Combined index

**Returns**: Element index

**Behavior**: Masks with kElementIndexMask

#### Accessor Methods

**operator[]() const**:
```cpp
const T& operator[](uint32_t index) const
{
    FALCOR_ASSERT(!mCpuBuffers.empty());
    const uint32_t bufferIndex = getBufferIndex(index);
    const uint32_t elementIndex = getElementIndex(index);
    return mCpuBuffers[bufferIndex][elementIndex];
}
```

**Purpose**: Access element by combined index (const)

**Parameters**:
- index: Combined index

**Returns**: Reference to element

**Behavior**:
- Asserts CPU buffers not empty
- Extracts buffer and element indices
- Returns element from CPU buffer

**operator[]()**:
```cpp
T& operator[](uint32_t index)
{
    FALCOR_ASSERT(!mCpuBuffers.empty());
    const uint32_t bufferIndex = getBufferIndex(index);
    const uint32_t elementIndex = getElementIndex(index);
    return mCpuBuffers[bufferIndex][elementIndex];
}
```

**Purpose**: Access element by combined index

**Parameters**:
- index: Combined index

**Returns**: Reference to element

**Behavior**:
- Asserts CPU buffers not empty
- Extracts buffer and element indices
- Returns element from CPU buffer

#### Memory Management Methods

**dropCpuData()**:
```cpp
void dropCpuData() { mCpuBuffers.clear(); }
```

**Purpose**: Removes all CPU data to conserve memory

**Behavior**: Clears all CPU buffers

**hasCpuData()**:
```cpp
bool hasCpuData() const { return !mCpuBuffers.empty(); }
```

**Purpose**: Checks if CPU data is present

**Returns**: true if CPU data exists, false otherwise

**Behavior**: Checks if CPU buffers not empty

#### GPU Buffer Access Methods

**getGpuBuffer()**:
```cpp
ref<Buffer> getGpuBuffer(uint32_t bufferIndex) const { return mGpuBuffers[bufferIndex]; }
```

**Purpose**: Get GPU buffer by buffer index

**Parameters**:
- bufferIndex: Buffer index

**Returns**: Reference to GPU buffer

**Behavior**: Returns GPU buffer at specified index

**getCpuBuffer()**:
```cpp
const std::vector<T>& getCpuBuffer(uint32_t bufferIndex) const { return mCpuBuffers[bufferIndex]; }
```

**Purpose**: Get CPU buffer by buffer index

**Parameters**:
- bufferIndex: Buffer index

**Returns**: Reference to CPU buffer

**Behavior**: Returns CPU buffer at specified index

**getGpuAddress()**:
```cpp
uint64_t getGpuAddress(uint32_t index) const
{
    const uint32_t bufferIndex = getBufferIndex(index);
    const uint32_t elementIndex = getElementIndex(index);
    return getGpuBuffer(bufferIndex)->getGpuAddress() + size_t(elementIndex) * sizeof(T);
}
```

**Purpose**: Get GPU address of element by combined index

**Parameters**:
- index: Combined index

**Returns**: GPU address of element

**Behavior**:
- Extracts buffer and element indices
- Gets GPU buffer address
- Adds element offset
- Returns GPU address

#### Shader Integration Methods

**getShaderDefines()**:
```cpp
void getShaderDefines(DefineList& defines) const
{
    FALCOR_ASSERT(!mBufferCountDefinePrefix.empty());
    defines.add(mBufferCountDefinePrefix + "_BUFFER_COUNT", std::to_string(mGpuBuffers.size()));
    defines.add(mBufferCountDefinePrefix + "_BUFFER_INDEX_BITS", std::to_string(kBufferIndexBits));
}
```

**Purpose**: Adds shader defines for buffer count and index bits

**Parameters**:
- defines: Define list to add defines to

**Behavior**:
- Asserts buffer count define prefix is set
- Adds buffer count define
- Adds buffer index bits define

**bindShaderData()**:
```cpp
void bindShaderData(const ShaderVar& var) const
{
    static const std::string kDataStr = "data";
    for (size_t i = 0; i < mGpuBuffers.size(); ++i)
        var[kDataStr][i] = mGpuBuffers[i];
}
```

**Purpose**: Binds all GPU buffers to shader variable

**Parameters**:
- var: Shader variable to bind to

**Behavior**:
- Binds each GPU buffer to corresponding array element in shader variable

#### Private Members

**Static Constants**:
```cpp
static constexpr size_t k4GBSizeLimit = (UINT64_C(1) << UINT64_C(32)) - UINT64_C(1024);
static constexpr size_t k2GBSizeLimit = (UINT64_C(1) << UINT64_C(31));
static constexpr size_t kBufferSizeLimit = (sizeof(T) < 16 || TByteBuffer) ? k2GBSizeLimit : k4GBSizeLimit;
static constexpr size_t kMaxElementCount = (kBufferSizeLimit + 1 - sizeof(T)) / sizeof(T);
static_assert(kMaxElementCount * sizeof(T) <= kBufferSizeLimit);

static constexpr size_t kElementIndexBits = bitCount(kMaxElementCount - 1);
static constexpr size_t kBufferIndexBits = 32 - kElementIndexBits;
static constexpr size_t kMaxBufferCount = (1u << kBufferIndexBits);
static_assert(kMaxBufferCount * sizeof(T) <= kBufferSizeLimit);

static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);
```

**k4GBSizeLimit**: 4GB size limit (4 * 1024^3 bytes)

**k2GBSizeLimit**: 2GB size limit (2 * 1024^3 bytes)

**kBufferSizeLimit**: Buffer size limit (2GB for byte buffers, 4GB otherwise)

**kMaxElementCount**: Maximum number of elements in a buffer

**kElementIndexBits**: Number of bits needed for element index

**kBufferIndexBits**: Number of bits needed for buffer index

**kMaxBufferCount**: Maximum number of buffers

**kBufferIndexOffset**: Bit offset for buffer index in combined index

**kElementIndexMask**: Mask for extracting element index from combined index

**Private Members**:
```cpp
std::string mBufferName;
std::string mBufferCountDefinePrefix;
std::vector<std::vector<T>> mCpuBuffers;
std::vector<ref<Buffer>> mGpuBuffers;
```

**mBufferName**: Name used for CPU reporting and GPU buffer names

**mBufferCountDefinePrefix**: Prefix for buffer count define in GPU code

**mCpuBuffers**: CPU-side data buffers (vector of vectors)

**mGpuBuffers**: GPU-side buffers (vector of Buffer references)

## Technical Details

### GPU Limit Handling

Handles 4GB GPU buffer limit:

```cpp
static constexpr size_t kBufferSizeLimit = (sizeof(T) < 16 || TByteBuffer) ? k2GBSizeLimit : k4GBSizeLimit;
```

**Characteristics**:
- 2GB limit for byte buffers (sizeof(T) < 16)
- 4GB limit for larger buffers
- Workaround for GPU buffer limitations
- Automatic limit selection based on type

### Power-of-2 Encoding

Uses upper bits of index for buffer selection:

```cpp
static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);

uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }
uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }
```

**Characteristics**:
- Upper bits: buffer index
- Lower bits: element index
- Efficient encoding/decoding
- No runtime overhead

### Balanced Buffering

Balances content across all buffers:

```cpp
auto it = std::min_element(
    mCpuBuffers.begin(),
    mCpuBuffers.end(),
    [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
);
```

**Characteristics**:
- Finds buffer with fewest items
- Balances content distribution
- Minimizes maximum buffer size
- Efficient insertion strategy

### Automatic GPU Creation

Creates GPU buffers on demand:

```cpp
void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
{
    mGpuBuffers.clear();
    mGpuBuffers.reserve(mCpuBuffers.size());
    for (size_t i = 0; i < mCpuBuffers.size(); ++i)
    {
        if (mCpuBuffers[i].empty())
        {
            mGpuBuffers.push_back({});
            continue;
        }

        ref<Buffer> buffer = mpDevice->createStructuredBuffer(
            sizeof(T), mCpuBuffers[i].size(), bindFlags, MemoryType::DeviceLocal, mCpuBuffers[i].data(), false
        );
        buffer->setName(fmt::format("SplitBuffer:{}:[{}]", mBufferName, i));
        mGpuBuffers.push_back(std::move(buffer));
    }
}
```

**Characteristics**:
- Lazy GPU buffer creation
- Device-local memory
- Automatic buffer naming
- Locks further inserts
- Efficient buffer creation

### CPU Data Management

Manages CPU-side data:

```cpp
std::vector<std::vector<T>> mCpuBuffers;
```

**Characteristics**:
- Vector of vectors for each buffer
- Efficient memory layout
- Easy data access
- Automatic memory management

### Index Encoding

Encodes buffer and element indices into single index:

```cpp
uint32_t insert(Iter first, Iter last)
{
    // ...
    return ((bufferIndex << kBufferIndexOffset) | elementIndex);
}
```

**Encoding**:
- Upper bits: buffer index (0 to kMaxBufferCount-1)
- Lower bits: element index (0 to kMaxElementCount-1)
- Combined index: (bufferIndex << kBufferIndexOffset) | elementIndex

**Decoding**:
- Buffer index: index >> kBufferIndexOffset
- Element index: index & kElementIndexMask

### Shader Integration

Adds shader defines and binds buffers:

```cpp
void getShaderDefines(DefineList& defines) const
{
    defines.add(mBufferCountDefinePrefix + "_BUFFER_COUNT", std::to_string(mGpuBuffers.size()));
    defines.add(mBufferCountDefinePrefix + "_BUFFER_INDEX_BITS", std::to_string(kBufferIndexBits));
}

void bindShaderData(const ShaderVar& var) const
{
    static const std::string kDataStr = "data";
    for (size_t i = 0; i < mGpuBuffers.size(); ++i)
        var[kDataStr][i] = mGpuBuffers[i];
}
```

**Characteristics**:
- Configurable define prefix
- Automatic buffer count define
- Automatic buffer index bits define
- Array-based shader binding
- Efficient shader integration

### Memory Efficiency

Minimizes memory overhead:

```cpp
static constexpr size_t kMaxElementCount = (kBufferSizeLimit + 1 - sizeof(T)) / sizeof(T);
```

**Characteristics**:
- Calculates maximum elements per buffer
- Accounts for element size
- Efficient memory usage
- No wasted space

## Integration Points

### Falcor Core Integration

- **Core/API/Buffer.h**: Buffer class
- **Core/API/Device.h**: Device, ResourceBindFlags
- **Core/Program/DefineList.h**: DefineList
- **Core/Program/ShaderVar.h**: ShaderVar
- **Core/Error**: FALCOR_ASSERT, FALCOR_CHECK, FALCOR_THROW

### STL Integration

- **std::vector**: Vector container for CPU buffers
- **std::min_element**: Finding buffer with fewest items
- **std::distance**: Calculating item count
- **std::move**: Moving GPU buffers

### Internal Falcor Usage

- **Scene**: Scene caching
- **Rendering**: Rendering buffer management
- **Utils**: Internal utilities

## Architecture Patterns

### GPU Limit Workaround Pattern

Works around 4GB GPU buffer limit:

```cpp
static constexpr size_t kBufferSizeLimit = (sizeof(T) < 16 || TByteBuffer) ? k2GBSizeLimit : k4GBSizeLimit;
```

Benefits:
- Handles GPU limitations
- Automatic limit selection
- Type-aware sizing
- Transparent to user

### Power-of-2 Encoding Pattern

Encodes buffer and element indices:

```cpp
static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);

uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }
uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }
```

Benefits:
- Efficient encoding/decoding
- No runtime overhead
- Compact representation
- Easy to use

### Balanced Buffering Pattern

Balances content across all buffers:

```cpp
auto it = std::min_element(
    mCpuBuffers.begin(),
    mCpuBuffers.end(),
    [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
);
```

Benefits:
- Minimizes maximum buffer size
- Balanced distribution
- Efficient insertion
- Automatic balancing

### Lazy Initialization Pattern

Creates GPU buffers on demand:

```cpp
void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
{
    // ... lazy GPU buffer creation ...
}
```

Benefits:
- No upfront allocation
- Create on first use
- Efficient resource usage
- Automatic cleanup

### RAII Pattern

Uses RAII for resource management:

```cpp
std::vector<ref<Buffer>> mGpuBuffers;
```

Benefits:
- Automatic cleanup
- Exception safety
- No manual resource management
- Efficient memory management

## Code Patterns

### Power-of-2 Encoding Pattern

Encode buffer and element indices:

```cpp
static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);

uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }
uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }
```

Pattern:
- Upper bits: buffer index
- Lower bits: element index
- Bit shifting for encoding
- Bit masking for decoding

### Balanced Insertion Pattern

Insert into buffer with fewest items:

```cpp
auto it = std::min_element(
    mCpuBuffers.begin(),
    mCpuBuffers.end(),
    [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
);
```

Pattern:
- Find buffer with fewest items
- Calculate buffer index
- Insert into buffer
- Return combined index

### Lazy GPU Creation Pattern

Create GPU buffers on demand:

```cpp
void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
{
    mGpuBuffers.clear();
    mGpuBuffers.reserve(mCpuBuffers.size());
    for (size_t i = 0; i < mCpuBuffers.size(); ++i)
    {
        if (mCpuBuffers[i].empty())
        {
            mGpuBuffers.push_back({});
            continue;
        }

        ref<Buffer> buffer = mpDevice->createStructuredBuffer(
            sizeof(T), mCpuBuffers[i].size(), bindFlags, MemoryType::DeviceLocal, mCpuBuffers[i].data(), false
        );
        buffer->setName(fmt::format("SplitBuffer:{}:[{}]", mBufferName, i));
        mGpuBuffers.push_back(std::move(buffer));
    }
}
```

Pattern:
- Clear existing buffers
- Reserve space
- Create buffers from CPU data
- Set buffer names
- Add to list

### Index Access Pattern

Access element by combined index:

```cpp
const T& operator[](uint32_t index) const
{
    FALCOR_ASSERT(!mCpuBuffers.empty());
    const uint32_t bufferIndex = getBufferIndex(index);
    const uint32_t elementIndex = getElementIndex(index);
    return mCpuBuffers[bufferIndex][elementIndex];
}
```

Pattern:
- Assert buffers not empty
- Extract buffer index
- Extract element index
- Return element from CPU buffer

### Assertion Pattern

Assert conditions for validation:

```cpp
FALCOR_ASSERT(mCpuBuffers.empty(), "Cannot change buffer count after creating GPU buffers.");
FALCOR_CHECK(bufferCount >= getBufferCount(), "Cannot reduce number of existing buffers ({}).", getBufferCount());
FALCOR_CHECK(bufferCount <= kMaxBufferCount, "Cannot exceed max number of buffers ({}).", kMaxBufferCount);
FALCOR_ASSERT((((1 << kBufferIndexOffset) - 1) & elementIndex) == elementIndex, "Element index overflows into buffer index");
```

Pattern:
- Check preconditions
- Provide clear error messages
- Use appropriate assertion macro
- Include context in error messages

## Use Cases

### Basic Usage

```cpp
SplitBuffer<float3> buffer;

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());

// Access items
float3 item = buffer[firstIndex];
```

### Multiple Buffers

```cpp
SplitBuffer<float3> buffer;

// Set buffer count
buffer.setBufferCount(4);

// Create GPU buffers
buffer.createGpuBuffers(mpDevice, ResourceBindFlags::ShaderResource);
```

### Byte Buffer Type

```cpp
SplitBuffer<uint8_t, true> buffer;

// Uses 2GB limit instead of 4GB
buffer.setBufferCount(8);

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());

// Access items
uint8_t item = buffer[firstIndex];
```

### Custom Buffer Count

```cpp
SplitBuffer<float3> buffer;

// Set custom buffer count
buffer.setBufferCount(2);

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());
```

### Empty Range

```cpp
SplitBuffer<float3> buffer;

// Insert empty range
uint32_t firstIndex = buffer.insertEmpty(100);
```

### CPU Data Drop

```cpp
SplitBuffer<float3> buffer;

// Insert items
buffer.insert(data.begin(), data.end());

// Drop CPU data to save memory
buffer.dropCpuData();

// GPU buffers are still valid
```

### Shader Integration

```cpp
SplitBuffer<float3> buffer;

// Create GPU buffers
buffer.createGpuBuffers(mpDevice, ResourceBindFlags::ShaderResource);

// Get shader defines
DefineList defines;
buffer.getShaderDefines(defines);

// Bind to shader
ShaderVar var;
buffer.bindShaderData(var);
```

### Index Encoding

```cpp
SplitBuffer<float3> buffer;

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());

// Extract buffer and element indices
uint32_t bufferIndex = buffer.getBufferIndex(firstIndex);
uint32_t elementIndex = buffer.getElementIndex(firstIndex);

// Use indices
float3& element = buffer[firstIndex];
```

### Memory Statistics

```cpp
SplitBuffer<float3> buffer;

// Insert items
buffer.insert(data.begin(), data.end());

// Get byte size
size_t byteSize = buffer.getByteSize();

// Get buffer count
size_t bufferCount = buffer.getBufferCount();
```

## Performance Considerations

### Memory Overhead

- **CPU Buffers**: Vector overhead for each buffer
- **GPU Buffers**: Buffer object overhead for each buffer
- **Index Encoding**: No runtime overhead (bit operations)
- **Balancing**: Minimal overhead for balancing

### Insertion Performance

- **Buffer Selection**: O(n) for finding buffer with fewest items
- **Insertion**: O(m) for inserting into vector
- **Index Calculation**: Bit operations (constant time)
- **Overall**: O(n + m) for insertion

### Access Performance

- **Index Decoding**: Bit operations (constant time)
- **Element Access**: Direct array access (constant time)
- **Overall**: O(1) for access

### GPU Creation Performance

- **Buffer Creation**: O(n) for n buffers
- **Memory Type**: Device-local memory (fast)
- **Overall**: O(n) for GPU creation

### Memory Management

- **CPU Storage**: Vector of vectors (efficient)
- **GPU Storage**: Vector of Buffer references
- **Drop CPU Data**: Clears all CPU vectors
- **Overall**: Minimal overhead

### Compiler Optimizations

- **Inline Expansion**: Methods can be inlined
- **Constant Propagation**: Constants can be propagated
- **Dead Code Elimination**: Unused code can be eliminated
- **Template Instantiation**: Template instantiation overhead

### Comparison with Alternatives

**SplitBuffer vs Single Buffer**:
- SplitBuffer: Handles 4GB limit, multiple buffers
- Single Buffer: No limit, but may exceed GPU limit
- Performance: SplitBuffer has minimal overhead

**SplitBuffer vs Vector of Buffers**:
- SplitBuffer: Automatic balancing, efficient insertion
- Vector of Buffers: Manual balancing, more control
- Performance: SplitBuffer is more efficient

### Optimization Tips

1. **Use Appropriate Buffer Count**: Use appropriate buffer count for data size
2. **Use Byte Buffer Type**: Use byte buffer type for large data
3. **Drop CPU Data**: Drop CPU data when GPU buffers are created
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

## Limitations

### Feature Limitations

- **No Buffer Invalidation**: No method to invalidate buffers
- **No Buffer Clearing**: No method to clear buffers (except dropCpuData)
- **No Buffer Removal**: No method to remove buffers
- **No Buffer Resizing**: No method to resize individual buffers
- **No Buffer Merging**: No method to merge buffers
- **No Buffer Splitting**: No method to split buffers after creation
- **No Capacity Query**: No method to get buffer capacity
- **No Usage Statistics**: No statistics on buffer usage
- **No Buffer Selection**: No method to select specific buffer for insertion

### API Limitations

- **No Erase Method**: No method to erase items
- **No Clear Method**: No method to clear all items
- **No Reserve Method**: No method to reserve capacity
- **No Size Method**: No method to get item count
- **No Capacity Method**: No method to get capacity
- **No Iteration**: No iterators for traversal
- **No Range-Based Access**: No range-based access

### Type Limitations

- **Power-of-2 Encoding**: Limited to 32-bit indices
- **Buffer Index Bits**: Limited by element size
- **Maximum Items**: Limited by buffer size and count
- **Type Requirements**: T must be trivially copyable

### Performance Limitations

- **Balancing Overhead**: Balancing algorithm overhead
- **GPU Creation Overhead**: GPU buffer creation overhead
- **Index Encoding**: Bit operation overhead (minimal)
- **Memory Overhead**: CPU and GPU buffer overhead

### Platform Limitations

- **C++11 Only**: Requires C++11 or later
- **GPU Limit**: Limited by 4GB buffer size
- **Device-Local Memory**: Uses device-local memory

## Best Practices

### When to Use

**Appropriate Use Cases**:
- Large datasets exceeding GPU buffer limit
- Power-of-2 byte sizes (1, 2, 4, 8, 16 bytes)
- Need for multiple GPU buffers
- Balanced data distribution
- Automatic buffer management

**Inappropriate Use Cases**:
- Small datasets (use single buffer)
- Non-power-of-2 types (use custom solution)
- Single GPU buffer (use single buffer)
- Manual buffer management (use custom solution)

### Usage Patterns

**Basic Usage**:
```cpp
SplitBuffer<float3> buffer;

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());

// Access items
float3 item = buffer[firstIndex];
```

**Multiple Buffers**:
```cpp
SplitBuffer<float3> buffer;

// Set buffer count
buffer.setBufferCount(4);

// Create GPU buffers
buffer.createGpuBuffers(mpDevice, ResourceBindFlags::ShaderResource);
```

**Byte Buffer Type**:
```cpp
SplitBuffer<uint8_t, true> buffer;

// Uses 2GB limit instead of 4GB
buffer.setBufferCount(8);

// Insert items
uint32_t firstIndex = buffer.insert(data.begin(), data.end());

// Access items
uint8_t item = buffer[firstIndex];
```

### Performance Tips

1. **Use Appropriate Buffer Count**: Use appropriate buffer count for data size
2. **Use Byte Buffer Type**: Use byte buffer type for large data
3. **Drop CPU Data**: Drop CPU data when GPU buffers are created
4. **Enable Optimizations**: Enable compiler optimizations
5. **Profile**: Profile to identify bottlenecks

### Memory Management

- **Drop CPU Data**: Drop CPU data when GPU buffers are created
- **Use Appropriate Buffer Count**: Use appropriate buffer count
- **Use Byte Buffer Type**: Use byte buffer type for large data
- **Minimize Overhead**: Minimize memory overhead

### Error Handling

- **Handle Exceptions**: Handle FALCOR_THROW exceptions
- **Check Preconditions**: Check preconditions before operations
- **Use Assertions**: Use assertions for debugging
- **Provide Clear Messages**: Provide clear error messages

## Implementation Notes

### GPU Limit Handling

Handles 4GB GPU buffer limit:

```cpp
static constexpr size_t kBufferSizeLimit = (sizeof(T) < 16 || TByteBuffer) ? k2GBSizeLimit : k4GBSizeLimit;
```

**Characteristics**:
- 2GB limit for byte buffers (sizeof(T) < 16)
- 4GB limit for larger buffers
- Workaround for GPU buffer limitations
- Automatic limit selection based on type

### Power-of-2 Encoding Implementation

Encodes buffer and element indices:

```cpp
static constexpr size_t kBufferIndexOffset = 32 - kBufferIndexBits;
static constexpr size_t kElementIndexMask = ((1 << kBufferIndexOffset) - 1);

uint32_t getBufferIndex(uint32_t index) const { return (index >> kBufferIndexOffset); }
uint32_t getElementIndex(uint32_t index) const { return index & kElementIndexMask; }
```

**Characteristics**:
- Upper bits: buffer index
- Lower bits: element index
- Efficient encoding/decoding
- No runtime overhead

### Balanced Buffering Implementation

Balances content across all buffers:

```cpp
auto it = std::min_element(
    mCpuBuffers.begin(),
    mCpuBuffers.end(),
    [](const std::vector<T>& lhs, const std::vector<T>& rhs) { return lhs.size() < rhs.size(); }
);
```

**Characteristics**:
- Finds buffer with fewest items
- Balances content distribution
- Minimizes maximum buffer size
- Efficient insertion strategy

### GPU Buffer Creation Implementation

Creates GPU buffers on demand:

```cpp
void createGpuBuffers(const ref<Device>& mpDevice, ResourceBindFlags bindFlags)
{
    mGpuBuffers.clear();
    mGpuBuffers.reserve(mCpuBuffers.size());
    for (size_t i = 0; i < mCpuBuffers.size(); ++i)
    {
        if (mCpuBuffers[i].empty())
        {
            mGpuBuffers.push_back({});
            continue;
        }

        ref<Buffer> buffer = mpDevice->createStructuredBuffer(
            sizeof(T), mCpuBuffers[i].size(), bindFlags, MemoryType::DeviceLocal, mCpuBuffers[i].data(), false
        );
        buffer->setName(fmt::format("SplitBuffer:{}:[{}]", mBufferName, i));
        mGpuBuffers.push_back(std::move(buffer));
    }
}
```

**Characteristics**:
- Lazy GPU buffer creation
- Device-local memory
- Automatic buffer naming
- Locks further inserts
- Efficient buffer creation

### Index Encoding Implementation

Encodes buffer and element indices:

```cpp
uint32_t insert(Iter first, Iter last)
{
    // ...
    return ((bufferIndex << kBufferIndexOffset) | elementIndex);
}
```

**Characteristics**:
- Upper bits: buffer index
- Lower bits: element index
- Combined index: (bufferIndex << kBufferIndexOffset) | elementIndex
- Efficient encoding/decoding

## Future Enhancements

### Potential Improvements

1. **Buffer Invalidation**: Add method to invalidate buffers
2. **Buffer Clearing**: Add method to clear buffers
3. **Buffer Removal**: Add method to remove buffers
4. **Buffer Resizing**: Add method to resize individual buffers
5. **Buffer Merging**: Add method to merge buffers
6. **Buffer Splitting**: Add method to split buffers after creation
7. **Capacity Query**: Add method to get buffer capacity
8. **Usage Statistics**: Add statistics on buffer usage
9. **Buffer Selection**: Add method to select specific buffer for insertion
10. **Iteration Support**: Add iterators for traversal

### API Extensions

1. **erase()**: Add method to erase items
2. **clear()**: Add method to clear all items
3. **reserve()**: Add method to reserve capacity
4. **size()**: Add method to get item count
5. **capacity()**: Add method to get capacity
6. **begin()/end()**: Add iterator support
7. **rbegin()/rend()**: Add const iterator support
8. **front()/back()**: Add front/back accessors
9. **at()**: Add bounds-checked access
10. **data()**: Add method to get raw data

### Performance Enhancements

1. **Better Balancing**: Add better balancing algorithms
2. **Lazy GPU Creation**: Add more lazy GPU creation
3. **Buffer Pooling**: Add buffer pooling for better performance
4. **Async Operations**: Add async operations for better performance
5. **Memory Pool**: Add memory pool for better performance

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Best Practices**: Add best practices guide
4. **Migration Guide**: Add migration guide from single buffer
5. **API Reference**: Add complete API reference

## References

### C++ Standard

- **std::vector**: C++11 vector container
- **std::min_element**: C++11 algorithm
- **std::distance**: C++11 iterator algorithm
- **std::move**: C++11 move semantics

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Utils**: Utils module documentation

### Related Technologies

- **GPU Buffers**: GPU buffer management
- **Memory Management**: Memory management patterns
- **Power-of-2 Encoding**: Power-of-2 encoding
- **Balanced Data Distribution**: Balanced data distribution algorithms
