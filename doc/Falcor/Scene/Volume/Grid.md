# Grid - Voxel Grid Based on NanoVDB

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Core/Object.h (Object base class)
- Falcor/Core/API/Device.h (Device creation)
- Falcor/Core/API/Buffer.h (Buffer for GPU storage)
- Falcor/Core/Program/ShaderVar.h (Shader variable binding)
- Falcor/Utils/Math/AABB.h (AABB for bounds)
- Falcor/Utils/Math/Matrix.h (Matrix for transformations)
- Falcor/Utils/UI/Gui.h (UI rendering)
- Falcor/Scene/Volume/BrickedGrid.h (BrickedGrid structure)
- Falcor/Scene/Volume/GridConverter.h (Grid conversion utilities)
- NanoVDB (nanovdb/NanoVDB.h, nanovdb/util/GridHandle.h, nanovdb/util/HostBuffer.h)
- NanoVDB Utilities (nanovdb/util/IO.h, nanovdb/util/GridStats.h, nanovdb/util/GridBuilder.h, nanovdb/util/Primitives.h, nanovdb/util/OpenToNanoVDB.h)
- OpenVDB (openvdb/openvdb.h)
- PNanoVDB (nanovdb/PNanoVDB.h for shader access)

## Module Overview

Grid is a comprehensive voxel grid implementation based on NanoVDB, providing sparse volumetric data representation with both CPU and GPU access. It supports multiple creation methods (sphere, box, file loading), multiple file formats (OpenVDB, NanoVDB), and provides efficient GPU rendering through bricked grid conversion.

## Component Specifications

### Grid Class (C++)

**Signature**: `class Grid : public Object`

**Purpose**: CPU-side voxel grid management with NanoVDB backend

**Factory Methods**:
- `createSphere(ref<Device> pDevice, float radius, float voxelSize, float blendRange = 3.f)`: Create sphere grid
- `createBox(ref<Device> pDevice, float width, float height, float depth, float voxelSize, float blendRange = 3.f)`: Create box grid
- `createFromFile(ref<Device> pDevice, const std::filesystem::path& path, const std::string& gridname)`: Load from file

**Query Methods**:
- `getMinIndex()`: Get minimum index (aligned to 8-voxel brick boundary)
- `getMaxIndex()`: Get maximum index (aligned to 8-voxel brick boundary)
- `getMinValue()`: Get minimum value stored in grid
- `getMaxValue()`: Get maximum value stored in grid
- `getVoxelCount()`: Get total number of active voxels
- `getGridSizeInBytes()`: Get total GPU memory size (NanoVDB + BrickedGrid)
- `getWorldBounds()`: Get world-space AABB
- `getValue(const int3& ijk)`: Get value at index (CPU-side, not thread-safe)
- `getGridHandle()`: Get raw NanoVDB grid handle
- `getTransform()`: Get affine transformation matrix (index-to-world)
- `getInvTransform()`: Get inverse affine transformation matrix (world-to-index)

**Rendering Methods**:
- `renderUI(Gui::Widgets& widget)`: Render debug UI with grid statistics
- `bindShaderData(const ShaderVar& var)`: Bind grid data to shader variables

**Private Methods**:
- `createFromNanoVDBFile()`: Load from .nvdb file
- `createFromOpenVDBFile()`: Load from .vdb file

**Member Variables**:
- `ref<Device> mpDevice`: GPU device
- `nanovdb::GridHandle<nanovdb::HostBuffer> mGridHandle`: NanoVDB host buffer
- `nanovdb::FloatGrid* mpFloatGrid`: Float grid pointer
- `nanovdb::FloatGrid::AccessorType mAccessor`: NanoVDB accessor
- `ref<Buffer> mpBuffer`: GPU buffer for NanoVDB data
- `BrickedGrid mBrickedGrid`: Bricked grid for GPU rendering

### Grid Struct (Slang)

**Signature**: `struct Grid`

