# HitInfo - Ray Hit Information

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **HitInfo** - Hit information host-side utility
  - [x] **HitInfo.h** - Hit information header (72 lines)
  - [x] **HitInfo.cpp** - Hit information implementation (124 lines)
- [x] **HitInfo.slang** - Hit information shader (500 lines)
- [x] **HitInfoType.slang** - Hit information type definitions (52 lines)

### External Dependencies

- **Core/Macros** - Macro definitions
- **Core/API/Formats** - Resource format definitions
- **Core/Program/DefineList** - Shader define list
- **Scene/Scene** - Scene interface
- **Utils/Logger** - Logging utilities
- **Utils/Math/FormatConversion** - Format conversion utilities

## Module Overview

HitInfo provides a polymorphic container for storing ray hit information. PackedHitInfo holds data in HitInfo. The upper bits of the first uint are used to store the type of hit. A zero initialized value represents hit type 'None'. HitInfo stores data for one of the following type specific structs: TriangleHit, DisplacedTriangleHit, CurveHit, CurveOTSHit, SDFGridHit, VolumeHit.

By default, HitInfo is encoded in 128 bits. There is a compression mode where HitInfo is encoded in 64 bits. This mode is only available in scenes that exclusively use triangle meshes and are small enough so header information fits in 32 bits. In compression mode, barycentrics are quantized to 16 bit unorms.

## Component Specifications

### HitInfo (Host-side Utility)

**File**: [`HitInfo.h`](Source/Falcor/Scene/HitInfo.h:1)

**Purpose**: Host side utility to setup bit allocations for device side HitInfo.

**Key Features**:
- Bit allocation for hit information encoding
- Compression mode support (64 bits vs 128 bits)
- Dynamic bit allocation based on scene content
- Shader define generation
- Resource format selection

**Public Methods**:
- [`HitInfo()`](Source/Falcor/Scene/HitInfo.h:53) - Default constructor
- [`HitInfo(const Scene& scene, bool useCompression = false)`](Source/Falcor/Scene/HitInfo.h:54) - Constructor with scene and compression flag
- [`init(const Scene& scene, bool useCompression)`](Source/Falcor/Scene/HitInfo.h:55) - Initialize hit info with scene
- [`getDefines() const`](Source/Falcor/Scene/HitInfo.h:59) - Returns defines needed for packing/unpacking HitInfo struct
- [`getFormat() const`](Source/Falcor/Scene/HitInfo.h:63) - Returns resource format required for encoding packed hit information

**Private Members**:
- [`mUseCompression`](Source/Falcor/Scene/HitInfo.h:66) - Store in compressed format (64 bits instead of 128 bits)
- [`mTypeBits`](Source/Falcor/Scene/HitInfo.h:68) - Number of bits to store hit type
- [`mInstanceIDBits`](Source/Falcor/Scene/HitInfo.h:69) - Number of bits to store instance ID
- [`mPrimitiveIndexBits`](Source/Falcor/Scene/HitInfo.h:70) - Number of bits to store primitive index

### HitInfo Implementation

**File**: [`HitInfo.cpp`](Source/Falcor/Scene/HitInfo.cpp:1)

**Purpose**: Hit information implementation with bit allocation and compression support.

**Key Features**:
- Bit allocation algorithm using reverse bit scan
- Scene-based bit allocation for instance IDs and primitive indices
- Compression support detection
- Define list generation
- Resource format selection (RGBA32Uint vs RG32Uint)

**Implementation Details**:

**Bit Allocation**:
- [`allocateBits(const uint32_t count)`](Source/Falcor/Scene/HitInfo.cpp:44) - Allocates minimum bits needed to represent count values
- Uses reverse bit scan algorithm: `bitScanReverse(maxValue) + 1`
- Handles edge case: `if (count <= 1) return 0`

