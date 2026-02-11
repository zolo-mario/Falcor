---
modified: 2026-01-20T10:43:22+08:00
created: 2026-01-13T08:39:58+08:00
tags:
  - 2026/01/13
---
# CudaUtils - CUDA Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Dependencies

- Falcor/Core (Macros, Object, API/fwd, API/Handles, API/Device, API/Fence, Error, API/Buffer, API/Texture)
- CUDA Driver API (<cuda.h>)
- CUDA Runtime API (<cuda_runtime.h>)
- Falcor/Utils/CudaRuntime (for type isolation)

### Dependent Modules

- Falcor/DiffRendering (PyTorch/CUDA integration)
- Falcor/Rendering (CUDA-accelerated rendering)
- Falcor/Scene (CUDA-based scene processing)

## Module Overview

CudaUtils provides high-level utilities for integrating NVIDIA CUDA with Falcor's rendering system. The module offers memory management functions, device synchronization, external resource import/export for DirectX/Vulkan interop, and wrapper classes for CUDA devices, external memory, and external semaphores. It enables seamless data sharing between Falcor's graphics API (D3D12/Vulkan) and CUDA for hybrid CPU-GPU-CUDA workflows.

## Component Specifications

### Error Checking Macros

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 41-64)

Two macros provide error checking for CUDA API calls.

#### FALCOR_CUDA_CHECK Macro

```cpp
#define FALCOR_CUDA_CHECK(call) \
    { \
        cudaError_t result = call; \
        if (result != cudaSuccess) \
        { \
            const char* errorName = cudaGetErrorName(result); \
            const char* errorString = cudaGetErrorString(result); \
            FALCOR_THROW("CUDA call {} failed with error {} ({}).", #call, errorName, errorString); \
        } \
    }
```

**Purpose**: Checks CUDA runtime API call results

**Behavior**:
- Executes the CUDA call
- Checks if result is cudaSuccess
- On failure, retrieves error name and string
- Throws exception with descriptive message including:
  - The actual call (via `#call`)
  - Error name (e.g., "cudaErrorInvalidValue")
  - Error description (e.g., "Invalid argument")

**Usage**:
```cpp
FALCOR_CUDA_CHECK(cudaMalloc(&ptr, size));
```

#### FALCOR_CU_CHECK Macro

```cpp
#define FALCOR_CU_CHECK(call) \
    do \
    { \
        CUresult result = call; \
        if (result != CUDA_SUCCESS) \
        { \
            const char* errorName; \
            cuGetErrorName(result, &errorName); \
            const char* errorString; \
            cuGetErrorString(result, &errorString); \
            FALCOR_THROW("CUDA call {} failed with error {} ({}).", #call, errorName, errorString); \
        } \
    } while (0)
```

**Purpose**: Checks CUDA driver API call results

**Behavior**:
- Executes the CUDA driver call
- Checks if result is CUDA_SUCCESS
- On failure, retrieves error name and string
- Throws exception with descriptive message

**Usage**:
```cpp
FALCOR_CU_CHECK(cuDeviceGet(&device, 0));
```

### Type Definition

**File**: `Source/Falcor/Utils/CudaUtils.h` (line 67)

```cpp
typedef unsigned long long CUdeviceptr;
```

**Purpose**: Defines CUDA device pointer type

**Note**: This type is normally defined by CUDA driver API but is defined here for compatibility.

### cuda_utils Namespace Functions

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 72-128)

#### deviceSynchronize()

```cpp
FALCOR_API void deviceSynchronize();
```

**Purpose**: Synchronizes CUDA device

**Implementation** ([`CudaUtils.cpp:51`](Source/Falcor/Utils/CudaUtils.cpp:51)):
```cpp
void deviceSynchronize()
{
    cudaDeviceSynchronize();
    cudaError_t error = cudaGetLastError();
    FALCOR_CHECK(error == cudaSuccess, "Failed to sync CUDA device: {}.", cudaGetErrorString(error));
}
```

**Behavior**:
- Blocks host execution until all CUDA operations complete
- Checks for any pending CUDA errors
- Throws exception if synchronization fails

#### mallocDevice()

```cpp
FALCOR_API void* mallocDevice(size_t size);
```

**Purpose**: Allocates memory on CUDA device

**Implementation** ([`CudaUtils.cpp:58`](Source/Falcor/Utils/CudaUtils.cpp:58)):
```cpp
void* mallocDevice(size_t size)
{
    void* devPtr;
    FALCOR_CUDA_CHECK(cudaMalloc(&devPtr, size));
    return devPtr;
}
```

**Parameters**:
- size: Number of bytes to allocate

**Returns**: Pointer to allocated device memory

#### freeDevice()

```cpp
FALCOR_API void freeDevice(void* devPtr);
```

**Purpose**: Frees CUDA device memory

**Implementation** ([`CudaUtils.cpp:65`](Source/Falcor/Utils/CudaUtils.cpp:65)):
```cpp
void freeDevice(void* devPtr)
{
    if (!devPtr)
        return;
    FALCOR_CUDA_CHECK(cudaFree(devPtr));
}
```

**Parameters**:
- devPtr: Pointer to device memory to free (null-safe)

#### memcpyDeviceToDevice()

```cpp
FALCOR_API void memcpyDeviceToDevice(void* dst, const void* src, size_t count);
```

**Purpose**: Copies memory between CUDA device locations

**Implementation** ([`CudaUtils.cpp:72`](Source/Falcor/Utils/CudaUtils.cpp:72)):
```cpp
void memcpyDeviceToDevice(void* dst, const void* src, size_t count)
{
    FALCOR_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyDeviceToDevice));
}
```

**Parameters**:
- dst: Destination device pointer
- src: Source device pointer
- count: Number of bytes to copy

#### memcpyHostToDevice()

```cpp
FALCOR_API void memcpyHostToDevice(void* dst, const void* src, size_t count);
```

**Purpose**: Copies memory from host to CUDA device

**Implementation** ([`CudaUtils.cpp:77`](Source/Falcor/Utils/CudaUtils.cpp:77)):
```cpp
void memcpyHostToDevice(void* dst, const void* src, size_t count)
{
    FALCOR_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyHostToDevice));
}
```

**Parameters**:
- dst: Destination device pointer
- src: Source host pointer
- count: Number of bytes to copy

#### memcpyDeviceToHost()

```cpp
FALCOR_API void memcpyDeviceToHost(void* dst, const void* src, size_t count);
```

**Purpose**: Copies memory from CUDA device to host

**Implementation** ([`CudaUtils.cpp:82`](Source/Falcor/Utils/CudaUtils.cpp:82)):
```cpp
void memcpyDeviceToHost(void* dst, const void* src, size_t count)
{
    FALCOR_CUDA_CHECK(cudaMemcpy(dst, src, count, cudaMemcpyDeviceToHost));
}
```

