# GridConverter - NanoVDB to BrickedGrid Converter

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Scene/Volume/BrickedGrid.h (BrickedGrid structure)
- Falcor/Scene/Volume/BC4Encode.h (BC4 compression)
- Falcor/Core/API/Device.h (Device for texture creation)
- Falcor/Core/API/Formats.h (ResourceFormat enums)
- Falcor/Utils/Logger.h (Logging)
- Falcor/Utils/HostDeviceShared.slangh (Shared utilities)
- Falcor/Utils/NumericRange.h (NumericRange for parallel execution)
- Falcor/Utils/Math/Vector.h (Vector types)
- Falcor/Utils/Timing/CpuTimer.h (CPU timing)
- NanoVDB (nanovdb/NanoVDB.h)
- STL (algorithm, atomic, execution, vector)

## Module Overview

GridConverter is a template-based converter that transforms NanoVDB sparse volume grids into BrickedGrid format for efficient GPU rendering. It supports multiple texel formats (BC4, UNORM8, UNORM16), parallel processing, automatic mipmap generation, and BC4 compression for reduced memory footprint.

## Component Specifications

### NanoVDBToBricksConverter Template

**Signature**: `template <typename TexelType, unsigned int kBitsPerTexel> struct NanoVDBToBricksConverter`

**Purpose**: Converts NanoVDB FloatGrid to BrickedGrid format

**Template Parameters**:
- `TexelType`: Output texel type (uint64_t for BC4, uint8_t for UNORM8, uint16_t for UNORM16)
- `kBitsPerTexel`: Bits per texel (4 for BC4, 8 for UNORM8, 16 for UNORM16)

**Type Aliases**:
- `NanoVDBConverterBC4`: BC4 converter (4-bit, 50% compression)
- `NanoVDBConverterUNORM8`: UNORM8 converter (8-bit, no compression)
- `NanoVDBConverterUNORM16`: UNORM16 converter (16-bit, no compression)

**Public Methods**:
- `NanoVDBToBricksConverter(const nanovdb::FloatGrid* grid)`: Constructor
- `BrickedGrid convert(ref<Device> pDevice)`: Perform conversion and create GPU textures

**Private Methods**:
- `convertSlice(int z)`: Convert a single Z-slice of bricks
- `computeMip(int mip)`: Compute a single mipmap level

**Helper Methods**:
- `getAtlasSizeBricks()`: Get atlas size in bricks
- `getAtlasSizePixels()`: Get atlas size in pixels
- `getAtlasMaxBrick()`: Get maximum number of bricks in atlas
- `getAtlasFormat()`: Get GPU texture format based on bit depth
- `combineMajMin(float2 a, float2 b)`: Combine two majorant/minorant pairs
- `unpackMajMin(const uint32_t* data)`: Unpack majorant/minorant from packed format
- `expandMinorantMajorant(float value, float& min_inout, float& maj_inout)`: Expand minorant/majorant range

**Static Constants**:
- `kBrickSize = 8`: Brick size in voxels (must match NanoVDB leaf size)
- `kBC4Compress`: Boolean indicating whether to use BC4 compression

**Member Variables**:
- `const nanovdb::FloatGrid* mpFloatGrid`: Input NanoVDB grid
- `uint3 mAtlasSizeBricks`: Atlas dimensions in bricks
- `int3 mLeafDim[4]`: Leaf dimensions for each mipmap level (0-3)
- `int3 mBBMin, mBBMax, mPixDim`: Bounding box and pixel dimensions
- `uint32_t mLeafCount[4]`: Cumulative leaf count for each mipmap level
- `std::vector<uint32_t> mRangeData`: Range data (majorant/minorant) for all mipmap levels
- `std::vector<uint32_t> mPtrData`: Indirection data (brick pointers)
- `std::vector<TexelType> mAtlasData`: Atlas data (compressed or uncompressed)
- `std::atomic_uint32_t mNonEmptyCount`: Atomic counter for non-empty bricks

## Technical Details

### Conversion Pipeline

The conversion process consists of three main stages:

1. **Slice Conversion** (Parallel): Convert each Z-slice of bricks
2. **Mipmap Generation** (Sequential): Compute 4 mipmap levels for range texture
3. **Texture Creation** (Sequential): Create GPU textures from converted data