**Purpose**: GPU-side voxel grid representation

**Data Members**:
- `int3 minIndex`: Minimum index stored in grid
- `float minValue`: Minimum value stored in grid
- `int3 maxIndex`: Maximum index stored in grid
- `float maxValue`: Maximum value stored in grid
- `StructuredBuffer<uint> buf`: NanoVDB data buffer
- `Texture3D<float2> rangeTex`: Brick range texture
- `Texture3D<uint4> indirectionTex`: Brick indirection texture
- `Texture3D<float> atlasTex`: Brick atlas texture

**Query Methods**:
- `getMinIndex()`: Get minimum index
- `getMaxIndex()`: Get maximum index
- `getMinValue()`: Get minimum value
- `getMaxValue()`: Get maximum value
- `getMeanValue()`: Get mean value from NanoVDB statistics
- `getStdDevValue()`: Get standard deviation from NanoVDB statistics

**Transform Methods**:
- `worldToIndexPos(float3 pos)`: Transform world position to index space
- `worldToIndexDir(float3 dir)`: Transform world direction to index space (normalized)
- `worldToIndexDirUnnormalized(float3 dir)`: Transform world direction to index space (unnormalized)
- `indexToWorldPos(float3 pos)`: Transform index position to world space
- `indexToWorldDir(float3 dir)`: Transform index direction to world space (normalized)
- `indexToWorldDirUnnormalized(float3 dir)`: Transform index direction to world space (unnormalized)

**Accessor Methods**:
- `createAccessor()`: Create NanoVDB accessor for efficient traversal
- `lookupWorld(float3 pos, inout Accessor accessor)`: Nearest-neighbor lookup in world space
- `lookupIndex(int3 index, inout Accessor accessor)`: Nearest-neighbor lookup in index space
- `lookupIndexTex(int3 index)`: Nearest-neighbor lookup using texture
- `lookupLinearWorld(float3 pos, inout Accessor accessor)`: Trilinear lookup in world space
- `lookupLinearIndex(float3 index, inout Accessor accessor)`: Trilinear lookup in index space
- `lookupLinearIndexTex(float3 index)`: Trilinear lookup using texture
- `lookupStochasticWorld(float3 pos, float3 u, inout Accessor accessor)`: Stochastic lookup in world space
- `lookupStochasticIndex(float3 index, float3 u, inout Accessor accessor)`: Stochastic lookup in index space
- `lookupStochasticIndexTex(float3 index, float3 u)`: Stochastic lookup using texture

**Majorant Lookup**:
- `lookupIndexLocalMajorantTex(int3 index, int mip)`: Lookup local majorant at mipmap level

### DDA Structure (Slang)

**Signature**: `struct DDA`

**Purpose**: Digital Differential Analyzer for efficient ray traversal through NanoVDB tree

**Enum**:
- `Level`: Traversal level (Voxel = 1, Leaf = 8)

**Constants**:
- `gridType`: PNANOVDB_GRID_TYPE_FLOAT

**Data Members**:
- `float3 iorigin`: Ray origin in index space
- `float3 idir`: Ray direction in index space (normalized)
- `float scale`: Scale factor for t values
- `Level lowestLevel`: Lowest traversal level
- `uint stepCount`: Number of steps taken
- `pnanovdb_hdda_t hdda`: NanoVDB hierarchical DDA state
- `pnanovdb_address_t cachedAddress`: Cached node address
- `uint cachedAddressLevel`: Cached node level (0-3)

**Methods**:
- `initialize(Grid grid, inout Accessor accessor, float3 pos, float3 dir, float tmin, float tmax, DDA::Level level)`: Initialize DDA
- `isActive(Grid grid, inout Accessor accessor, pnanovdb_coord_t ijk)`: Check if element is active
- `step(Grid grid, inout Accessor accessor, inout bool hasData, inout float dt)`: Make one DDA step
- `getStepData(Grid grid, inout Accessor accessor)`: Get data for current step

## Technical Details