**Parameters**:
- dst: Destination host pointer
- src: Source device pointer
- count: Number of bytes to copy

#### memsetDevice()

```cpp
FALCOR_API void memsetDevice(void* devPtr, int value, size_t count);
```

**Purpose**: Sets CUDA device memory to a value

**Implementation** ([`CudaUtils.cpp:87`](Source/Falcor/Utils/CudaUtils.cpp:87)):
```cpp
void memsetDevice(void* devPtr, int value, size_t count)
{
    FALCOR_CUDA_CHECK(cudaMemset(devPtr, value, count));
}
```

**Parameters**:
- devPtr: Device pointer to memory
- value: Byte value to set (typically 0)
- count: Number of bytes to set

#### importExternalMemory()

```cpp
FALCOR_API cudaExternalMemory_t importExternalMemory(const Buffer* buffer);
```

**Purpose**: Imports a Falcor buffer as CUDA external memory

**Implementation** ([`CudaUtils.cpp:92`](Source/Falcor/Utils/CudaUtils.cpp:92)):
```cpp
cudaExternalMemory_t importExternalMemory(const Buffer* buffer)
{
    FALCOR_CHECK(buffer, "'buffer' is nullptr.");
    FALCOR_CHECK(is_set(buffer->getBindFlags(), ResourceBindFlags::Shared), "Buffer must be created with ResourceBindFlags::Shared.");
    SharedResourceApiHandle sharedHandle = buffer->getSharedApiHandle();
    FALCOR_CHECK(sharedHandle, "Buffer shared handle creation failed.");

    cudaExternalMemoryHandleDesc desc = {};
    switch (buffer->getDevice()->getType())
    {
#if FALCOR_WINDOWS
    case Device::Type::D3D12:
        desc.type = cudaExternalMemoryHandleTypeD3D12Resource;
        desc.handle.win32.handle = sharedHandle;
        break;
    case Device::Type::Vulkan:
        desc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        desc.handle.win32.handle = sharedHandle;
        break;
#elif FALCOR_LINUX
    case Device::Type::Vulkan:
        desc.type = cudaExternalMemoryHandleTypeOpaqueFd;
        desc.handle.fd = (int)reinterpret_cast<intptr_t>(sharedHandle);
        break;
#endif
    default:
        FALCOR_THROW("Unsupported device type '{}'.", buffer->getDevice()->getType());
    }
    desc.size = buffer->getSize();
    desc.flags = cudaExternalMemoryDedicated;

    cudaExternalMemory_t extMem;
    FALCOR_CUDA_CHECK(cudaImportExternalMemory(&extMem, &desc));
    return extMem;
}
```

**Parameters**:
- buffer: Falcor buffer to import (must have ResourceBindFlags::Shared)

**Returns**: CUDA external memory handle

**Behavior**:
- Validates buffer is not null
- Validates buffer has Shared bind flag
- Gets shared API handle from buffer
- Creates external memory descriptor based on device type:
  - D3D12 on Windows: cudaExternalMemoryHandleTypeD3D12Resource
  - Vulkan on Windows: cudaExternalMemoryHandleTypeOpaqueWin32
  - Vulkan on Linux: cudaExternalMemoryHandleTypeOpaqueFd
- Imports external memory with CUDA

#### destroyExternalMemory()

```cpp
FALCOR_API void destroyExternalMemory(cudaExternalMemory_t extMem);
```

**Purpose**: Destroys CUDA external memory handle

**Implementation** ([`CudaUtils.cpp:128`](Source/Falcor/Utils/CudaUtils.cpp:128)):
```cpp
void destroyExternalMemory(cudaExternalMemory_t extMem)
{
    FALCOR_CUDA_CHECK(cudaDestroyExternalMemory(extMem));
}
```

**Parameters**:
- extMem: External memory handle to destroy

#### externalMemoryGetMappedBuffer()

```cpp
FALCOR_API void* externalMemoryGetMappedBuffer(cudaExternalMemory_t extMem, size_t offset, size_t size);
```

**Purpose**: Maps external memory to a CUDA device pointer

**Implementation** ([`CudaUtils.cpp:133`](Source/Falcor/Utils/CudaUtils.cpp:133)):
```cpp
void* externalMemoryGetMappedBuffer(cudaExternalMemory_t extMem, size_t offset, size_t size)
{
    cudaExternalMemoryBufferDesc desc = {};
    desc.offset = offset;
    desc.size = size;

    void* devPtr = nullptr;
    FALCOR_CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(&devPtr, extMem, &desc));
    return devPtr;
}
```

**Parameters**:
- extMem: External memory handle
- offset: Offset into external memory (bytes)
- size: Size of region to map (bytes)

**Returns**: CUDA device pointer to mapped memory

#### importExternalSemaphore()

```cpp
FALCOR_API cudaExternalSemaphore_t importExternalSemaphore(const Fence* fence);
```

**Purpose**: Imports a Falcor fence as CUDA external semaphore

**Implementation** ([`CudaUtils.cpp:144`](Source/Falcor/Utils/CudaUtils.cpp:144)):
```cpp
cudaExternalSemaphore_t importExternalSemaphore(const Fence* fence)
{
    FALCOR_CHECK(fence, "'fence' is nullptr.");
    FALCOR_CHECK(fence->getDesc().shared, "'fence' must be created with shared=true.");
    SharedFenceApiHandle sharedHandle = fence->getSharedApiHandle();
    FALCOR_CHECK(sharedHandle, "Fence shared handle creation failed.");

    cudaExternalSemaphoreHandleDesc desc = {};
    switch (fence->getDevice()->getType())
    {
#if FALCOR_WINDOWS
    case Device::Type::D3D12:
        desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence;
        desc.handle.win32.handle = sharedHandle;
        break;
    case Device::Type::Vulkan:
        desc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32;
        desc.handle.win32.handle = sharedHandle;
        break;
#elif FALCOR_LINUX
    case Device::Type::Vulkan:
        desc.type = cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd;
        desc.handle.fd = (int)reinterpret_cast<intptr_t>(sharedHandle);
        break;
#endif
    default:
        FALCOR_THROW("Unsupported device type '{}'.", fence->getDevice()->getType());
    }

    cudaExternalSemaphore_t extSem;
    FALCOR_CUDA_CHECK(cudaImportExternalSemaphore(&extSem, &desc));
    return extSem;
}
```

**Parameters**:
- fence: Falcor fence to import (must have shared=true)

**Returns**: CUDA external semaphore handle

**Behavior**:
- Validates fence is not null
- Validates fence has shared flag
- Gets shared API handle from fence
- Creates external semaphore descriptor based on device type:
  - D3D12 on Windows: cudaExternalSemaphoreHandleTypeD3D12Fence
  - Vulkan on Windows: cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32
  - Vulkan on Linux: cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd
- Imports external semaphore with CUDA

#### destroyExternalSemaphore()

```cpp
FALCOR_API void destroyExternalSemaphore(cudaExternalSemaphore_t extSem);
```

**Purpose**: Destroys CUDA external semaphore handle

**Implementation** ([`CudaUtils.cpp:178`](Source/Falcor/Utils/CudaUtils.cpp:178)):
```cpp
void destroyExternalSemaphore(cudaExternalSemaphore_t extSem)
{
    FALCOR_CUDA_CHECK(cudaDestroyExternalSemaphore(extSem));
}
```

**Parameters**:
- extSem: External semaphore handle to destroy

#### signalExternalSemaphore()

```cpp
FALCOR_API void signalExternalSemaphore(cudaExternalSemaphore_t extSem, uint64_t value, cudaStream_t stream = 0);
```

**Purpose**: Signals a CUDA external semaphore

**Implementation** ([`CudaUtils.cpp:183`](Source/Falcor/Utils/CudaUtils.cpp:183)):
```cpp
void signalExternalSemaphore(cudaExternalSemaphore_t extSem, uint64_t value, cudaStream_t stream)
{
    cudaExternalSemaphoreSignalParams params = {};
    params.params.fence.value = value;
    FALCOR_CUDA_CHECK(cudaSignalExternalSemaphoresAsync(&extSem, &params, 1, stream));
}
```

**Parameters**:
- extSem: External semaphore handle
- value: Signal value (typically incrementing counter)
- stream: CUDA stream to signal on (default: 0)

**Behavior**:
- Signals semaphore with specified value
- Signal occurs asynchronously on specified stream

#### waitExternalSemaphore()

```cpp
FALCOR_API void waitExternalSemaphore(cudaExternalSemaphore_t extSem, uint64_t value, cudaStream_t stream = 0);
```

**Purpose**: Waits on a CUDA external semaphore

**Implementation** ([`CudaUtils.cpp:190`](Source/Falcor/Utils/CudaUtils.cpp:190)):
```cpp
void waitExternalSemaphore(cudaExternalSemaphore_t extSem, uint64_t value, cudaStream_t stream)
{
    cudaExternalSemaphoreWaitParams params = {};
    params.params.fence.value = value;
    FALCOR_CUDA_CHECK(cudaWaitExternalSemaphoresAsync(&extSem, &params, 1, stream));
}
```

**Parameters**:
- extSem: External semaphore handle
- value: Value to wait for
- stream: CUDA stream to wait on (default: 0)

**Behavior**:
- Waits for semaphore to reach specified value
- Wait occurs asynchronously on specified stream

#### getSharedDevicePtr()

```cpp
FALCOR_API void* getSharedDevicePtr(Device::Type deviceType, SharedResourceApiHandle sharedHandle, uint32_t bytes);
```

**Purpose**: Gets CUDA device pointer for shared resource

**Implementation** ([`CudaUtils.cpp:197`](Source/Falcor/Utils/CudaUtils.cpp:197)):
```cpp
void* getSharedDevicePtr(Device::Type deviceType, SharedResourceApiHandle sharedHandle, uint32_t bytes)
{
    FALCOR_CHECK(sharedHandle, "Texture shared handle creation failed");

    cudaExternalMemoryHandleDesc externalMemoryDesc = {};
    switch (deviceType)
    {
#if FALCOR_WINDOWS
    case Device::Type::D3D12:
        externalMemoryDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
        externalMemoryDesc.handle.win32.handle = sharedHandle;
        break;
    case Device::Type::Vulkan:
        externalMemoryDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
        externalMemoryDesc.handle.win32.handle = sharedHandle;
        break;
#elif FALCOR_LINUX
    case Device::Type::Vulkan:
        externalMemoryDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
        externalMemoryDesc.handle.fd = (int)reinterpret_cast<intptr_t>(sharedHandle);
        break;
#endif
    default:
        FALCOR_THROW("Unsupported device type '{}'.", deviceType);
    }

    externalMemoryDesc.size = bytes;
    externalMemoryDesc.flags = cudaExternalMemoryDedicated;

    cudaExternalMemory_t externalMemory;
    FALCOR_CUDA_CHECK(cudaImportExternalMemory(&externalMemory, &externalMemoryDesc));

    cudaExternalMemoryBufferDesc bufDesc;
    memset(&bufDesc, 0, sizeof(bufDesc));
    bufDesc.size = bytes;

    void* devPtr = nullptr;
    FALCOR_CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(&devPtr, externalMemory, &bufDesc));

    return devPtr;
}
```

**Parameters**:
- deviceType: Type of graphics API device
- sharedHandle: Shared resource handle (e.g., from Resource::getSharedApiHandle())
- bytes: Size of resource in bytes

**Returns**: CUDA device pointer to shared resource

**Behavior**:
- Creates external memory descriptor based on device type
- Imports external memory
- Maps buffer from external memory
- Returns device pointer

**Note**: Caller is responsible for calling cudaFree() on returned pointer

#### freeSharedDevicePtr()

```cpp
FALCOR_API bool freeSharedDevicePtr(void* ptr);
```

**Purpose**: Frees shared device pointer

**Implementation** ([`CudaUtils.cpp:243`](Source/Falcor/Utils/CudaUtils.cpp:243)):
```cpp
bool freeSharedDevicePtr(void* ptr)
{
    if (!ptr)
        return false;
    return cudaSuccess == cudaFree(ptr);
}
```

**Parameters**:
- ptr: Device pointer to free

**Returns**: true if successful, false if ptr was null

#### importTextureToMipmappedArray()

```cpp
FALCOR_API cudaMipmappedArray_t importTextureToMipmappedArray(ref<Texture> pTex, uint32_t cudaUsageFlags);
```

**Purpose**: Imports a Falcor texture into a CUDA mipmapped array