```
NanoVDB Grid
    ↓
[Parallel] Slice Conversion
    ↓
[Sequential] Mipmap Generation
    ↓
[Sequential] Texture Creation
    ↓
BrickedGrid (GPU)
```

### Slice Conversion (convertSlice)

**Purpose**: Convert a single Z-slice of bricks from NanoVDB to BrickedGrid format

**Algorithm**:
1. Iterate over all bricks in the Z-slice
2. For each brick:
   - Compute brick coordinates in NanoVDB index space
   - Probe NanoVDB leaf node containing the brick
   - Extract 8x8x8 voxel data from leaf
   - Compute minorant (minimum) and majorant (maximum) values
   - Include 1-voxel halo from neighboring bricks
   - Allocate brick in atlas (atomic)
   - Store range data (majorant/minorant)
   - Store indirection data (atlas brick index)
   - Compress and store atlas data (BC4 or uncompressed)

**Halo Support**:
- 1-voxel halo around each brick for smooth trilinear interpolation
- Fetches halo voxels from neighboring bricks
- Ensures continuity at brick boundaries

**Atomic Allocation**:
- Uses `mNonEmptyCount.fetch_add(1)` for thread-safe brick allocation
- Each non-empty brick gets a unique index
- Empty bricks are not allocated in atlas

**BC4 Compression**:
- Divides 8x8x8 brick into 4x4 tiles
- Compresses each 4x4 tile using BC4 format
- Stores compressed data in atlas
- 50% memory reduction (8 bytes per 4x4 tile vs 16 bytes uncompressed)

### Mipmap Generation (computeMip)

**Purpose**: Compute hierarchical range data for early ray termination

**Algorithm**:
1. For each mipmap level (1-3):
   - Read range data from previous level (mip-1)
   - Combine 2x2x2 blocks of ranges
   - Compute new majorant (max of max values) and minorant (min of min values)
   - Store in current mipmap level

**Range Combination**:
```cpp
float2 combineMajMin(float2 a, float2 b)
{
    return float2(std::max(a.x, b.x), std::min(a.y, b.y));
}
```

**Mipmap Levels**:
- Level 0: Per-brick range (8x8x8 voxels)
- Level 1: Per-2x2x2-brick range (16x16x16 voxels)
- Level 2: Per-4x4x4-brick range (32x32x32 voxels)
- Level 3: Per-8x8x8-brick range (64x64x64 voxels)

**Early Ray Termination**:
- Coarser mipmap levels provide conservative bounds
- Enables early termination when ray is outside all ranges
- Reduces ray marching steps for empty regions

### Texture Creation (convert)

**Purpose**: Create GPU textures from converted data

**Algorithm**:
1. Convert all slices in parallel using `std::for_each(std::execution::par, ...)`
2. Compute all 4 mipmap levels sequentially
3. Create range texture with 4 mipmap levels (RG16Float format)
4. Create indirection texture (RGBA8Uint format)
5. Create atlas texture (BC4Unorm, R8Unorm, or R16Unorm format)

**Parallel Execution**:
- Uses C++17 parallel algorithms
- Each Z-slice processed independently
- Atomic counter ensures thread-safe brick allocation
- Significant speedup for large grids

**Texture Formats**:
- Range: RG16Float (16-bit float for majorant/minorant)
- Indirection: RGBA8Uint (32-bit packed brick index)
- Atlas: BC4Unorm (4-bit compressed), R8Unorm (8-bit), or R16Unorm (16-bit)

### Memory Layout

**Range Data**:
```
mRangeData Layout:
├── Level 0: mLeafCount[0] entries (per-brick range)
├── Level 1: mLeafCount[1] - mLeafCount[0] entries (2x2x2-brick range)
├── Level 2: mLeafCount[2] - mLeafCount[1] entries (4x4x4-brick range)
└── Level 3: mLeafCount[3] - mLeafCount[2] entries (8x8x8-brick range)

Each entry: uint32_t packed as:
- Bits 0-15: majorant (f16)
- Bits 16-31: minorant (f16)
```