### NanoVDB Tree Structure

NanoVDB uses a hierarchical tree structure for sparse volume representation:

```
Root (Level 3)
├── Upper Nodes (Level 2)
│   ├── Lower Nodes (Level 1)
│   │   └── Leaf Nodes (Level 0)
│   │       └── Voxels (8x8x8)
```

**Node Sizes**:
- Root: 32³ tiles
- Upper: 16³ tiles
- Lower: 8³ tiles
- Leaf: 8³ voxels

**Brick Alignment**: Index bounding box is aligned to 8-voxel brick boundaries for texture-based access

### Grid Creation

**Sphere Creation**:
```cpp
auto handle = nanovdb::createFogVolumeSphere<float>(
    radius,           // Sphere radius
    nanovdb::Vec3f(0.f),  // Center
    voxelSize,        // Voxel size
    blendRange        // Blend range in voxels
);
```

**Box Creation**:
```cpp
auto handle = nanovdb::createFogVolumeBox<float>(
    width, height, depth,  // Box dimensions
    nanovdb::Vec3f(0.f),  // Center
    voxelSize,        // Voxel size
    blendRange        // Blend range in voxels
);
```

**File Loading**:
- NanoVDB (.nvdb): Direct read using `nanovdb::io::readGrid()`
- OpenVDB (.vdb): Convert using `nanovdb::openToNanoVDB()`

### Grid Conversion to BrickedGrid

**Conversion Process**:
1. Create NanoVDB grid handle
2. Compute grid statistics if not present
3. Upload NanoVDB data to GPU buffer
4. Convert to BrickedGrid using `NanoVDBConverterBC4`
5. Store both representations for flexibility

**Memory Layout**:
```
GPU Memory:
├── NanoVDB Buffer (raw NanoVDB data)
└── BrickedGrid
    ├── Range Texture (min/max per brick)
    ├── Indirection Texture (brick mapping)
    └── Atlas Texture (voxel data)
```

**Memory Overhead**: ~15% increased footprint by keeping both representations

### Coordinate Transformations

**World-to-Index**:
```cpp
float3 worldToIndexPos(float3 pos)
{
    return pnanovdb_grid_world_to_indexf(buf, { pnanovdb_address_null() }, pos);
}
```

**Index-to-World**:
```cpp
float3 indexToWorldPos(float3 pos)
{
    return pnanovdb_grid_index_to_worldf(buf, { pnanovdb_address_null() }, pos);
}
```

**Affine Transform**:
- 3x3 matrix for linear transformation
- 3D vector for translation
- Supports rotation, scaling, and translation

### Sampling Methods

**Nearest-Neighbor**:
- Direct voxel lookup
- Fast but blocky
- Suitable for binary data or discrete values

**Trilinear**:
- Interpolates between 8 neighboring voxels
- Smooth but can blur sharp features
- Standard for volume rendering

**Stochastic**:
- Randomly selects one of 8 neighboring voxels
- Reduces variance in Monte Carlo rendering
- Useful for path tracing

**Texture-Based vs NanoVDB-Based**:
- Texture-based: Uses BrickedGrid textures (faster, compressed)
- NanoVDB-based: Uses NanoVDB buffer (more accurate, uncompressed)

### DDA Ray Traversal

**Initialization**:
```cpp
void initialize(Grid grid, inout Accessor accessor, float3 pos, float3 dir,
              float tmin, float tmax, DDA::Level level)
{
    iorigin = grid.worldToIndexPos(pos);
    idir = grid.worldToIndexDirUnnormalized(dir);
    const float length = sqrt(dot(idir, idir));
    tmin *= length;
    tmax *= length;
    scale = 1.f / length;
    idir *= scale;
    lowestLevel = level;

    // Setup HDDA
    pnanovdb_hdda_init(hdda, iorigin, tmin - 0.0001f, idir, tmax + 0.0001f, dim);
    stepCount = 0;
}
```