**Initialization**:
- [`init(const Scene& scene, bool useCompression)`](Source/Falcor/Scene/HitInfo.cpp:52) - Initializes hit info with scene
- Allocates bits for hit type based on `HitType::Count`
- Allocates bits for instance ID based on `scene.getGeometryInstanceCount()`
- Allocates bits for primitive index based on maximum primitive count across meshes, curves, and SDF grids
- Checks that final bit allocation fits: `if (mPrimitiveIndexBits > 32 || (mTypeBits + mInstanceIDBits) > 32)`

**Compression Support**:
- Computes compressed header bits: `kCompressedTypeBits + mInstanceIDBits + mPrimitiveIndexBits`
- Checks compression support: `compressedHeaderBits <= 32 && scene.getGeometryTypes() == Scene::GeometryTypeFlags::TriangleMesh`
- Uses compression if supported and requested
- Switches to compressed type bits: `if (mUseCompression) mTypeBits = kCompressedTypeBits`

**Static Assertions**:
- Ensures hit types fit in compressed type field: `(uint32_t)HitType::None < (1 << kCompressedTypeBits)`
- Ensures triangle hit type fits: `(uint32_t)HitType::Triangle < (1 << kCompressedTypeBits)`
- Ensures volume hit type fits: `(uint32_t)HitType::Volume < (1 << kCompressedTypeBits)`

**Define Generation**:
- [`getDefines() const`](Source/Falcor/Scene/HitInfo.cpp:108) - Generates shader defines
- Adds `HIT_INFO_DEFINES = "1"`
- Adds `HIT_INFO_USE_COMPRESSION` based on compression mode
- Adds bit count defines: `HIT_INFO_TYPE_BITS`, `HIT_INFO_INSTANCE_ID_BITS`, `HIT_INFO_PRIMITIVE_INDEX_BITS`

**Resource Format**:
- [`getFormat() const`](Source/Falcor/Scene/HitInfo.cpp:120) - Returns resource format
- Compressed mode: `ResourceFormat::RG32Uint`
- Uncompressed mode: `ResourceFormat::RGBA32Uint`

### HitInfoType (Hit Type Enum)

**File**: [`HitInfoType.slang`](Source/Falcor/Scene/HitInfoType.slang:1)

**Purpose**: Hit type enumeration for ray hit information.

**Key Features**:
- Hit type enumeration
- Base hit types (None, Triangle, Volume)
- Extended hit types (DisplacedTriangle, Curve, SDFGrid) available without compression

**Hit Types**:
- [`None = 0`](Source/Falcor/Scene/HitInfoType.slang:35) - No hit
- [`Triangle = 1`](Source/Falcor/Scene/HitInfoType.slang:36) - Triangle hit
- [`Volume = 2`](Source/Falcor/Scene/HitInfoType.slang:37) - Volume hit
- [`DisplacedTriangle = 3`](Source/Falcor/Scene/HitInfoType.slang:41) - Displaced triangle hit (uncompressed only)
- [`Curve = 4`](Source/Falcor/Scene/HitInfoType.slang:42) - Curve hit (uncompressed only)
- [`SDFGrid = 6`](Source/Falcor/Scene/HitInfoType.slang:43) - SDF grid hit (uncompressed only)
- [`Count`](Source/Falcor/Scene/HitInfoType.slang:49) - Must be last

### HitInfo (Shader-side)

**File**: [`HitInfo.slang`](Source/Falcor/Scene/HitInfo.slang:1)

**Purpose**: Ray hit information shader with polymorphic container for storing ray hit information.

**Key Features**:
- Polymorphic hit information container
- Multiple hit type specific structs (TriangleHit, DisplacedTriangleHit, CurveHit, CurveOTSHit, SDFGridHit, VolumeHit)
- Packed hit info encoding/decoding
- Compression support (64 bits vs 128 bits)
- Barycentric weight computation
- Hit type extraction
- Instance ID and primitive index extraction