**Indirection Data**:
```
mPtrData Layout:
└── Level 0: mLeafCount[0] entries (per-brick pointer)

Each entry: uint32_t packed as:
- Bits 0-7: atlas X brick index
- Bits 8-15: atlas Y brick index
- Bits 16-23: atlas Z brick index
- Bits 24-31: reserved (unused)
```

**Atlas Data**:
```
mAtlasData Layout:
└── Linear layout of bricks in atlas

BC4 Compressed:
- 8x8x8 brick = 8 * (8/4 * 8/4) = 32 BC4 blocks
- Each BC4 block: 8 bytes
- Total: 256 bytes per brick

Uncompressed:
- 8x8x8 brick = 512 voxels
- Each voxel: 1 byte (UNORM8) or 2 bytes (UNORM16)
- Total: 512 bytes (UNORM8) or 1024 bytes (UNORM16) per brick
```

### Atlas Layout

**Brick Allocation**:
- Linear allocation using atomic counter
- Bricks packed tightly in X-Y-Z order
- First two dimensions are powers of 2 for optimal texture layout
- Third dimension adjusted to fit all bricks

**Atlas Size Calculation**:
```cpp
uint leafCount = grid->tree().nodeCount(0);
uint approxdim = 1u << uint(log2f((float)leafCount + 1.f) / 3.f);
uint lastdim = (leafCount + approxdim * approxdim - 1) / (approxdim * approxdim);
mAtlasSizeBricks = uint3(approxdim, approxdim, lastdim);
```

**Pixel Coordinates**:
```cpp
uint32_t atlasx = myleaf % mAtlasSizeBricks.x;
uint32_t atlasy = (myleaf / mAtlasSizeBricks.x) % mAtlasSizeBricks.y;
uint32_t atlasz = myleaf / bricksPerSlice;
```

### Value Normalization

**Purpose**: Normalize voxel values to texel range for GPU texture storage

**Algorithm**:
```cpp
float invRange = ((1 << kBitsPerTexel) - 1.f) / (majorant - minorant);
TexelType texel = TexelType((value - minorant) * invRange);
```

**Denormalization** (in shader):
```cpp
float value = texel * (majorant - minorant) + minorant;
```

**BC4 Special Case**:
- BC4 uses 8-bit values (0-255)
- Normalization: `uint8_t((value - minorant) * (255.f / (majorant - minorant)))`
- Denormalization: `float(texel) / 255.f * (majorant - minorant) + minorant`

### Halo Fetching

**Purpose**: Fetch 1-voxel halo around each brick for smooth interpolation

**Algorithm**:
- Fetch central 8x8x8 brick from leaf node
- Fetch halo voxels from 6 faces (±X, ±Y, ±Z)
- Fetch halo voxels from 12 edges
- Fetch halo voxels from 8 corners
- Total: 8³ + 6*8² + 12*8 + 8 = 1000 voxels

**Optimization**:
- Fetch order maximizes NanoVDB internal cache reuse
- Sequential access patterns for better memory locality
- Minimizes tree traversal overhead

## Integration Points

### Usage in Volume Module

The GridConverter is used by the Grid class to convert NanoVDB grids to BrickedGrid format:

```cpp
Grid::Grid(ref<Device> pDevice, nanovdb::GridHandle<nanovdb::HostBuffer> gridHandle)
    : mpDevice(pDevice)
    , mGridHandle(std::move(gridHandle))
    , mpFloatGrid(mGridHandle.grid<float>())
    , mAccessor(mpFloatGrid->getAccessor())
{
    // Compute grid statistics
    if (!mpFloatGrid->hasMinMax())
    {
        nanovdb::gridStats(*mpFloatGrid);
    }

    // Upload NanoVDB to GPU
    mpBuffer = mpDevice->createStructuredBuffer(
        sizeof(uint32_t),
        uint32_t(div_round_up(mGridHandle.size(), sizeof(uint32_t))),
        ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mGridHandle.data()
    );

    // Convert to BrickedGrid
    using NanoVDBGridConverter = NanoVDBConverterBC4;
    mBrickedGrid = NanoVDBGridConverter(mpFloatGrid).convert(mpDevice);
}
```

### Integration Pattern