**Stepping**:
```cpp
bool step(Grid grid, inout Accessor accessor, inout bool hasData, inout float dt)
{
    if (stepCount > 0)
    {
        if (!pnanovdb_hdda_step(hdda)) return false;

        // Update level of traversal
        int dim = pnanovdb_uint32_as_int32(
            pnanovdb_readaccessor_get_dim(gridType, grid.buf, accessor, hdda.voxel)
        );
        dim = max(dim, int(lowestLevel));
        pnanovdb_hdda_update(hdda, iorigin, idir, dim);
    }

    hasData = isActive(grid, accessor, hdda.voxel);
    dt = (min(min(hdda.next.x, hdda.next.y), hdda.next.z) - hdda.tmin) * scale;

    stepCount++;
    return true;
}
```

**Active Check**:
- **Voxel Level**: Check if individual voxel is active
- **Leaf Level**: Check if any node above voxel is active, cache address for data retrieval

**Data Retrieval**:
- Returns max value at cached node level
- Supports different aggregation modes (max, min, ave, stddev)
- Currently hardcoded to max value

### Python Scripting Bindings

**Properties**:
- `voxelCount`: Read-only voxel count
- `minIndex`: Read-only minimum index
- `maxIndex`: Read-only maximum index
- `minValue`: Read-only minimum value
- `maxValue`: Read-only maximum value

**Methods**:
- `getValue(ijk)`: Get value at index
- `createSphere(radius, voxelSize, blendRange)`: Create sphere
- `createBox(width, height, depth, voxelSize, blendRange)`: Create box
- `createFromFile(path, gridname)`: Load from file

## Integration Points

### Usage in Volume Module

The Grid class is used throughout the Volume module:

1. **GridVolume**: Uses Grid for volume data representation
2. **GridConverter**: Converts between different volume formats
3. **Volume Rendering**: Shaders use Grid struct for volume sampling

### Integration Pattern

```cpp
// Create grid
ref<Grid> grid = Grid::createSphere(device, 10.0f, 0.1f, 3.0f);

// Bind to shader
ShaderVar var = shader["grid"];
grid->bindShaderData(var);

// Access from shader
float value = grid.lookupWorld(worldPos, accessor);
```

### Shader Integration

**Slang Shader Example**:
```slang
Grid grid;
Accessor accessor = grid.createAccessor();

// Sample volume
float density = grid.lookupLinearWorld(worldPos, accessor);

// Ray march with DDA
DDA dda;
dda.initialize(grid, accessor, rayOrigin, rayDir, tMin, tMax, DDA::Level::Voxel);

while (dda.step(grid, accessor, hasData, dt))
{
    if (hasData)
    {
        float value = dda.getStepData(grid, accessor);
        // Process value
    }
}
```

## Architecture Patterns

### Dual Representation Pattern

Grid maintains both CPU and GPU representations:
- **CPU**: NanoVDB host buffer for CPU access and manipulation
- **GPU**: BrickedGrid textures for efficient GPU rendering

### Factory Pattern

Multiple factory methods for different creation scenarios:
- `createSphere()`: Procedural sphere generation
- `createBox()`: Procedural box generation
- `createFromFile()`: File-based loading

### Adapter Pattern

NanoVDB accessor provides efficient traversal:
- Caches node addresses during traversal
- Reduces tree traversal overhead
- Supports both voxel and leaf-level traversal

### Strategy Pattern

Multiple sampling strategies:
- Nearest-neighbor: Fast, blocky
- Trilinear: Smooth, interpolated
- Stochastic: Random, variance-reducing

### Visitor Pattern

DDA traverses NanoVDB tree efficiently:
- Hierarchical traversal skips empty regions
- Adapts to different tree levels
- Caches node addresses for data retrieval

## Code Patterns

### Grid Creation Pattern