**Implementation** ([`CudaUtils.cpp:250`](Source/Falcor/Utils/CudaUtils.cpp:250)):
```cpp
cudaMipmappedArray_t importTextureToMipmappedArray(ref<Texture> pTex, uint32_t cudaUsageFlags)
{
    SharedResourceApiHandle sharedHandle = pTex->getSharedApiHandle();
    FALCOR_CHECK(sharedHandle, "Texture shared handle creation failed");

    cudaExternalMemoryHandleDesc externalMemoryHandleDesc;
    memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));

    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
    externalMemoryHandleDesc.handle.win32.handle = sharedHandle;
    externalMemoryHandleDesc.size = pTex->getTextureSizeInBytes();
    externalMemoryHandleDesc.flags = cudaExternalMemoryDedicated;

    cudaExternalMemory_t externalMemory;
    FALCOR_CUDA_CHECK(cudaImportExternalMemory(&externalMemory, &externalMemoryHandleDesc));

    cudaExternalMemoryMipmappedArrayDesc mipDesc;
    memset(&mipDesc, 0, sizeof(mipDesc));
    auto format = pTex->getFormat();
    mipDesc.formatDesc.x = getNumChannelBits(format, 0);
    mipDesc.formatDesc.y = getNumChannelBits(format, 1);
    mipDesc.formatDesc.z = getNumChannelBits(format, 2);
    mipDesc.formatDesc.w = getNumChannelBits(format, 3);
    mipDesc.formatDesc.f = (getFormatType(format) == FormatType::Float) ? cudaChannelFormatKindFloat : cudaChannelFormatKindUnsigned;
    mipDesc.extent.depth = 1;
    mipDesc.extent.width = pTex->getWidth();
    mipDesc.extent.height = pTex->getHeight();
    mipDesc.flags = cudaUsageFlags;
    mipDesc.numLevels = 1;

    cudaMipmappedArray_t mipmappedArray;
    FALCOR_CUDA_CHECK(cudaExternalMemoryGetMappedMipmappedArray(&mipmappedArray, externalMemory, &mipDesc));
    return mipmappedArray;
}
```

**Parameters**:
- pTex: Falcor texture to import
- cudaUsageFlags: CUDA usage flags (e.g., cudaArraySurfaceLoadStore)

**Returns**: CUDA mipmapped array handle

**Behavior**:
- Gets shared handle from texture
- Imports texture as external memory
- Creates mipmapped array descriptor:
  - Sets channel bit depths from texture format
  - Sets format type (float or unsigned)
  - Sets texture dimensions
  - Sets usage flags
- Maps mipmapped array from external memory

**Note**: Should only be called once per texture resource

#### mapTextureToSurface()

```cpp
FALCOR_API cudaSurfaceObject_t mapTextureToSurface(ref<Texture> pTex, uint32_t usageFlags);
```

**Purpose**: Maps a texture to a CUDA surface object

**Implementation** ([`CudaUtils.cpp:286`](Source/Falcor/Utils/CudaUtils.cpp:286)):
```cpp
cudaSurfaceObject_t mapTextureToSurface(ref<Texture> pTex, uint32_t usageFlags)
{
    cudaMipmappedArray_t mipmap = importTextureToMipmappedArray(pTex, usageFlags);

    cudaArray_t cudaArray;
    FALCOR_CUDA_CHECK(cudaGetMipmappedArrayLevel(&cudaArray, mipmap, 0));

    cudaResourceDesc resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.res.array.array = cudaArray;
    resDesc.resType = cudaResourceTypeArray;

    cudaSurfaceObject_t surface;
    FALCOR_CUDA_CHECK(cudaCreateSurfaceObject(&surface, &resDesc));
    return surface;
}
```

**Parameters**:
- pTex: Falcor texture to map
- usageFlags: CUDA usage flags for mipmapped array

**Returns**: CUDA surface object handle

**Behavior**:
- Imports texture as mipmapped array
- Gets level 0 from mipmapped array
- Creates surface object from array
- Returns surface object

**Note**: Should only be called once per texture on initial load. Store returned surface object for repeated use.

### CudaDevice Class

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 131-148)

Wraps a CUDA device, context, and stream.

#### Class Definition

```cpp
class FALCOR_API CudaDevice : public Object
{
    FALCOR_OBJECT(cuda_utils::CudaDevice)
public:
    /// Constructor.
    /// Creates a CUDA device on the same adapter as the Falcor device.
    CudaDevice(const Device* pDevice);
    ~CudaDevice();

    CUdevice getDevice() const { return mCudaDevice; }
    CUcontext getContext() const { return mCudaContext; }
    CUstream getStream() const { return mCudaStream; }

private:
    CUdevice mCudaDevice;
    CUcontext mCudaContext;
    CUstream mCudaStream;
};
```

#### Constructor

**Implementation** ([`CudaUtils.cpp:335`](Source/Falcor/Utils/CudaUtils.cpp:335)):
```cpp
CudaDevice::CudaDevice(const Device* pDevice)
{
    FALCOR_CHECK(pDevice, "'pDevice' is nullptr.");
    FALCOR_CU_CHECK(cuInit(0));

    // Get a list of all available CUDA devices.
    int32_t deviceCount;
    FALCOR_CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
    std::vector<cudaDeviceProp> devices(deviceCount);
    for (int32_t i = 0; i < deviceCount; ++i)
        FALCOR_CUDA_CHECK(cudaGetDeviceProperties(&devices[i], i));

    // First we try to find the matching CUDA device by LUID.
    int32_t selectedDevice = findDeviceByLUID(devices, pDevice->getInfo().adapterLUID);
    if (selectedDevice < 0)
    {
        logWarning("Failed to find CUDA device by LUID. Falling back to device name.");
        // Next we try to find the matching CUDA device by name.
        selectedDevice = findDeviceByName(devices, pDevice->getInfo().adapterName);
        if (selectedDevice < 0)
        {
            logWarning("Failed to find CUDA device by name. Falling back to first compatible device.");
            // Finally we try to find the first compatible CUDA device.
            for (int32_t i = 0; i < devices.size(); ++i)
            {
                if (devices[i].major >= 7)
                {
                    selectedDevice = i;
                    break;
                }
            }
        }
    }

    if (selectedDevice < 0)
        FALCOR_THROW("No compatible CUDA device found.");

    FALCOR_CUDA_CHECK(cudaSetDevice(selectedDevice));
    FALCOR_CU_CHECK(cuDeviceGet(&mCudaDevice, selectedDevice));
    FALCOR_CU_CHECK(cuDevicePrimaryCtxRetain(&mCudaContext, mCudaDevice));
    FALCOR_CU_CHECK(cuStreamCreate(&mCudaStream, CU_STREAM_DEFAULT));

    const auto& props = devices[selectedDevice];
    logInfo("Created CUDA device '{}' (architecture {}.{}).", props.name, props.major, props.minor);
}
```

**Parameters**:
- pDevice: Falcor graphics device to match CUDA device with

**Behavior**:
- Initializes CUDA driver
- Enumerates all CUDA devices
- Attempts to find matching CUDA device using three strategies:
  1. By LUID (Locally Unique Identifier) - most reliable
  2. By adapter name - fallback
  3. By compute capability (major >= 7) - last resort
- Sets CUDA device
- Creates CUDA device handle
- Retains primary context
- Creates CUDA stream
- Logs device information

#### Destructor

**Implementation** ([`CudaUtils.cpp:381`](Source/Falcor/Utils/CudaUtils.cpp:381)):
```cpp
CudaDevice::~CudaDevice()
{
    FALCOR_CU_CHECK(cuStreamDestroy(mCudaStream));
    FALCOR_CU_CHECK(cuDevicePrimaryCtxRelease(mCudaDevice));
}
```