```cpp
// Create converter
NanoVDBConverterBC4 converter(grid);

// Convert to BrickedGrid
BrickedGrid bricks = converter.convert(device);

// Use BrickedGrid for GPU rendering
shaderData.brickedGrid = bricks;
```

## Architecture Patterns

### Template Pattern

Compile-time specialization for different texel types:
- `NanoVDBConverterBC4`: 4-bit BC4 compressed
- `NanoVDBConverterUNORM8`: 8-bit uncompressed
- `NanoVDBConverterUNORM16`: 16-bit uncompressed

### Parallel Processing Pattern

Multi-threaded conversion using C++17 parallel algorithms:
- `std::for_each(std::execution::par, ...)`: Parallel slice conversion
- `std::atomic_uint32_t`: Thread-safe brick allocation
- Independent slice processing for maximum parallelism

### Strategy Pattern

Multiple compression strategies:
- BC4 compression: 50% memory reduction, lossy
- UNORM8: No compression, 8-bit precision
- UNORM16: No compression, 16-bit precision

### Builder Pattern

Incremental construction of BrickedGrid:
1. Convert slices (parallel)
2. Compute mipmaps (sequential)
3. Create textures (sequential)
4. Return complete BrickedGrid

### Visitor Pattern

Iterate over NanoVDB tree structure:
- Probe leaf nodes for brick data
- Access voxel data efficiently
- Maximize cache reuse

## Code Patterns

### Constructor Pattern

```cpp
NanoVDBToBricksConverter(const nanovdb::FloatGrid* grid)
{
    mNonEmptyCount.store(0);
    mpFloatGrid = grid;

    // Compute bounding box (aligned to 8-voxel brick boundaries)
    auto& voxelbox = mpFloatGrid->indexBBox();
    mBBMin = (int3(voxelbox.min().x(), voxelbox.min().y(), voxelbox.min().z())) & (~7);
    mBBMax = (int3(voxelbox.max().x(), voxelbox.max().y(), voxelbox.max().z()) + 7) & (~7);
    mPixDim = mBBMax - mBBMin;

    // Pad to 64-voxel boundary for 4 mipmap levels
    mPixDim = (mPixDim + 63) & ~63;

    // Compute leaf dimensions for each mipmap level
    for (uint i = 0; i < 4; ++i)
    {
        mLeafDim[i] = mPixDim / (8 << i);
        mLeafCount[i] = (mLeafDim[i].x * mLeafDim[i].y * mLeafDim[i].z) +
                       (i ? mLeafCount[i - 1] : 0);
    }

    // Compute atlas size
    uint leafCount = grid->tree().nodeCount(0);
    uint approxdim = 1u << uint(log2f((float)leafCount + 1.f) / 3.f);
    uint lastdim = (leafCount + approxdim * approxdim - 1) / (approxdim * approxdim);
    mAtlasSizeBricks = uint3(approxdim, approxdim, lastdim);

    // Allocate buffers
    uint3 atlasSizePixels = getAtlasSizePixels();
    uint leafTexelCount = atlasSizePixels.x * atlasSizePixels.y * atlasSizePixels.z;
    mRangeData.resize(mLeafCount[3]);
    mPtrData.resize(mLeafCount[0]);
    mAtlasData.resize(kBC4Compress ? (leafTexelCount / 16) : leafTexelCount);
}
```

### Convert Pattern

```cpp
BrickedGrid convert(ref<Device> pDevice)
{
    auto t0 = CpuTimer::getCurrentTimePoint();

    // Parallel slice conversion
    auto range = NumericRange<int>(0, mLeafDim[0].z);
    std::for_each(std::execution::par, range.begin(), range.end(),
                  [&](int z) { convertSlice(z); });

    // Sequential mipmap generation
    for (int mip = 1; mip < 4; ++mip)
        computeMip(mip);

    // Create GPU textures
    BrickedGrid bricks;
    bricks.range = pDevice->createTexture3D(
        mLeafDim[0].x, mLeafDim[0].y, mLeafDim[0].z,
        ResourceFormat::RG16Float, 4, mRangeData.data(),
        ResourceBindFlags::ShaderResource
    );
    bricks.indirection = pDevice->createTexture3D(
        mLeafDim[0].x, mLeafDim[0].y, mLeafDim[0].z,
        ResourceFormat::RGBA8Uint, 1, mPtrData.data(),
        ResourceBindFlags::ShaderResource
    );
    bricks.atlas = pDevice->createTexture3D(
        getAtlasSizePixels().x, getAtlasSizePixels().y, getAtlasSizePixels().z,
        getAtlasFormat(), 1, mAtlasData.data(),
        ResourceBindFlags::ShaderResource
    );

    double dt = CpuTimer::calcDuration(t0, CpuTimer::getCurrentTimePoint());
    logDebug("Converted '{}' in {:.4}ms: mNonEmptyCount {} vs max {}",
              mpFloatGrid->gridName(), dt, mNonEmptyCount.load(), getAtlasMaxBrick());

    return bricks;
}
```

