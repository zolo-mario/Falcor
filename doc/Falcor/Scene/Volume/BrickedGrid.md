# BrickedGrid - Bricked Grid Data Structure

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Core/API/Texture.h (ref<Texture>)

## Module Overview

BrickedGrid is a simple data structure that represents a bricked grid volume using three texture resources. It provides the storage foundation for sparse volumetric data representation, enabling efficient memory access patterns for GPU-based volume rendering.

## Component Specifications

### BrickedGrid Structure

**Signature**: `struct BrickedGrid`

**Purpose**: Represents a bricked grid volume with three texture resources

**Members**:
- `ref<Texture> range`: Range texture for storing value range information
- `ref<Texture> indirection`: Indirection texture for sparse brick lookup
- `ref<Texture> atlas`: Atlas texture storing actual voxel data

**Structure Definition**:
```cpp
struct BrickedGrid
{
    ref<Texture> range;
    ref<Texture> indirection;
    ref<Texture> atlas;
};
```

### Range Texture

**Purpose**: Stores value range information for bricks

**Characteristics**:
- Typically stores min/max values per brick
- Enables early ray termination in volume rendering
- Helps with empty space skipping
- Format: Usually RG16F or similar 2-channel format
- Resolution: Grid dimensions divided by brick size

**Usage Pattern**:
- Ray marching queries range texture to determine brick bounds
- Early termination if ray is outside all brick ranges
- Adaptive sampling based on value ranges

### Indirection Texture

**Purpose**: Maps grid coordinates to atlas brick locations

**Characteristics**:
- Stores brick indices or pointers into atlas
- Enables sparse representation by only allocating used bricks
- Format: Typically R32UI or similar integer format
- Resolution: Grid dimensions divided by brick size
- Value 0 or special value indicates empty/unallocated brick

**Usage Pattern**:
- Convert world/grid coordinates to brick coordinates
- Sample indirection texture to get atlas brick index
- If brick is allocated, sample atlas texture for voxel data
- If brick is empty, skip or use default value

### Atlas Texture

**Purpose**: Stores actual voxel data in a compact brick layout

**Characteristics**:
- Stores voxel data for allocated bricks
- Bricks are packed tightly in the atlas
- Format: Depends on data type (BC4 for single-channel, R8 for 8-bit, etc.)
- Resolution: Variable, depends on number of allocated bricks
- Brick size: Typically 8x8x8 or 16x16x16 voxels

**Usage Pattern**:
- Use indirection texture to locate brick in atlas
- Sample atlas texture using brick-local coordinates
- Retrieve actual voxel values for rendering

## Technical Details

### Bricked Grid Concept

A bricked grid divides the volume into fixed-size bricks (e.g., 8x8x8 voxels). Instead of storing all voxels contiguously, it uses a sparse representation:

1. **Grid Division**: Volume is divided into bricks
2. **Sparse Allocation**: Only allocate bricks containing meaningful data
3. **Indirection Mapping**: Map brick coordinates to atlas locations
4. **Range Information**: Store value ranges per brick for optimization

### Memory Layout

```
BrickedGrid Structure:
├── Range Texture (grid/brick_size x grid/brick_size x grid/brick_size)
│   └── Stores min/max values per brick
├── Indirection Texture (grid/brick_size x grid/brick_size x grid/brick_size)
│   └── Maps brick coordinates to atlas brick indices
└── Atlas Texture (variable dimensions)
    └── Stores voxel data for allocated bricks
        ├── Brick 0 (brick_size x brick_size x brick_size)
        ├── Brick 1 (brick_size x brick_size x brick_size)
        └── ...
```

### Coordinate Transformation

**World/Grid Space to Brick Space**:
```cpp
// Convert grid coordinate to brick coordinate
uint3 brickCoord = gridCoord / brickSize;

// Convert brick coordinate to brick-local coordinate
uint3 localCoord = gridCoord % brickSize;
```

**Brick Space to Atlas Space**:
```cpp
// Sample indirection texture to get atlas brick index
uint atlasBrickIndex = indirectionTexture[brickCoord];

// If brick is allocated, compute atlas coordinates
if (atlasBrickIndex != 0) {
    uint3 atlasBrickCoord = atlasBrickIndexToCoord(atlasBrickIndex);
    uint3 atlasCoord = atlasBrickCoord * brickSize + localCoord;
}
```