**Behavior**:
- Destroys CUDA stream
- Releases primary context

#### Accessor Methods

```cpp
CUdevice getDevice() const { return mCudaDevice; }
CUcontext getContext() const { return mCudaContext; }
CUstream getStream() const { return mCudaStream; }
```

**Returns**: CUDA device, context, and stream handles

### ExternalMemory Class

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 151-187)

Wraps an external memory resource (RAII wrapper).

#### Class Definition

```cpp
class ExternalMemory : public Object
{
    FALCOR_OBJECT(cuda_utils::ExternalMemory)
public:
    ExternalMemory(ref<Resource> pResource) : mpResource(pResource.get())
    {
        FALCOR_CHECK(mpResource, "'resource' is null.");
        if (auto pBuffer = pResource->asBuffer())
        {
            mExternalMemory = importExternalMemory(pBuffer.get());
            mSize = pBuffer->getSize();
        }
        else
        {
            FALCOR_THROW("'resource' must be a buffer.");
        }
    }

    ~ExternalMemory() { destroyExternalMemory(mExternalMemory); }

    size_t getSize() const { return mSize; }

    void* getMappedData() const
    {
        if (!mMappedData)
            mMappedData = externalMemoryGetMappedBuffer(mExternalMemory, 0, mSize);
        return mMappedData;
    }

private:
    /// Keep a non-owning pointer to the resource.
    /// TODO: If available, we should use a weak_ref here.
    Resource* mpResource;
    cudaExternalMemory_t mExternalMemory;
    size_t mSize;
    mutable void* mMappedData{nullptr};
};
```

#### Constructor

**Parameters**:
- pResource: Falcor resource (must be a buffer)

**Behavior**:
- Validates resource is not null
- Checks if resource is a buffer
- Imports buffer as external memory
- Stores buffer size
- Throws if resource is not a buffer

**Note**: Keeps non-owning pointer to resource (TODO: should use weak_ref)

#### Destructor

**Behavior**:
- Destroys external memory handle

#### Accessor Methods

```cpp
size_t getSize() const { return mSize; }
void* getMappedData() const
{
    if (!mMappedData)
        mMappedData = externalMemoryGetMappedBuffer(mExternalMemory, 0, mSize);
    return mMappedData;
}
```

**getSize()**: Returns size of external memory in bytes

**getMappedData()**: Lazily maps and returns CUDA device pointer to external memory. Mapping occurs only on first call.

### ExternalSemaphore Class

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 190-225)

Wraps an external semaphore for API synchronization.

#### Class Definition

```cpp
class ExternalSemaphore : public Object
{
    FALCOR_OBJECT(cuda_utils::ExternalSemaphore)
public:
    ExternalSemaphore(ref<Fence> pFence) : mpFence(pFence.get())
    {
        FALCOR_CHECK(mpFence, "'fence' is null.");
        FALCOR_CHECK(fence->getDesc().shared, "'fence' must be created with shared=true.");
        mExternalSemaphore = importExternalSemaphore(mpFence);
    }

    ~ExternalSemaphore() { destroyExternalSemaphore(mExternalSemaphore); }

    void signal(uint64_t value, cudaStream_t stream = 0) { signalExternalSemaphore(mExternalSemaphore, value, stream); }

    void wait(uint64_t value, cudaStream_t stream = 0) { waitExternalSemaphore(mExternalSemaphore, value, stream); }

    void waitForCuda(CopyContext* pCopyContext, cudaStream_t stream = 0, uint64_t value = Fence::kAuto)
    {
        uint64_t signalValue = mpFence->updateSignaledValue(value);
        signal(signalValue, stream);
        pCopyContext->wait(mpFence, signalValue);
    }

    void waitForFalcor(CopyContext* pCopyContext, cudaStream_t stream = 0, uint64_t value = Fence::kAuto)
    {
        uint64_t signalValue = pCopyContext->signal(mpFence, value);
        wait(signalValue, stream);
    }

private:
    /// Keep a non-owning pointer to fence.
    /// TODO: If available, we should use a weak_ref here.
    Fence* mpFence;
    cudaExternalSemaphore_t mExternalSemaphore;
};
```

#### Constructor

**Parameters**:
- pFence: Falcor fence (must have shared=true)

**Behavior**:
- Validates fence is not null
- Validates fence has shared flag
- Imports fence as external semaphore

**Note**: Keeps non-owning pointer to fence (TODO: should use weak_ref)

#### Destructor

**Behavior**:
- Destroys external semaphore handle

#### Signal/Wait Methods

```cpp
void signal(uint64_t value, cudaStream_t stream = 0) { signalExternalSemaphore(mExternalSemaphore, value, stream); }
void wait(uint64_t value, cudaStream_t stream = 0) { waitExternalSemaphore(mExternalSemaphore, value, stream); }
```

**signal()**: Signals semaphore with specified value on CUDA stream

**wait()**: Waits for semaphore to reach specified value on CUDA stream

#### waitForCuda()

```cpp
void waitForCuda(CopyContext* pCopyContext, cudaStream_t stream = 0, uint64_t value = Fence::kAuto)
{
    uint64_t signalValue = mpFence->updateSignaledValue(value);
    signal(signalValue, stream);
    pCopyContext->wait(mpFence, signalValue);
}
```

**Purpose**: Synchronizes Falcor to wait for CUDA

**Parameters**:
- pCopyContext: Falcor copy context
- stream: CUDA stream to signal on
- value: Signal value (default: Fence::kAuto)

**Behavior**:
- Updates fence signaled value
- Signals CUDA semaphore
- Waits on Falcor fence

#### waitForFalcor()

```cpp
void waitForFalcor(CopyContext* pCopyContext, cudaStream_t stream = 0, uint64_t value = Fence::kAuto)
{
    uint64_t signalValue = pCopyContext->signal(mpFence, value);
    wait(signalValue, stream);
}
```

**Purpose**: Synchronizes CUDA to wait for Falcor

**Parameters**:
- pCopyContext: Falcor copy context
- stream: CUDA stream to wait on
- value: Signal value (default: Fence::kAuto)

**Behavior**:
- Signals Falcor fence
- Waits on CUDA semaphore

### InteropBuffer Struct

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 232-245)

Encapsulates DirectX/Vulkan to CUDA interop data for a buffer.

#### Struct Definition

```cpp
struct InteropBuffer
{
    ref<Buffer> buffer;                     // Falcor buffer
    CUdeviceptr devicePtr = (CUdeviceptr)0; // CUDA pointer to buffer

    void free()
    {
        if (devicePtr)
        {
            cuda_utils::freeSharedDevicePtr((void*)devicePtr);
            devicePtr = (CUdeviceptr)0;
        }
    }
};
```