**Shader Defines**:
- `HIT_INFO_DEFINES` - Marks that the rest of the defines are available
- `HIT_INFO_USE_COMPRESSION` - Use compressed format (64 bits instead of 128 bits)
- `HIT_INFO_TYPE_BITS` - Bits needed to encode the hit type
- `HIT_INFO_INSTANCE_ID_BITS` - Bits needed to encode the instance ID of the hit
- `HIT_INFO_PRIMITIVE_INDEX_BITS` - Bits needed to encode the primitive index of the hit

**PackedHitInfo Type**:
- With compression: `typedef uint2 PackedHitInfo`
- Without compression: `typedef uint4 PackedHitInfo`

**GeometryHit (Base Class)**:
- [`GeometryInstanceID instanceID`](Source/Falcor/Scene/HitInfo.slang:70) - Geometry instance ID
- [`uint primitiveIndex`](Source/Falcor/Scene/HitInfo.slang:71) - Primitive index
- [`float2 barycentrics`](Source/Falcor/Scene/HitInfo.slang:72) - Barycentric coordinates

**Method**:
- [`getBarycentricWeights()`](Source/Falcor/Scene/HitInfo.slang:76) - Returns barycentric weights: `float3(1.f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y)`

**TriangleHit**:
- **Encoding (without compression)**: header (64 bits) | barycentrics.x (32 bit float) | barycentrics.y (32 bit float)
- **Encoding (with compression)**: header (32 bits) | barycentrics.x (16 bit unorm) | barycentrics.y (16 bit unorm)

**Methods**:
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:94) - Initialize from packed hit info
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:109) - Pack to packed hit info

**DisplacedTriangleHit**:
- **Encoding**: header (64 bits) | barycentrics.x (24 bit unorm) | barycentrics.y (24 bit unorm) | displacement (16 bit float)

**Additional Member**:
- [`float displacement`](Source/Falcor/Scene/HitInfo.slang:133) - Displacement offset

**Methods**:
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:135) - Initialize from packed hit info
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:148) - Pack to packed hit info

**Barycentric Decoding**:
- Unpacks from 24-bit unorm: `float2(ux, uy) * (1.f / 16777215)`
- Where `ux = (packed[2] >> 8)` and `uy = ((packed[2] & 0xff) << 16) | (packed[3] >> 16)`

**CurveHit**:
- **Encoding (without compression)**: header (64 bits) | barycentrics.x (32 bit float) | barycentrics.y (32 bit float)

**Methods**:
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:169) - Initialize from packed hit info
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:180) - Pack to packed hit info

**SDFGridHit**:
- **Encoding**: header (64 bits) | extra (32 bit) | extra (32 bit)

**Members**:
- [`GeometryInstanceID instanceID`](Source/Falcor/Scene/HitInfo.slang:201) - Geometry instance ID
- [`SDFGridHitData hitData`](Source/Falcor/Scene/HitInfo.slang:202) - SDF grid hit data

**Methods**:
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:204) - Initialize from packed hit info
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:228) - Pack to packed hit info

**SDF Grid Implementation Support**:
- `SCENE_SDF_GRID_IMPLEMENTATION_NDSDF`: `hitData.lod = primitiveData`, `hitData.hitT = asfloat(packed[2])`
- `SCENE_SDF_GRID_IMPLEMENTATION_SVS || SCENE_SDF_GRID_IMPLEMENTATION_SBS`: `hitData.primitiveID = primitiveData`, `hitData.hitT = asfloat(packed[2])`
- `SCENE_SDF_GRID_IMPLEMENTATION_SVO`: `hitData.svoIndex = primitiveData`, `hitData.hitT = asfloat(packed[2])`

**VolumeHit**:
- **Encoding (without compression)**: header (64 bits) | t (32 bit float) | g (32 bit float)
- **Encoding (with compression)**: header (16 bits) | g (16 bit float) | t (32 bit float)

**Members**:
- [`float t`](Source/Falcor/Scene/HitInfo.slang:266) - t parameter
- [`float g`](Source/Falcor/Scene/HitInfo.slang:267) - g parameter

**Methods**:
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:269) - Initialize from packed hit info
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:280) - Pack to packed hit info