### Brick Packing

Bricks are packed in the atlas using a space-filling curve or linear layout:

**Linear Layout**:
```
Atlas Texture:
┌─────────────────┬─────────────────┬─────────────────┐
│     Brick 0     │     Brick 1     │     Brick 2     │
│  (0,0,0)        │  (1,0,0)        │  (2,0,0)        │
├─────────────────┼─────────────────┼─────────────────┤
│     Brick 3     │     Brick 4     │     Brick 5     │
│  (0,1,0)        │  (1,1,0)        │  (2,1,0)        │
└─────────────────┴─────────────────┴─────────────────┘
```

**Morton Order** (for better cache locality):
```
Brick indices follow Morton code (Z-order curve)
```

## Integration Points

### Usage in Volume Module

The BrickedGrid structure is used throughout the Volume module:

1. **GridVolume**: Uses BrickedGrid for sparse volume storage
2. **GridConverter**: Converts between different volume formats using BrickedGrid
3. **Volume Rendering**: Shaders sample BrickedGrid textures for volume data

### Integration Pattern

```cpp
// Example usage pattern
BrickedGrid grid;
grid.range = createRangeTexture();
grid.indirection = createIndirectionTexture();
grid.atlas = createAtlasTexture();

// Pass to shader
shaderData.brickedGrid = grid;
```

### Shader Integration

**Slang Shader Example**:
```slang
struct BrickedGrid
{
    Texture3D<float2> range;        // min/max range
    Texture3D<uint> indirection;    // brick index
    Texture3D<float> atlas;         // voxel data
};

float sampleVolume(BrickedGrid grid, float3 worldPos)
{
    // Convert to grid coordinates
    uint3 gridCoord = worldPosToGridCoord(worldPos);
    uint3 brickCoord = gridCoord / brickSize;
    uint3 localCoord = gridCoord % brickSize;

    // Sample indirection
    uint atlasBrickIndex = grid.indirection[brickCoord];

    // Check if brick is allocated
    if (atlasBrickIndex == 0) return 0.0;

    // Sample atlas
    uint3 atlasCoord = atlasBrickIndexToCoord(atlasBrickIndex);
    uint3 atlasLocalCoord = atlasCoord * brickSize + localCoord;
    return grid.atlas[atlasLocalCoord];
}
```

## Architecture Patterns

### Data Structure Pattern

BrickedGrid implements a sparse data structure pattern:
- **Indirection**: Uses indirection texture for sparse access
- **Atlas**: Packs allocated data in a compact atlas
- **Range**: Stores metadata for optimization

### Resource Management Pattern

Uses smart pointers for automatic resource management:
- **Reference Counting**: ref<> handles lifecycle
- **Automatic Cleanup**: Resources released when no longer referenced
- **Shared Ownership**: Multiple components can share the same grid

### Texture-Based Pattern

Leverages GPU texture hardware for efficient data access:
- **Hardware Acceleration**: GPU handles texture sampling
- **Caching**: GPU texture caches improve performance
- **Interpolation**: Hardware filtering for smooth sampling

## Code Patterns

### Structure Initialization Pattern

```cpp
BrickedGrid grid;
grid.range = device->createTexture3D(width, height, depth, format, ...);
grid.indirection = device->createTexture3D(width, height, depth, format, ...);
grid.atlas = device->createTexture3D(atlasWidth, atlasHeight, atlasDepth, format, ...);
```

### Brick Allocation Pattern

```cpp
uint allocateBrick(BrickedGrid& grid, uint3 brickCoord)
{
    uint atlasBrickIndex = getNextFreeBrickIndex();
    grid.indirection->setElement(brickCoord, atlasBrickIndex);
    return atlasBrickIndex;
}
```

### Range Update Pattern

```cpp
void updateBrickRange(BrickedGrid& grid, uint3 brickCoord, float2 range)
{
    grid.range->setElement(brickCoord, range);
}
```

## Use Cases

### Sparse Volume Data

Primary use case is storing sparse volumetric data:
- Medical imaging with empty regions
- Scientific visualization with sparse data
- Atmospheric effects with localized clouds
- Volumetric lighting with sparse participating media

### Memory Optimization