#### Members

**buffer**: Falcor buffer (owning reference)

**devicePtr**: CUDA device pointer to shared buffer

#### free() Method

**Behavior**:
- Frees CUDA device pointer if valid
- Resets devicePtr to 0

### createInteropBuffer() Function

**File**: `Source/Falcor/Utils/CudaUtils.h` (lines 247-259)

Creates an interop buffer for sharing between Falcor and CUDA.

```cpp
inline InteropBuffer createInteropBuffer(ref<Device> pDevice, size_t byteSize)
{
    InteropBuffer interop;

    // Create a new DX <-> CUDA shared buffer using Falcor API to create, then find its CUDA pointer.
    interop.buffer =
        pDevice->createBuffer(byteSize, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess | ResourceBindFlags::Shared);
    interop.devicePtr = (CUdeviceptr)cuda_utils::getSharedDevicePtr(pDevice->getType(), interop.buffer->getSharedApiHandle(), (uint32_t)interop.buffer->getSize());
    FALCOR_CHECK(interop.devicePtr != (CUdeviceptr)0, "Failed to create CUDA device ptr for buffer");

    return interop;
}
```

**Parameters**:
- pDevice: Falcor device
- byteSize: Size of buffer in bytes

**Returns**: InteropBuffer struct with Falcor buffer and CUDA pointer

**Behavior**:
- Creates Falcor buffer with Shared bind flag
- Gets CUDA device pointer for shared buffer
- Validates device pointer is valid
- Returns interop buffer

## Technical Details

### Device Matching Algorithm

The [`CudaDevice`](Source/Falcor/Utils/CudaUtils.h:131) constructor uses a three-stage device matching algorithm:

**Stage 1: LUID Matching** ([`findDeviceByLUID()`](Source/Falcor/Utils/CudaUtils.cpp:306))
- Compares 8-byte LUID (Locally Unique Identifier)
- Most reliable method
- Same LUID for D3D12, Vulkan, and CUDA on same GPU
- Windows: Compares cudaDeviceProp::luid
- Linux: Compares cudaDeviceProp::uuid (LUID not supported)

**Stage 2: Name Matching** ([`findDeviceByName()`](Source/Falcor/Utils/CudaUtils.cpp:327))
- Compares adapter names as string
- Fallback if LUID matching fails
- Less reliable (names can differ)

**Stage 3: Compute Capability**
- Selects first device with compute capability >= 7
- Last resort fallback
- Ensures minimum feature level

### External Memory Import

External memory import follows this process:

```
1. Get shared handle from Falcor resource
2. Create external memory descriptor:
   - Set type based on device (D3D12/Vulkan)
   - Set platform-specific handle (Windows handle/Linux fd)
   - Set size
   - Set flags (dedicated)
3. Import external memory with CUDA
4. Map buffer from external memory
5. Return device pointer
```

### External Semaphore Import

External semaphore import follows this process:

```
1. Get shared handle from Falcor fence
2. Create external semaphore descriptor:
   - Set type based on device (D3D12/Vulkan)
   - Set platform-specific handle
3. Import external semaphore with CUDA
4. Return semaphore handle
```

### Texture to Surface Mapping

Texture to surface mapping involves:

```
1. Import texture as external memory
2. Create mipmapped array descriptor:
   - Set channel bit depths from format
   - Set format type (float/unsigned)
   - Set dimensions (width, height, depth=1)
   - Set usage flags
   - Set mip levels (1)
3. Map mipmapped array from external memory
4. Get level 0 from mipmapped array
5. Create surface object from array
6. Return surface object
```

### Platform-Specific Handle Types

**Windows**:
- D3D12: cudaExternalMemoryHandleTypeD3D12Resource
- Vulkan: cudaExternalMemoryHandleTypeOpaqueWin32
- D3D12 Fence: cudaExternalSemaphoreHandleTypeD3D12Fence
- Vulkan Semaphore: cudaExternalSemaphoreHandleTypeTimelineSemaphoreWin32

**Linux**:
- Vulkan: cudaExternalMemoryHandleTypeOpaqueFd
- Vulkan Semaphore: cudaExternalSemaphoreHandleTypeTimelineSemaphoreFd

### Synchronization Patterns

**Falcor to CUDA** ([`waitForCuda()`](Source/Falcor/Utils/CudaUtils.h:207)):
```
1. Update Falcor fence signaled value
2. Signal CUDA semaphore
3. Wait on Falcor fence
```

**CUDA to Falcor** ([`waitForFalcor()`](Source/Falcor/Utils/CudaUtils.h:214)):
```
1. Signal Falcor fence
2. Wait on CUDA semaphore
```

## Integration Points

### Falcor Core Integration

- **Object**: Base class for CudaDevice, ExternalMemory, ExternalSemaphore
- **Device**: Graphics device for CUDA device matching
- **Buffer**: Buffer resource for external memory import
- **Texture**: Texture resource for surface mapping
- **Fence**: Fence resource for external semaphore import
- **CopyContext**: Copy context for synchronization

### CUDA Integration

- **CUDA Runtime API**: cudaMalloc, cudaFree, cudaMemcpy, cudaMemset
- **CUDA Driver API**: cuInit, cuDeviceGet, cuDevicePrimaryCtxRetain
- **CUDA Interop API**: cudaImportExternalMemory, cudaImportExternalSemaphore
- **CUDA External Memory**: cudaExternalMemoryGetMappedBuffer
- **CUDA External Semaphore**: cudaSignalExternalSemaphoresAsync, cudaWaitExternalSemaphoresAsync

### Internal Falcor Usage

- **DiffRendering**: PyTorch/CUDA integration for inverse rendering
- **Rendering**: CUDA-accelerated rendering passes
- **Scene**: CUDA-based scene processing and optimization

## Architecture Patterns

### RAII Pattern

Classes use RAII for resource management:

```cpp
class ExternalMemory : public Object
{
    ~ExternalMemory() { destroyExternalMemory(mExternalMemory); }
};
```

Benefits:
- Automatic resource cleanup
- Exception safety
- Clear ownership semantics

### Wrapper Pattern

Classes wrap CUDA handles:

```cpp
class CudaDevice : public Object
{
    CUdevice mCudaDevice;
    CUcontext mCudaContext;
    CUstream mCudaStream;
};
```

Benefits:
- Type safety
- Encapsulation
- Convenience methods

### Macro-Based Error Checking

Macros provide consistent error handling:

```cpp
#define FALCOR_CUDA_CHECK(call) \
    { \
        cudaError_t result = call; \
        if (result != cudaSuccess) \
        { \
            FALCOR_THROW("CUDA call {} failed with error {} ({}).", #call, errorName, errorString); \
        } \
    }
```