**Compression Decoding**:
- With compression: `t = asfloat(packed[1])`, `g = f16tof32(packed[0] & 0xffff)`
- Without compression: `t = asfloat(packed[1])`, `g = asfloat(packed[2])`

**Compression Encoding**:
- With compression: `packed[1] = asuint(t)`, `packed[0] |= f32tof16(g) & 0xffff`
- Without compression: `packed[1] = asuint(t)`, `packed[2] = asuint(g)`

**HitInfo (Polymorphic Container)**:

**Static Constants**:
- `kTypeBits` - Bits for hit type (from defines or default 1)
- `kInstanceIDBits` - Bits for instance ID (from defines or default 1)
- `kPrimitiveIndexBits` - Bits for primitive index (from defines or default 1)
- `kTypeOffset` - Offset for type bits: `32u - kTypeBits`
- `kInstanceIDOffset` - Offset for instance ID bits: `kPrimitiveIndexBits`
- `kHeaderBits` - Total header bits: `kTypeBits + kInstanceIDBits + kPrimitiveIndexBits`

**Member**:
- [`PackedHitInfo data`](Source/Falcor/Scene/HitInfo.slang:314) - Packed hit information

**Methods**:
- [`__init()`](Source/Falcor/Scene/HitInfo.slang:318) - Initialize empty (invalid) hit info
- [`__init(const PackedHitInfo packed)`](Source/Falcor/Scene/HitInfo.slang:326) - Initialize hit info from a packed hit info
- [`__init(const TriangleHit triangleHit)`](Source/Falcor/Scene/HitInfo.slang:334) - Initialize hit info from a triangle hit
- [`__init(const DisplacedTriangleHit displacedTriangleHit)`](Source/Falcor/Scene/HitInfo.slang:342) - Initialize hit info from a displaced triangle hit
- [`__init(const CurveHit curveHit)`](Source/Falcor/Scene/HitInfo.slang:350) - Initialize hit info from a curve hit
- [`__init(const SDFGridHit sdfGridHit)`](Source/Falcor/Scene/HitInfo.slang:359) - Initialize hit info from a SDF grid hit
- [`__init(const VolumeHit volumeHit)`](Source/Falcor/Scene/HitInfo.slang:367) - Initialize hit info from a volume hit
- [`isValid()`](Source/Falcor/Scene/HitInfo.slang:374) - Return true if object represents a valid hit
- [`getType()`](Source/Falcor/Scene/HitInfo.slang:381) - Return hit type
- [`getTriangleHit()`](Source/Falcor/Scene/HitInfo.slang:389) - Return the triangle hit (only valid if type is HitType::Triangle)
- [`getDisplacedTriangleHit()`](Source/Falcor/Scene/HitInfo.slang:397) - Return the displaced triangle hit (only valid if type is HitType::DisplacedTriangle)
- [`getCurveHit()`](Source/Falcor/Scene/HitInfo.slang:405) - Return the curve hit (only valid if type is HitType::Curve)
- [`getSDFGridHit()`](Source/Falcor/Scene/HitInfo.slang:414) - Return the SDF grid hit (only valid if type is HitType::SDFGrid)
- [`getVolumeHit()`](Source/Falcor/Scene/HitInfo.slang:422) - Return the volume hit (only valid if type is HitType::Volume)
- [`pack()`](Source/Falcor/Scene/HitInfo.slang:430) - Return the packed hit info
- [`getInstanceID()`](Source/Falcor/Scene/HitInfo.slang:438) - Get the geometry instance id from the packed header
- [`getPrimitiveIndex()`](Source/Falcor/Scene/HitInfo.slang:449) - Get the primitive index from the packed header

**Static Methods**:
- [`packHeader(inout PackedHitInfo packed, const HitType type)`](Source/Falcor/Scene/HitInfo.slang:457) - Pack header with hit type
- [`packHeader(inout PackedHitInfo packed, const HitType type, const GeometryInstanceID instanceID, const uint primitiveIndex)`](Source/Falcor/Scene/HitInfo.slang:462) - Pack header with hit type, instance ID, and primitive index
- [`unpackHeader(const PackedHitInfo packed, out GeometryInstanceID instanceID, out uint primitiveIndex)`](Source/Falcor/Scene/HitInfo.slang:475) - Unpack header to extract instance ID and primitive index