Reducing memory footprint for large volumes:
- Only allocate bricks with meaningful data
- Skip empty regions entirely
- Reduce memory bandwidth by skipping empty bricks

### Performance Optimization

Improving rendering performance:
- Early ray termination using range texture
- Empty space skipping using indirection texture
- Efficient texture caching for atlas access

### Dynamic Volume Updates

Supporting dynamic volume data:
- Allocate new bricks as needed
- Update range information dynamically
- Rebuild atlas periodically

## Performance Considerations

### Memory Efficiency

- **Sparse Representation**: Only store bricks with data
- **Compression**: Use compressed texture formats (BC4, etc.)
- **Atlas Packing**: Tight packing reduces atlas size
- **Range Texture**: Small texture for metadata

### Access Performance

- **Texture Caching**: GPU texture caches improve access speed
- **Spatial Locality**: Bricks improve spatial locality
- **Indirection Overhead**: One extra texture sample for indirection
- **Atlas Access**: Efficient access due to brick layout

### Build Performance

- **Brick Allocation**: O(N) where N is number of non-empty bricks
- **Range Computation**: O(BrickSize³) per brick
- **Atlas Packing**: O(N) for linear layout
- **Indirection Update**: O(N) for N bricks

### Rendering Performance

- **Ray Marching**: Early termination using range texture
- **Empty Space Skipping**: Skip bricks with indirection value 0
- **Texture Sampling**: Hardware-accelerated texture sampling
- **Cache Efficiency**: Brick layout improves cache hit rate

## Limitations

### Structural Limitations

- **Fixed Brick Size**: Brick size is fixed at creation time
- **Texture Resolution**: Limited by GPU texture size limits
- **Indirection Precision**: Limited by integer format precision
- **Atlas Fragmentation**: May become fragmented over time

### Quality Limitations

- **Discrete Sampling**: Limited to brick boundaries
- **Interpolation**: May introduce artifacts at brick boundaries
- **Compression**: Compressed formats introduce quantization error
- **Range Approximation**: Range texture provides only approximate bounds

### Dynamic Limitations

- **Brick Reallocation**: Difficult to reallocate bricks in atlas
- **Atlas Growth**: Limited by maximum texture size
- **Range Updates**: Requires recomputation when data changes
- **Indirection Updates**: May require texture updates

### Memory Limitations

- **Texture Memory**: Limited by GPU memory
- **Atlas Size**: May exceed texture size limits for large volumes
- **Range Texture**: Additional memory overhead for metadata
- **Indirection Texture**: Additional memory overhead for mapping

## Best Practices

### When to Use BrickedGrid

1. **Sparse Data**: Ideal for volumes with large empty regions
2. **Large Volumes**: Suitable for volumes that don't fit in memory
3. **GPU Rendering**: Designed for GPU-based volume rendering
4. **Dynamic Updates**: Supports dynamic volume data updates

### When to Avoid BrickedGrid

1. **Dense Data**: Use regular grid for dense volumes
2. **Small Volumes**: Overhead may not be justified
3. **CPU Access**: Not optimized for CPU-side access
4. **High Precision**: May not support high-precision formats

### Usage Guidelines

1. **Brick Size**: Choose brick size based on data characteristics
2. **Texture Formats**: Use appropriate formats for each texture
3. **Atlas Layout**: Consider Morton order for better cache locality
4. **Range Updates**: Update range texture when data changes
5. **Memory Management**: Monitor GPU memory usage

### Optimization Tips

1. **Early Termination**: Use range texture for early ray termination
2. **Empty Space Skipping**: Skip bricks with indirection value 0
3. **Texture Compression**: Use compressed formats for atlas
4. **Brick Layout**: Use Morton order for better cache locality
5. **Async Updates**: Use async texture updates for dynamic data

## Notes

- Simple POD structure with no complex logic
- Designed for GPU texture access
- Uses ref<> smart pointers for automatic memory management
- Three-texture design enables efficient sparse representation
- Range texture enables early ray termination
- Indirection texture enables sparse brick allocation
- Atlas texture stores actual voxel data
- Brick size is typically 8x8x8 or 16x16x16 voxels
- Atlas packing can use linear layout or Morton order
- Indirection value 0 or special value indicates empty brick
- Range texture typically uses RG16F format
- Indirection texture typically uses R32UI format
- Atlas texture format depends on data type