Benefits:
- Consistent error handling
- Descriptive error messages
- Minimal code duplication

### Lazy Initialization

ExternalMemory uses lazy mapping:

```cpp
void* getMappedData() const
{
    if (!mMappedData)
        mMappedData = externalMemoryGetMappedBuffer(mExternalMemory, 0, mSize);
    return mMappedData;
}
```

Benefits:
- Delay resource allocation until needed
- Avoid unnecessary work
- Improve startup time

### Fallback Pattern

Device matching uses fallback strategy:

```cpp
int32_t selectedDevice = findDeviceByLUID(devices, pDevice->getInfo().adapterLUID);
if (selectedDevice < 0)
{
    selectedDevice = findDeviceByName(devices, pDevice->getInfo().adapterName);
    if (selectedDevice < 0)
    {
        // Find first compatible device
    }
}
```

Benefits:
- Graceful degradation
- Multiple strategies for device matching
- Improved compatibility

## Code Patterns

### Platform-Specific Code

Uses preprocessor directives for platform support:

```cpp
#if FALCOR_WINDOWS
    case Device::Type::D3D12:
        desc.type = cudaExternalMemoryHandleTypeD3D12Resource;
        desc.handle.win32.handle = sharedHandle;
        break;
#elif FALCOR_LINUX
    case Device::Type::Vulkan:
        desc.type = cudaExternalMemoryHandleTypeOpaqueFd;
        desc.handle.fd = (int)reinterpret_cast<intptr_t>(sharedHandle);
        break;
#endif
```

### Descriptor Initialization

Consistent descriptor initialization pattern:

```cpp
cudaExternalMemoryHandleDesc desc = {};
desc.size = buffer->getSize();
desc.flags = cudaExternalMemoryDedicated;
```

### Null-Safe Operations

Functions check for null pointers:

```cpp
void freeDevice(void* devPtr)
{
    if (!devPtr)
        return;
    FALCOR_CUDA_CHECK(cudaFree(devPtr));
}
```

### Ref-Based Construction

Classes use Falcor's ref system:

```cpp
ExternalMemory(ref<Resource> pResource) : mpResource(pResource.get())
{
    FALCOR_CHECK(mpResource, "'resource' is null.");
    // ...
}
```

### Validation Pattern

Functions validate inputs before processing:

```cpp
FALCOR_CHECK(buffer, "'buffer' is nullptr.");
FALCOR_CHECK(is_set(buffer->getBindFlags(), ResourceBindFlags::Shared), "Buffer must be created with ResourceBindFlags::Shared.");
```

## Use Cases

### Basic CUDA Memory Operations

```cpp
// Allocate device memory
void* devPtr = cuda_utils::mallocDevice(size);

// Copy data to device
cuda_utils::memcpyHostToDevice(devPtr, hostPtr, size);

// Copy data from device
cuda_utils::memcpyDeviceToHost(hostPtr, devPtr, size);

// Free device memory
cuda_utils::freeDevice(devPtr);
```

### Creating CUDA Device

```cpp
// Create CUDA device matching Falcor device
cuda_utils::CudaDevice cudaDevice(pFalcorDevice);

// Get CUDA handles
CUdevice device = cudaDevice.getDevice();
CUcontext context = cudaDevice.getContext();
CUstream stream = cudaDevice.getStream();
```

### Sharing Buffer with CUDA

```cpp
// Create shared buffer
ref<Buffer> buffer = pDevice->createBuffer(size, ResourceBindFlags::Shared);

// Import as external memory
cudaExternalMemory_t extMem = cuda_utils::importExternalMemory(buffer.get());

// Map to CUDA pointer
void* devPtr = cuda_utils::externalMemoryGetMappedBuffer(extMem, 0, size);

// Use in CUDA kernel
// ...

// Cleanup
cuda_utils::destroyExternalMemory(extMem);
```

### Using ExternalMemory Wrapper

```cpp
// Create external memory wrapper
cuda_utils::ExternalMemory extMem(buffer);

// Get mapped data (lazy)
void* devPtr = extMem.getMappedData();

// Use in CUDA
// ...

// Automatic cleanup on destruction
```

### Synchronizing Between APIs

```cpp
// Create shared fence
ref<Fence> fence = pDevice->createFence(true);

// Import as external semaphore
cuda_utils::ExternalSemaphore extSem(fence);

// Synchronize Falcor to wait for CUDA
extSem.waitForCuda(pCopyContext, cudaStream);

// Synchronize CUDA to wait for Falcor
extSem.waitForFalcor(pCopyContext, cudaStream);
```

### Mapping Texture to Surface

```cpp
// Import texture as mipmapped array
cudaMipmappedArray_t mipmap = cuda_utils::importTextureToMipmappedArray(texture, cudaArraySurfaceLoadStore);

// Map to surface
cudaSurfaceObject_t surface = cuda_utils::mapTextureToSurface(texture, cudaArraySurfaceLoadStore);

// Use surface in CUDA kernel
// ...

// Cleanup
cudaDestroySurfaceObject(surface);
```

### Creating Interop Buffer

```cpp
// Create interop buffer
InteropBuffer interop = createInteropBuffer(pDevice, byteSize);

// Use Falcor buffer
// interop.buffer->...

// Use CUDA pointer
// interop.devicePtr...

// Cleanup
interop.free();
```

## Performance Considerations

### Memory Transfer Overhead

- **Host to Device**: PCIe bandwidth limited
- **Device to Host**: PCIe bandwidth limited
- **Device to Device**: GPU memory bandwidth (much faster)
- **External Memory**: Zero-copy when possible

### Synchronization Overhead

- **Semaphore Operations**: Low overhead
- **Fence Operations**: Moderate overhead
- **Stream Synchronization**: High overhead (blocks entire stream)
- **Device Synchronization**: Very high overhead (blocks entire device)

### Lazy Initialization Benefits

- **External Memory Mapping**: Delayed until first use
- **Surface Object Creation**: Delayed until first use
- **Device Matching**: Only performed once per device

### Resource Sharing Benefits

- **Zero-Copy**: External memory avoids copies
- **Shared Memory**: Single allocation for both APIs
- **Pinned Memory**: Faster host-device transfers

### Optimization Tips

1. **Minimize Transfers**: Keep data on GPU when possible
2. **Use External Memory**: Share resources instead of copying
3. **Batch Operations**: Combine multiple operations
4. **Asynchronous Operations**: Use streams for overlap
5. **Pinned Memory**: Use cudaHostAlloc for frequent transfers

## Limitations

### Platform Limitations

- **Windows Only**: D3D12 interop only on Windows
- **NVIDIA Only**: Only works with NVIDIA GPUs
- **CUDA Required**: Requires CUDA toolkit installed
- **Driver Support**: Requires compatible GPU drivers