```cpp
ref<Grid> Grid::createSphere(ref<Device> pDevice, float radius, float voxelSize, float blendRange)
{
    auto handle = nanovdb::createFogVolumeSphere<float>(
        radius, nanovdb::Vec3f(0.f), voxelSize, blendRange
    );
    return ref<Grid>(new Grid(pDevice, std::move(handle)));
}
```

### File Loading Pattern

```cpp
ref<Grid> Grid::createFromFile(ref<Device> pDevice, const std::filesystem::path& path,
                             const std::string& gridname)
{
    if (!std::filesystem::exists(path))
    {
        logWarning("Error when loading grid. Can't open grid file '{}'.", path);
        return nullptr;
    }

    if (hasExtension(path, "nvdb"))
    {
        return createFromNanoVDBFile(pDevice, path, gridname);
    }
    else if (hasExtension(path, "vdb"))
    {
        return createFromOpenVDBFile(pDevice, path, gridname);
    }
    else
    {
        logWarning("Error when loading grid. Unsupported grid file '{}'.", path);
        return nullptr;
    }
}
```

### Shader Binding Pattern

```cpp
void Grid::bindShaderData(const ShaderVar& var)
{
    var["buf"] = mpBuffer;
    var["rangeTex"] = mBrickedGrid.range;
    var["indirectionTex"] = mBrickedGrid.indirection;
    var["atlasTex"] = mBrickedGrid.atlas;
    var["minIndex"] = getMinIndex();
    var["minValue"] = getMinValue();
    var["maxIndex"] = getMaxIndex();
    var["maxValue"] = getMaxValue();
}
```

### Trilinear Interpolation Pattern

```cpp
float lookupLinearIndex(const float3 index, inout Accessor accessor)
{
    const float3 indexOffset = index - 0.5f;
    const int3 i = floor(indexOffset);
    const float3 f = indexOffset - i;
    const float x0z0 = lerp(lookupIndex(i + int3(0, 0, 0), accessor),
                            lookupIndex(i + int3(1, 0, 0), accessor), f.x);
    const float x1z0 = lerp(lookupIndex(i + int3(0, 1, 0), accessor),
                            lookupIndex(i + int3(1, 1, 0), accessor), f.x);
    const float y0 = lerp(x0z0, x1z0, f.y);
    const float x0z1 = lerp(lookupIndex(i + int3(0, 0, 1), accessor),
                            lookupIndex(i + int3(1, 0, 1), accessor), f.x);
    const float x1z1 = lerp(lookupIndex(i + int3(0, 1, 1), accessor),
                            lookupIndex(i + int3(1, 1, 1), accessor), f.x);
    const float y1 = lerp(x0z1, x1z1, f.y);
    return lerp(y0, y1, f.z);
}
```

## Use Cases

### Volume Rendering

Primary use case is GPU-accelerated volume rendering:
- Medical imaging (CT, MRI)
- Scientific visualization
- Atmospheric effects
- Volumetric lighting

### Path Tracing

Supports Monte Carlo path tracing through volumes:
- Stochastic sampling for variance reduction
- DDA traversal for efficient ray marching
- Majorant estimation for delta tracking

### Procedural Content

Generate procedural volume data:
- Sphere and box primitives
- Fog volumes with blend ranges
- Custom shapes through file loading

### Data Import

Load volume data from various sources:
- OpenVDB files (industry standard)
- NanoVDB files (GPU-optimized)
- Custom formats through conversion

## Performance Considerations

### Memory Efficiency

- **Sparse Representation**: NanoVDB tree only stores active voxels
- **Bricked Grid**: Further reduces memory through BC4 compression
- **Dual Representation**: ~15% overhead for flexibility
- **Texture Compression**: BC4 reduces atlas size by 50%

### Access Performance

- **NanoVDB Accessor**: Caches node addresses for efficient traversal
- **Texture-Based Access**: Hardware-accelerated texture sampling
- **DDA Traversal**: Hierarchical traversal skips empty regions
- **Brick Alignment**: 8-voxel bricks for optimal texture access

### Rendering Performance