**Header Packing Logic**:
- If `kHeaderBits <= 32`: Single 32-bit header in `packed[0]`
- If `kHeaderBits > 32`: Split across `packed[0]` (type + instanceID) and `packed[1]` (primitiveIndex)

**Header Unpacking Logic**:
- If `kHeaderBits <= 32`: Extract from `packed[0]`
- If `kHeaderBits > 32`: Extract instanceID from `packed[0]`, primitiveIndex from `packed[1]`

**Helper Function**:
- [`makeInvalidHit()`](Source/Falcor/Scene/HitInfo.slang:490) - Create invalid hit info
- [`unpackHitInfo(PackedHitInfo packedHitInfo)`](Source/Falcor/Scene/HitInfo.slang:497) - Unpack packed hit info

## Technical Details

### Bit Allocation Algorithm

**Reverse Bit Scan**:
- Computes minimum bits needed to represent count values
- Formula: `bitScanReverse(maxValue) + 1`
- Handles edge case: count of 0 or 1 needs 0 bits

**Scene-Based Allocation**:
- Type bits: Based on `HitType::Count`
- Instance ID bits: Based on `scene.getGeometryInstanceCount()`
- Primitive index bits: Based on maximum of:
  - Triangle count across all meshes
  - Curve segment count across all curves
  - SDF grid max primitive ID bits

### Compression Mode

**Compression Requirements**:
- Compressed header bits <= 32
- Scene geometry types == TriangleMesh only
- Hit types limited to None, Triangle, Volume (2 bits)

**Compression Benefits**:
- Reduces hit info size from 128 bits to 64 bits
- Barycentric quantization to 16-bit unorm
- Reduced memory bandwidth for ray tracing

**Compression Trade-offs**:
- Only available for triangle-only scenes
- Barycentric precision loss from quantization
- Limited hit type support

### Encoding Formats

**TriangleHit (Uncompressed)**:
- Header: 64 bits (type, instanceID, primitiveIndex)
- Barycentrics.x: 32-bit float
- Barycentrics.y: 32-bit float
- Total: 128 bits (uint4)

**TriangleHit (Compressed)**:
- Header: 32 bits (type, instanceID, primitiveIndex)
- Barycentrics.x: 16-bit unorm
- Barycentrics.y: 16-bit unorm
- Total: 64 bits (uint2)

**DisplacedTriangleHit**:
- Header: 64 bits (type, instanceID, primitiveIndex)
- Barycentrics.x: 24-bit unorm
- Barycentrics.y: 24-bit unorm
- Displacement: 16-bit float
- Total: 128 bits (uint4)

**CurveHit**:
- Header: 64 bits (type, instanceID, primitiveIndex)
- Barycentrics.x: 32-bit float
- Barycentrics.y: 32-bit float
- Total: 128 bits (uint4)

**SDFGridHit**:
- Header: 64 bits (type, instanceID, primitiveData)
- Extra: 32 bits (depends on SDF implementation)
- Extra: 32 bits (hitT)
- Total: 128 bits (uint4)

**VolumeHit (Uncompressed)**:
- Header: 64 bits (type, instanceID, primitiveIndex)
- t: 32-bit float
- g: 32-bit float
- Total: 128 bits (uint4)

**VolumeHit (Compressed)**:
- Header: 16 bits (type)
- g: 16-bit float
- t: 32-bit float
- Total: 64 bits (uint2)

### Barycentric Computation

**Barycentric Weights**:
- Formula: `float3(1.f - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y)`
- Ensures weights sum to 1.0
- Used for interpolation within triangle

**Displaced Triangle Barycentrics**:
- Encoded as 24-bit unorm values
- Range: [0, 1]
- Decoded with scale factor: 1.0 / 16777215
- Precision: ~7 decimal digits