### Device Limitations

- **Compute Capability**: Requires compute capability >= 7.0
- **Memory Limits**: Limited by GPU memory
- **Concurrent Kernels**: Limited by GPU resources
- **Stream Limits**: Limited number of concurrent streams

### API Limitations

- **D3D12 Only**: Only supports D3D12 on Windows
- **Vulkan Support**: Limited Vulkan support
- **No OpenGL**: No OpenGL interop
- **No DX11**: No DirectX 11 interop

### Resource Limitations

- **Buffers Only**: ExternalMemory only supports buffers
- **Single Mip Level**: Texture import only supports single mip level
- **No 3D Textures**: No 3D texture support
- **No Compressed Formats**: Limited format support

### Synchronization Limitations

- **Timeline Semaphores**: Requires timeline semaphore support
- **Shared Resources**: Requires shared resource support
- **Fence Limitations**: Limited fence functionality
- **Stream Ordering**: Complex stream ordering

## Best Practices

### Resource Management

- **Use RAII**: Use wrapper classes (ExternalMemory, ExternalSemaphore)
- **Automatic Cleanup**: Rely on destructors for cleanup
- **Validate Resources**: Check for null resources
- **Shared Flags**: Always use ResourceBindFlags::Shared for shared resources

### Memory Management

- **Minimize Transfers**: Keep data on GPU
- **Use External Memory**: Share resources instead of copying
- **Free Promptly**: Free CUDA memory when done
- **Check for Null**: Always check for null pointers

### Synchronization

- **Use Semaphores**: Prefer semaphores over device sync
- **Signal/Wait Pairs**: Always pair signal and wait operations
- **Avoid Blocking**: Use asynchronous operations
- **Stream Ordering**: Use streams for ordering

### Error Handling

- **Use Check Macros**: Always use FALCOR_CUDA_CHECK and FALCOR_CU_CHECK
- **Validate Inputs**: Check for null pointers and invalid parameters
- **Handle Failures**: Handle CUDA errors gracefully
- **Log Errors**: Log descriptive error messages

### Performance

- **Batch Operations**: Combine multiple operations
- **Use Streams**: Use streams for overlap
- **Pinned Memory**: Use cudaHostAlloc for frequent transfers
- **Profile**: Profile CUDA operations

### Platform Considerations

- **Windows**: Use D3D12 for best performance
- **Linux**: Use Vulkan for cross-platform support
- **LUID Matching**: Prefer LUID matching on Windows
- **UUID Matching**: Use UUID matching on Linux

### Device Matching

- **LUID First**: Always try LUID matching first
- **Name Fallback**: Use name matching as fallback
- **Compute Capability**: Use compute capability as last resort
- **Validate**: Always validate device selection

## Implementation Notes

### Handle Types

**Windows Handles**:
- SharedResourceApiHandle: Windows HANDLE (void*)
- SharedFenceApiHandle: Windows HANDLE (void*)
- Cast to win32.handle for CUDA

**Linux Handles**:
- SharedResourceApiHandle: File descriptor (int)
- SharedFenceApiHandle: File descriptor (int)
- Cast to fd for CUDA

### Memory Layout

**External Memory**:
- Imported from Falcor buffer
- Mapped to CUDA device pointer
- Same physical memory as Falcor buffer

**Mipmapped Array**:
- Imported from Falcor texture
- Mapped to CUDA array
- Same physical memory as Falcor texture

### Thread Safety

- **Not Thread-Safe**: Most functions are not thread-safe
- **Per-Thread Device**: Each thread should use its own CUDA device
- **Stream Ordering**: Use streams for thread-safe ordering
- **External Resources**: External resources can be shared across threads

### Exception Safety

- **Strong Guarantee**: Most functions provide strong exception safety
- **RAII Cleanup**: Resources cleaned up on exception
- **Check Macros**: Macros throw on error
- **No Leaks**: No resource leaks on exception

### Reference Counting

- **Non-Owning Pointers**: ExternalMemory and ExternalSemaphore keep non-owning pointers
- **TODO**: Should use weak_ref when available
- **Lifetime**: Caller must ensure resources outlive wrappers

### Device Context

- **Primary Context**: Uses device primary context
- **Retained**: Context is retained (reference counted)
- **Released**: Released in destructor
- **Shared**: Context can be shared across multiple CudaDevice instances

## Future Enhancements

### Potential Improvements

1. **More Resource Types**: Support for textures, 3D textures, compressed formats
2. **OpenGL Interop**: Add OpenGL interop support
3. **DX11 Support**: Add DirectX 11 interop
4. **More Formats**: Support for more texture formats
5. **Mip Chain Support**: Support for full mip chains
6. **Weak References**: Use weak_ref for resource references

### API Extensions

1. **CUDA Graph**: Add CUDA graph support
2. **CUDA Streams**: Add more stream management
2. **CUDA Events**: Add CUDA event support
3. **CUDA Arrays**: Add CUDA array support
4. **CUDA Textures**: Add CUDA texture support

### Performance Enhancements

1. **Async Copy**: Add asynchronous copy operations
2. **Pinned Memory**: Add pinned memory allocation
3. **Unified Memory**: Add unified memory support
4. **Peer Access**: Add peer-to-peer access
5. **Multi-GPU**: Add multi-GPU support

### Platform Enhancements

1. **More Platforms**: Add support for more platforms
2. **Better Detection**: Improve device detection
3. **Fallback Strategies**: Better fallback strategies
4. **Error Recovery**: Better error recovery

### Documentation Enhancements

1. **More Examples**: Add more usage examples
2. **Performance Guide**: Add performance optimization guide
3. **Troubleshooting**: Add troubleshooting guide
4. **Best Practices**: Add best practices guide

## References

### CUDA Documentation

- **CUDA Runtime API**: Official CUDA runtime documentation
- **CUDA Driver API**: Official CUDA driver documentation
- **CUDA Interop API**: CUDA interoperability documentation
- **CUDA Programming Guide**: CUDA programming guide

### Falcor Documentation

- **Falcor API**: Falcor API documentation
- **Falcor Architecture**: Falcor architecture documentation
- **Falcor Resource Management**: Falcor resource management documentation

### Graphics API Documentation

- **D3D12 Documentation**: DirectX 12 documentation
- **Vulkan Documentation**: Vulkan documentation
- **D3D12 Interop**: D3D12 CUDA interop documentation
- **Vulkan Interop**: Vulkan CUDA interop documentation

### Related Technologies

- **CUDA**: NVIDIA's parallel computing platform
- **OptiX**: NVIDIA's ray tracing engine
- **PyTorch**: Deep learning framework with CUDA support
- **TensorRT**: NVIDIA's inference optimizer