### Range Combination Pattern

```cpp
float2 combineMajMin(float2 a, float2 b)
{
    return float2(std::max(a.x, b.x), std::min(a.y, b.y));
}
```

### Mipmap Computation Pattern

```cpp
void computeMip(int mip)
{
    uint32_t* rangedst = mRangeData.data() + mLeafCount[mip - 1];
    uint32_t* rangesrc = mRangeData.data() + ((mip > 1) ? mLeafCount[mip - 2] : 0);
    int3 leafdim_src = mLeafDim[mip - 1];
    uint32_t rowstride_src = leafdim_src.x;
    uint32_t slicestride_src = leafdim_src.y * rowstride_src;

    int3 leafdim_tgt = mLeafDim[mip];
    uint32_t rowstride_tgt = leafdim_tgt.x;
    uint32_t slicestride_tgt = leafdim_tgt.y * rowstride_tgt;

    for (int z = 0; z < leafdim_tgt.z; ++z, rangesrc += slicestride_src)
    {
        for (int y = 0; y < leafdim_tgt.y; ++y, rangesrc += rowstride_src)
        {
            for (int x = 0; x < leafdim_tgt.x; ++x, rangesrc += 2)
            {
                float2 majmin_dst = combineMajMin(
                    combineMajMin(
                        combineMajMin(unpackMajMin(rangesrc), unpackMajMin(rangesrc + 1)),
                        combineMajMin(unpackMajMin(rangesrc + rowstride_src),
                                      unpackMajMin(rangesrc + 1 + rowstride_src))
                    ),
                    combineMajMin(
                        combineMajMin(unpackMajMin(rangesrc + slicestride_src),
                                      unpackMajMin(rangesrc + slicestride_src + 1)),
                        combineMajMin(unpackMajMin(rangesrc + slicestride_src + rowstride_src),
                                      unpackMajMin(rangesrc + slicestride_src + 1 + rowstride_src))
                    )
                );
                *rangedst++ = f32tof16(majmin_dst.x) + (f32tof16(majmin_dst.y) << 16);
            }
        }
    }
}
```

## Use Cases

### Volume Rendering

Primary use case is converting NanoVDB grids to GPU-friendly format:
- Medical imaging (CT, MRI)
- Scientific visualization
- Atmospheric effects
- Volumetric lighting

### Path Tracing

Supports Monte Carlo path tracing through volumes:
- Mipmap levels for majorant estimation
- Efficient ray marching with early termination
- Delta tracking for volume scattering

### Real-Time Rendering

Enables real-time volume rendering:
- GPU-accelerated texture sampling
- BC4 compression for reduced memory bandwidth
- Efficient bricked layout for cache performance

### Data Import

Convert various volume formats to GPU-ready format:
- OpenVDB files (industry standard)
- NanoVDB files (GPU-optimized)
- Custom formats through NanoVDB

## Performance Considerations

### Conversion Performance

- **Parallel Slice Conversion**: O(N/P) where N is number of bricks, P is number of threads
- **Mipmap Generation**: O(N) sequential
- **Texture Upload**: O(N) for uploading to GPU
- **Total**: O(N) with significant parallel speedup

### Memory Efficiency

- **BC4 Compression**: 50% reduction in atlas size
- **Sparse Representation**: Only allocate non-empty bricks
- **Mipmap Overhead**: ~33% additional memory for range texture mipmaps
- **Total**: ~67% of uncompressed size with BC4 compression