### Hit Type Detection

**Type Extraction**:
- Extracts type from upper bits of first uint
- Uses `kTypeOffset` for bit shift
- Returns `HitType` enum value

**Validation**:
- `isValid()` returns true if type != HitType::None
- Type-specific getters check for valid type before returning

### Shader Integration

**Define List**:
- Generated by host-side HitInfo class
- Passed to shader compilation
- Controls packing/unpacking behavior

**Resource Formats**:
- Uncompressed: `ResourceFormat::RGBA32Uint` (128 bits)
- Compressed: `ResourceFormat::RG32Uint` (64 bits)
- Used for hit info texture/buffer storage

## Integration Points

### Scene Integration

**Scene Builder**:
- HitInfo initialization during scene loading
- Bit allocation based on scene content
- Compression mode selection

**Ray Tracing Integration**:
- Hit info storage in ray tracing payload
- Hit info encoding in ray tracing shaders
- Hit info decoding in shading passes

**Material System Integration**:
- Hit info used for material shading
- Barycentric interpolation for material properties
- Instance ID for material selection

**Geometry System Integration**:
- Hit info stores geometry instance and primitive index
- Supports multiple geometry types (triangles, curves, SDF grids, volumes)
- Instance-based geometry access

## Architecture Patterns

### Polymorphic Container Pattern

- HitInfo acts as polymorphic container for multiple hit types
- Type-specific structures for different geometry types
- Common interface for hit information access

### Bit Packing Pattern

- Efficient bit packing for multiple data fields
- Dynamic bit allocation based on scene content
- Compression support for memory optimization

### Factory Pattern

- Host-side HitInfo class generates shader defines
- Configures packing/unpacking behavior
- Selects resource format based on compression mode

### Strategy Pattern

- Different encoding strategies for different hit types
- Compression strategy for triangle-only scenes
- Bit allocation strategy for optimal memory usage

## Code Patterns

### Bit Allocation Pattern

```cpp
uint32_t allocateBits(const uint32_t count)
{
    if (count <= 1) return 0;
    uint32_t maxValue = count - 1;
    return bitScanReverse(maxValue) + 1;
}
```

### Compression Detection Pattern

```cpp
const bool compressionSupported = compressedHeaderBits <= 32 && 
    scene.getGeometryTypes() == Scene::GeometryTypeFlags::TriangleMesh;
mUseCompression = compressionSupported && useCompression;
```

### Header Packing Pattern

```slang
static void packHeader(inout PackedHitInfo packed, const HitType type)
{
    packed[0] = uint(type) << kTypeOffset;
}
```

### Barycentric Decoding Pattern

```slang
const uint ux = (packed[2] >> 8);
const uint uy = ((packed[2] & 0xff) << 16) | (packed[3] >> 16);
barycentrics = float2(ux, uy) * (1.f / 16777215);
```

## Use Cases

### Ray Tracing

- Store ray hit information for shading
- Support multiple geometry types
- Efficient memory usage with compression

### Material Shading

- Barycentric interpolation for material properties
- Instance-based material selection
- Hit point reconstruction

### Path Tracing

- Hit information for path continuation
- Multiple bounce support
- Geometry type handling

### Debugging

- Hit information validation
- Hit type inspection
- Barycentric weight visualization

## Performance Considerations

### GPU Computation

**Bit Operations**:
- Bit packing/unpacking is O(1) per hit
- Minimal arithmetic operations
- Efficient GPU implementation

**Memory Usage**:
- Uncompressed: 128 bits per hit (uint4)
- Compressed: 64 bits per hit (uint2)
- 50% memory reduction with compression

**Memory Bandwidth**:
- Reduced memory bandwidth with compression
- Fewer texture reads for hit info
- Improved cache efficiency

### Computational Complexity

**Hit Info Operations**:
- Packing: O(1) per hit
- Unpacking: O(1) per hit
- Type extraction: O(1) per hit
- Barycentric weight computation: O(1) per hit