- **Trilinear Sampling**: Hardware-accelerated texture filtering
- **Stochastic Sampling**: Reduces variance in Monte Carlo rendering
- **Majorant Lookup**: Enables delta tracking for efficient volume path tracing
- **Early Termination**: Range texture enables early ray termination

### Build Performance

- **Grid Creation**: O(N) where N is number of active voxels
- **File Loading**: O(N) for reading and conversion
- **Bricked Conversion**: O(N) for converting to BrickedGrid
- **Statistics Computation**: O(N) for computing min/max/mean/stddev

## Limitations

### Format Limitations

- **Float Only**: Currently only supports float grids
- **Fixed Brick Size**: 8-voxel brick size is fixed
- **NanoVDB Version**: Dependent on specific NanoVDB version
- **OpenVDB Dependency**: Requires OpenVDB for .vdb file loading

### Quality Limitations

- **Compression Artifacts**: BC4 compression introduces quantization error
- **Interpolation Artifacts**: Trilinear sampling can blur sharp features
- **Brick Boundary Artifacts**: Potential discontinuities at brick boundaries
- **Stochastic Variance**: Stochastic sampling introduces noise

### Performance Limitations

- **Dual Representation**: ~15% memory overhead
- **CPU Access**: `getValue()` is not thread-safe
- **Conversion Overhead**: BrickedGrid conversion takes time
- **Texture Upload**: Initial texture upload can be slow

### Dynamic Limitations

- **No Dynamic Updates**: Grid is immutable after creation
- **No Real-time Modification**: Cannot modify grid data at runtime
- **No Streaming**: Entire grid must be in GPU memory
- **No LOD**: No automatic level-of-detail support

## Best Practices

### When to Use Grid

1. **Sparse Volume Data**: Ideal for volumes with large empty regions
2. **GPU Rendering**: Designed for GPU-accelerated volume rendering
3. **Multiple Formats**: Need to support both OpenVDB and NanoVDB
4. **Path Tracing**: Supports Monte Carlo path tracing through volumes

### When to Avoid Grid

1. **Dense Data**: Use regular grid for dense volumes
2. **CPU-Only Rendering**: Overhead of GPU conversion not justified
3. **Real-Time Updates**: Grid is immutable after creation
4. **High Precision**: BC4 compression may not be suitable

### Usage Guidelines

1. **Voxel Size**: Choose appropriate voxel size for your use case
2. **Blend Range**: Adjust blend range for smooth transitions
3. **Sampling Mode**: Choose appropriate sampling mode (nearest/trilinear/stochastic)
4. **Traversal Level**: Choose appropriate DDA level (voxel/leaf)
5. **Memory Management**: Monitor GPU memory usage for large grids

### Optimization Tips

1. **Texture-Based Access**: Use texture-based access for better performance
2. **DDA Traversal**: Use DDA for efficient ray marching
3. **Majorant Lookup**: Use majorant lookup for delta tracking
4. **Brick Alignment**: Ensure data is aligned to brick boundaries
5. **Compression**: Use BC4 compression to reduce memory footprint

## Notes

- Based on NanoVDB sparse volume representation
- Maintains both CPU and GPU representations
- Supports OpenVDB (.vdb) and NanoVDB (.nvdb) file formats
- Converts to BrickedGrid format for efficient GPU rendering
- ~15% memory overhead for dual representation
- Brick size is fixed at 8 voxels
- Index bounding box aligned to 8-voxel brick boundaries
- Supports multiple sampling modes (nearest, trilinear, stochastic)
- Implements DDA for efficient ray traversal
- Provides affine transformations between world and index space
- Python scripting bindings available
- `getValue()` is not thread-safe
- Grid is immutable after creation
- Currently only supports float grids
- Uses BC4 compression for BrickedGrid atlas
- NanoVDB accessor caches node addresses for efficient traversal
- DDA supports both voxel and leaf-level traversal
- Majorant lookup enables delta tracking for volume path tracing