### Rendering Performance

- **Texture Caching**: Bricked layout improves cache hit rate
- **Early Termination**: Mipmap levels enable early ray termination
- **Hardware Acceleration**: GPU texture sampling is hardware-accelerated
- **Memory Bandwidth**: BC4 compression reduces memory bandwidth by 50%

### Parallel Efficiency

- **Slice Independence**: Each Z-slice can be processed independently
- **Atomic Allocation**: Minimal contention for brick allocation
- **Cache Reuse**: Halo fetching order maximizes NanoVDB cache reuse
- **Scalability**: Scales well with number of CPU cores

## Limitations

### Format Limitations

- **Float Only**: Only converts float grids
- **Fixed Brick Size**: 8-voxel brick size is fixed
- **Limited Bit Depths**: Only supports 4, 8, and 16-bit texels
- **BC4 Only**: Only supports BC4 compression (no BC7, etc.)

### Quality Limitations

- **BC4 Compression**: Introduces quantization error
- **Value Normalization**: Limited precision due to texel format
- **Halo Approximation**: 1-voxel halo may not be sufficient for all cases
- **Mipmap Approximation**: Coarse mipmap levels are conservative approximations

### Performance Limitations

- **Sequential Mipmap Generation**: Mipmap generation is sequential
- **Texture Upload**: Initial texture upload can be slow
- **Memory Overhead**: Dual representation (NanoVDB + BrickedGrid)
- **Atomic Contention**: Some contention for brick allocation

### Dynamic Limitations

- **One-Time Conversion**: Conversion is one-time, not incremental
- **No Streaming**: Entire grid must be converted at once
- **No Updates**: Cannot update BrickedGrid after conversion
- **No LOD**: No automatic level-of-detail support

## Best Practices

### When to Use GridConverter

1. **GPU Rendering**: Convert NanoVDB grids for GPU-accelerated rendering
2. **Sparse Data**: Leverage sparse representation for memory efficiency
3. **Real-Time Rendering**: Use BC4 compression for reduced memory bandwidth
4. **Path Tracing**: Use mipmap levels for efficient majorant estimation

### When to Avoid GridConverter

1. **CPU-Only Rendering**: Overhead of conversion not justified
2. **Dense Data**: Sparse representation may not provide benefits
3. **High Precision**: BC4 compression may not be suitable
4. **Dynamic Data**: Grid is immutable after conversion

### Usage Guidelines

1. **Format Selection**: Choose appropriate format (BC4, UNORM8, UNORM16)
2. **Parallel Execution**: Leverage parallel processing for large grids
3. **Memory Monitoring**: Monitor GPU memory usage for large grids
4. **Quality Trade-offs**: Balance compression ratio vs. quality

### Optimization Tips

1. **BC4 Compression**: Use BC4 for memory-constrained scenarios
2. **Parallel Processing**: Use parallel execution for large grids
3. **Halo Size**: Ensure halo size is sufficient for your use case
4. **Atlas Layout**: Optimize atlas layout for texture access patterns
5. **Mipmap Levels**: Use all 4 mipmap levels for early ray termination

## Notes

- Template-based converter supporting multiple texel types
- Supports BC4 (4-bit), UNORM8 (8-bit), UNORM16 (16-bit) formats
- BC4 compression provides 50% memory reduction
- Parallel slice conversion using C++17 parallel algorithms
- Automatic 4-level mipmap generation for range texture
- 1-voxel halo around each brick for smooth interpolation
- Atomic brick allocation for thread-safe parallel processing
- Bricked layout optimized for GPU texture access
- Range texture enables early ray termination
- Indirection texture enables sparse brick allocation
- Atlas texture stores compressed or uncompressed voxel data
- Fixed 8-voxel brick size (matches NanoVDB leaf size)
- Index bounding box aligned to 8-voxel brick boundaries
- Padded to 64-voxel boundary for 4 mipmap levels
- Halo fetching order maximizes NanoVDB cache reuse
- Value normalization to texel range for GPU storage
- Linear atlas layout with first two dimensions as powers of 2
- Total conversion time logged for performance monitoring
- Non-empty brick count vs. maximum brick count logged