**Bit Allocation**:
- Host-side: O(n) where n is number of geometry types
- Once per scene initialization
- Negligible runtime overhead

## Limitations

### Feature Limitations

- **Compression Limitations**:
  - Only available for triangle-only scenes
  - Barycentric precision loss from quantization
  - Limited hit type support with compression
  - No displaced triangles with compression
  - No curves with compression
  - No SDF grids with compression
  - No volumes with compression

- **Hit Type Limitations**:
  - Maximum 6 hit types (None, Triangle, Volume, DisplacedTriangle, Curve, SDFGrid)
  - Adding new hit types requires modifying HitInfoType enum
  - Hit type bits limited by header size

- **Bit Allocation Limitations**:
  - Maximum 32 bits for primitive index
  - Maximum 32 bits for type + instanceID
  - Throws if > 64 bits needed for encoding
  - Requires scene to fit within bit allocation constraints

### Performance Limitations

- **Compression Overhead**:
  - Barycentric quantization adds overhead
  - Precision loss may affect shading quality
  - Compression detection adds scene initialization overhead

- **Memory Limitations**:
  - Uncompressed mode uses 2x memory
  - May exceed memory limits for large hit counts
  - No streaming support for hit info

### Integration Limitations

- **Scene Limitations**:
  - Requires scene to be initialized before HitInfo
  - Bit allocation based on scene content
  - Scene changes require HitInfo re-initialization

- **Shader Limitations**:
  - Requires compile-time defines for bit allocation
  - Shader recompilation on scene changes
  - No runtime bit allocation changes

## Best Practices

### Configuration Best Practices

- **Compression Mode Selection**:
  - Use compression for triangle-only scenes
  - Disable compression for mixed geometry scenes
  - Test compression impact on shading quality
  - Profile memory savings vs quality loss

- **Bit Allocation**:
  - Use default bit allocation for most scenes
  - Manually configure bit allocation for edge cases
  - Monitor bit allocation warnings
  - Ensure scene fits within bit constraints

### Performance Optimization

- **Memory Optimization**:
  - Use compression when possible
  - Minimize hit info size
  - Reuse hit info buffers
  - Batch hit info updates

- **GPU Optimization**:
  - Use efficient bit operations
  - Minimize texture fetches
  - Use structured buffers for hit info
  - Coalesce memory accesses

### Debugging

- **Hit Info Validation**:
  - Check hit type validity before access
  - Validate barycentric weights
  - Verify instance ID and primitive index
  - Use hit info visualization tools

- **Error Handling**:
  - Catch bit allocation errors
  - Validate compression support
  - Log hit info statistics
  - Monitor hit info quality

### Development

- **Hit Type Extension**:
  - Add new hit types to HitInfoType enum
  - Update HitInfo.slang with new hit structures
  - Implement packing/unpacking for new types
  - Update bit allocation if needed

- **Compression Extension**:
  - Add support for new geometry types
  - Implement new compression schemes
  - Update compression detection logic
  - Test compression quality impact

### Integration

- **Scene Integration**:
  - Initialize HitInfo after scene loading
  - Reinitialize on scene changes
  - Use scene-based bit allocation
  - Monitor scene geometry types

- **Ray Tracing Integration**:
  - Use hit info in ray tracing payload
  - Pack hit info in ray generation
  - Unpack hit info in shading passes
  - Validate hit info consistency

## Progress Log

- **2026-01-07T23:50:00Z**: HitInfo analysis completed. Analyzed HitInfo.h (72 lines), HitInfo.cpp (124 lines), HitInfo.slang (500 lines), and HitInfoType.slang (52 lines). Documented host-side utility with bit allocation, compression mode support, define generation, and resource format selection. Documented shader-side polymorphic container with multiple hit type structures (TriangleHit, DisplacedTriangleHit, CurveHit, SDFGridHit, VolumeHit), packing/unpacking logic, barycentric computation, and hit type extraction. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices. Marked HitInfo as Complete.

## Completion Status

The HitInfo module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
