# SceneTypes - Scene Type Definitions

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneTypes.slang** - Scene type definitions shader (491 lines)

### External Dependencies

- **Utils/HostDeviceShared.slangh** - Host device shared shader definitions
- **SceneDefines.slangh** - Scene defines
- **Utils/Math/PackedFormats.h** - Packed formats
- **VertexData.slang** - Vertex data definitions
- **Utils/SlangUtils** - Slang utilities
- **Utils/Attributes** - Attributes utilities

## Module Overview

SceneTypes is a comprehensive shader data structure system that provides type definitions and data structures for scene geometry, including geometry instances, meshes, curves, custom primitives, and split buffers. It provides packed data structures for efficient GPU memory usage, supports both HOST_CODE and non-HOST_CODE compilation modes, and includes comprehensive mesh and curve data structures with flags for dynamic/static/displaced/skinned/front-face detection. The system is designed for efficient GPU memory layout with 32-bit packed data and supports both read-only and read-write split buffers for dynamic geometry updates.

## Component Specifications

### GeometryInstanceID Structure

**Purpose**: Geometry instance ID. This uniquely identifies a geometry instance in the scene. All instances are sequentially indexed, with mesh instances first.

**Fields**:
- [`index`](Source/Falcor/Scene/SceneTypes.slang:51) - `uint` - Global instance index. This is computed as InstanceID() + GeometryIndex(). All instances are sequentially indexed, with mesh instances first

**Methods**:
- [`__init(uint instanceID, uint geometryIndex)`](Source/Falcor/Scene/SceneTypes.slang:58) - Construct a geometry instance ID
  - Parameters: instanceID - The DXR InstanceID() system value, geometryIndex - The DXR GeometryIndex() system value
  - Returns: index + geometryIndex

### GeometryType Enumeration

**Purpose**: Geometry types in the scene.

**Values**:
- [`None`](Source/Falcor/Scene/SceneTypes.slang:69) - `GEOMETRY_TYPE_NONE` - No geometry
- [`TriangleMesh`](Source/Falcor/Scene/SceneTypes.slang:70) - `GEOMETRY_TYPE_TRIANGLE_MESH` - Triangle mesh geometry
- [`DisplacedTriangleMesh`](Source/Falcor/Scene/SceneTypes.slang:71) - `GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH` - Displaced triangle mesh geometry
- [`Curve`](Source/Falcor/Scene/SceneTypes.slang:72) - `GEOMETRY_TYPE_CURVE` - Curve geometry
- [`SDFGrid`](Source/Falcor/Scene/SceneTypes.slang:73) - `GEOMETRY_TYPE_SDF_GRID` - SDF grid geometry
- [`Custom`](Source/Falcor/Scene/SceneTypes.slang:74) - `GEOMETRY_TYPE_CUSTOM` - Custom primitive geometry

**Count**:
- [`Count`](Source/Falcor/Scene/SceneTypes.slang:76) - Number of geometry types

### GeometryTypeFlags Enumeration

**Purpose**: Flags indicating what geometry types exist in the scene.

**Values**:
- [`TriangleMesh`](Source/Falcor/Scene/SceneTypes.slang:83) - `(1u << GEOMETRY_TYPE_TRIANGLE_MESH)` - Triangle mesh geometry present
- [`DisplacedTriangleMesh`](Source/Falcor/Scene/SceneTypes.slang:84) - `(1u << GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH)` - Displaced triangle mesh geometry present
- [`Curve`](Source/Falcor/Scene/SceneTypes.slang:85) - `(1u << GEOMETRY_TYPE_CURVE)` - Curve geometry present
- [`SDFGrid`](Source/Falcor/Scene/SceneTypes.slang:86) - `(1u << GEOMETRY_TYPE_SDF_GRID)` - SDF grid geometry present
- [`Custom`](Source/Falcor/Scene/SceneTypes.slang:87) - `(1u << GEOMETRY_TYPE_CUSTOM)` - Custom primitive geometry present

### GeometryInstanceData Structure

**Purpose**: Geometry instance data for GPU.

**Fields**:
- [`kTypeBits`](Source/Falcor/Scene/SceneTypes.slang:108) - `static const uint` - Upper kTypeBits bits are reserved for storing the type
- [`flags`](Source/Falcor/Scene/SceneTypes.slang:111) - `uint` - Upper kTypeBits bits are reserved for storing the type
- [`globalMatrixID`](Source/Falcor/Scene/SceneTypes.slang:112) - `uint` - This is actually CPU-side NodeID, with a confusing name
- [`materialID`](Source/Falcor/Scene/SceneTypes.slang:113) - `uint` - Material ID
- [`geometryID`](Source/Falcor/Scene/SceneTypes.slang:114) - `uint` - Geometry ID
- [`vbOffset`](Source/Falcor/Scene/SceneTypes.slang:116) - `uint` - Offset into vertex buffer
- [`ibOffset`](Source/Falcor/Scene/SceneTypes.slang:117) - `uint` - Offset into index buffer, or zero if non-indexed
- [`instanceIndex`](Source/Falcor/Scene/SceneTypes.slang:119) - `uint` - InstanceIndex in TLAS
- [`geometryIndex`](Source/Falcor/Scene/SceneTypes.slang:120) - `uint` - GeometryIndex in BLAS

**Methods**:
- [`GeometryInstanceData()`](Source/Falcor/Scene/SceneTypes.slang:123) - Default constructor
  - Initializes all fields to zero
- [`GeometryInstanceData(GeometryType type)`](Source/Falcor/Scene/SceneTypes.slang:125) - Constructor with geometry type
  - Parameters: type - Geometry type
  - Initializes: flags((uint32_t)type << kTypeOffset), sets other fields to zero
- [`getType()`](Source/Falcor/Scene/SceneTypes.slang:131) - Get geometry type
  - Returns: Geometry type from flags
- [`isDynamic()`](Source/Falcor/Scene/SceneTypes.slang:135) - Check if dynamic
  - Returns: (flags & (uint)GeometryInstanceFlags::IsDynamic) != 0
- [`isWorldFrontFaceCW()`](Source/Falcor/Scene/SceneTypes.slang:140) - Check if world front face is clockwise
  - Returns: (flags & (uint)GeometryInstanceFlags::IsWorldFrontFaceCW) != 0

### MeshFlags Enumeration

**Purpose**: Mesh flags for geometry instances.

**Values**:
- [`Use16BitIndices`](Source/Falcor/Scene/SceneTypes.slang:99) - `0x1` - Indices are in 16-bit format. The default is 32-bit
- [`IsDynamic`](Source/Falcor/Scene/SceneTypes.slang:100) - `0x2` - Mesh is dynamic, either through skinning or vertex animations
- [`IsFrontFaceCW`](Source/Falcor/Scene/SceneTypes.slang:101) - `0x4` - Front-facing side has clockwise winding in object space. Note that the winding in world space may be flipped due to the instance transform
- [`IsWorldFrontFaceCW`](Source/Falcor/Scene/SceneTypes.slang:103) - `0x10` - Front-facing side has clockwise winding in world space. This is a combination of the mesh winding and instance transform handedness
- [`IsDisplaced`](Source/Falcor/Scene/SceneTypes.slang:107) - `0x8` - Mesh has displacement map

### MeshDesc Structure

**Purpose**: Mesh data stored in 32B.

**Fields**:
- [`vbOffset`](Source/Falcor/Scene/SceneTypes.slang:160) - `uint` - Offset into global vertex buffer
- [`ibOffset`](Source/Falcor/Scene/SceneTypes.slang:161) - `uint` - Offset into global index buffer, or zero if non-indexed
- [`vertexCount`](Source/Falcor/Scene/SceneTypes.slang:162) - `uint` - Vertex count
- [`indexCount`](Source/Falcor/Scene/SceneTypes.slang:163) - `uint` - Index count, or zero if non-indexed
- [`skinningVbOffset`](Source/Falcor/Scene/SceneTypes.slang:164) - `uint` - Offset into skinning data buffer, or zero if no skinning data
- [`prevVbOffset`](Source/Falcor/Scene/SceneTypes.slang:165) - `uint` - Offset into previous vertex data buffer, or zero if neither skinned nor animated
- [`materialID`](Source/Falcor/Scene/SceneTypes.slang:166) - `uint` - Material ID
- [`flags`](Source/Falcor/Scene/SceneTypes.slang:167) - `uint` - See MeshFlags

**Methods**:
- [`getVertexCount()`](Source/Falcor/Scene/SceneTypes.slang:169) - Get vertex count
  - Returns: vertexCount
- [`getTriangleCount()`](Source/Falcor/Scene/SceneTypes.slang:174) - Get triangle count
  - Returns: (indexCount > 0 ? indexCount : vertexCount) / 3
- [`use16BitIndices()`](Source/Falcor/Scene/SceneTypes.slang:179) - Check if using 16-bit indices
  - Returns: (flags & (uint)MeshFlags::Use16BitIndices) != 0
- [`useVertexIndices()`](Source/Falcor/Scene/SceneTypes.slang:184) - Check if using vertex indices
  - Returns: indexCount > 0
- [`isSkinned()`](Source/Falcor/Scene/SceneTypes.slang:189) - Check if skinned
  - Returns: (flags & (uint)MeshFlags::IsSkinned) != 0
- [`isAnimated()`](Source/Falcor/Scene/SceneTypes.slang:194) - Check if animated
  - Returns: isSkinned() || isAnimated()
- [`isFrontFaceCW()`](Source/Falcor/Scene/SceneTypes.slang:199) - Check if front face is clockwise
  - Returns: (flags & (uint)MeshFlags::IsFrontFaceCW) != 0
- [`isDisplaced()`](Source/Falcor/Scene/SceneTypes.slang:204) - Check if displaced
  - Returns: (flags & (uint)MeshFlags::IsDisplaced) != 0

### StaticVertexData Structure

**Purpose**: Static vertex data for meshes.

**Fields**:
- [`position`](Source/Falcor/Scene/SceneTypes.slang:217) - `float3` - Position
- [`normal`](Source/Falcor/Scene/SceneTypes.slang:218) - `float3` - Shading normal
- [`tangent`](Source/Falcor/Scene/SceneTypes.slang:219) - `float4` - Shading tangent. The bitangent is computed: cross(normal, tangent.xyz) * tangent.w. NOTE: The tangent is *only* valid when tangent.w != 0
- [`texCrd`](Source/Falcor/Scene/SceneTypes.slang:220) - `float2` - Texture coordinates
- [`curveRadius`](Source/Falcor/Scene/SceneTypes.slang:221) - `float` - Curve cross-sectional radius. Valid only for geometry generated from curves

### PackedStaticVertexData Structure

**Purpose**: Packed static vertex data for 32B aligned access.

**Fields**:
- [`position`](Source/Falcor/Scene/SceneTypes.slang:228) - `float3` - Position
- [`packedNormalTangentCurveRadius`](Source/Falcor/Scene/SceneTypes.slang:229) - `float3` - Packed normal, tangent, sign, and curve radius
- [`texCrd`](Source/Falcor/Scene/SceneTypes.slang:230) - `float2` - Texture coordinates

**Packing Logic**:
- Packs normal, tangent, sign, and curve radius into 32-bit components
- Uses f32tof16 for normal components
- Stores sign and curve radius in tangent.w
- Handles curve radius validation

### PrevVertexData Structure

**Purpose**: Previous frame vertex data for motion vectors.

**Fields**:
- [`position`](Source/Falcor/Scene/SceneTypes.slang:299) - `float3` - Position

### SkinningVertexData Structure

**Purpose**: Skinning vertex data for animated meshes.

**Fields**:
- [`boneID`](Source/Falcor/Scene/SceneTypes.slang:304) - `uint4` - Bone ID
- [`boneWeight`](Source/Falcor/Scene/SceneTypes.slang:305) - `float4` - Bone weight
- [`staticIndex`](Source/Falcor/Scene/SceneTypes.slang:306) - `uint` - The index in the static vertex buffer
- [`bindMatrixID`](Source/Falcor/Scene/SceneTypes.slang:307) - `uint` - Skeleton matrix ID
- [`skeletonMatrixID`](Source/Falcor/Scene/SceneTypes.slang:308) - `uint` - Skeleton matrix ID

### CurveDesc Structure

**Purpose**: Curve descriptor.

**Fields**:
- [`vbOffset`](Source/Falcor/Scene/SceneTypes.slang:313) - `uint` - Offset into global curve vertex buffer
- [`ibOffset`](Source/Falcor/Scene/SceneTypes.slang:314) - `uint` - Offset into global curve index buffer
- [`vertexCount`](Source/Falcor/Scene/SceneTypes.slang:315) - `uint` - Vertex count
- [`indexCount`](Source/Falcor/Scene/SceneTypes.slang:316) - `uint` - Index count
- [`degree`](Source/Falcor/Scene/SceneTypes.slang:317) - `uint` - Polynomial degree of curve; linear (1) by default
- [`materialID`](Source/Falcor/Scene/SceneTypes.slang:318) - `uint` - Material ID

**Methods**:
- [`getSegmentCount()`](Source/Falcor/Scene/SceneTypes.slang:320) - Get segment count
  - Returns: indexCount

### StaticCurveVertexData Structure

**Purpose**: Static curve vertex data.

**Fields**:
- [`position`](Source/Falcor/Scene/SceneTypes.slang:329) - `float3` - Position
- [`radius`](Source/Falcor/Scene/SceneTypes.slang:330) - `float` - Radius of sphere at curve ends
- [`texCrd`](Source/Falcor/Scene/SceneTypes.slang:331) - `float2` - Texture coordinates

### DynamicCurveVertexData Structure

**Purpose**: Dynamic curve vertex data for animated curves.

**Fields**:
- [`position`](Source/Falcor/Scene/SceneTypes.slang:335) - `float3` - Position

### CustomPrimitiveDesc Structure

**Purpose**: Custom primitive data.

**Fields**:
- [`userID`](Source/Falcor/Scene/SceneTypes.slang:343) - `uint` - User-defined ID that is specified during scene creation. This can be used to identify different sub-types of custom primitives
- [`aabbOffset`](Source/Falcor/Scene/SceneTypes.slang:344) - `uint` - Offset into list of procedural primitive AABBs

**Note**: The custom primitives are currently mapped 1:1 to a list of custom primitive AABBs.

### SplitIndexBuffer Structure

**Purpose**: GPU representation for SplitBuffer<uint32_T>.

**Fields**:
- [`data`](Source/Falcor/Scene/SceneTypes.slang:367) - `ByteAddressBuffer data[kBufferCount]` - GPU representation
- `kBufferIndexBits`](Source/Falcor/Scene/SceneTypes.slang:369) - `static constexpr uint` - log2(sizeof(ElementType))
- `kBufferIndexOffset`](Source/Falcor/Scene/SceneTypes.slang:370) - `static constexpr uint` - 32 - kBufferIndexBits
- `kElementIndexMask`](Source/Falcor/Scene/SceneTypes.slang:371) - `static constexpr uint` - (1u << kBufferIndexOffset) - 1

**Methods**:
- [`Load16b(uint baseOffset, uint triangleIndex)`](Source/Falcor/Scene/SceneTypes.slang:389) - Load 16-bit indices
  - Parameters: baseOffset - offset to start of mesh, triangleIndex - triangle index
  - Returns: data[0].Load<uint16_t3>(baseOffset * 4 + triangleIndex * 6)
  - Logic: bufferIndex = baseOffset >> kBufferIndexOffset, byteOffset = (baseOffset & kElementIndexMask) * 4

- [`Load32b(uint baseOffset, uint triangleIndex)`](Source/Falcor/Scene/SceneTypes.slang:398) - Load 32-bit indices
  - Parameters: baseOffset - offset to start of mesh, triangleIndex - triangle index
  - Returns: data[0].Load3(baseOffset * 4 + triangleIndex * 12)
  - Logic: bufferIndex = baseOffset >> kBufferIndexOffset, byteOffset = (baseOffset & kElementIndexMask) * 4

### SplitVertexBuffer Structure

**Purpose**: GPU representation for SplitBuffer<PackedStaticVertexData>.

**Fields**:
- [`data`](Source/Falcor/Scene/SceneTypes.slang:433) - `StructuredBuffer<ElementType> data[ArrayMax<1, kBufferCount>.value]` - GPU representation
- `kBufferIndexBits`](Source/Falcor/Scene/SceneTypes.slang:425) - `static constexpr uint` - log2(sizeof(ElementType))
- `kBufferIndexOffset`](Source/Falcor/Scene/SceneTypes.slang:426) - `static constexpr uint` - 32 - kBufferIndexBits
- `kElementIndexMask`](Source/Falcor/Scene/SceneTypes.slang:427) - `static constexpr uint` - (1u << kBufferIndexOffset) - 1

**Methods**:
- [`__subscript(uint index)`](Source/Falcor/Scene/SceneTypes.slang:465) - Get element at index
  - Returns: data[0][index]
- [`get()`](Source/Falcor/Scene/SceneTypes.slang:466) - Get element at index
  - Returns: data[0][index]
- [`set(ElementType newValue)`](Source/Falcor/Scene/SceneTypes.slang:476) - Set element at index
  - Parameters: newValue - New element value
  - Logic: bufferIndex = index >> kBufferIndexOffset, elementIndex = index & kElementIndexMask, data[bufferIndex][elementIndex] = newValue

### RWSplitVertexBuffer Structure

**Purpose**: Read-write split vertex buffer.

**Fields**:
- [`data`](Source/Falcor/Scene/SceneTypes.slang:459) - `RWStructuredBuffer<ElementType> data[ArrayMax<1, kBufferCount>.value]` - GPU representation
- `kBufferIndexBits`](Source/Falcor/Scene/SceneTypes.slang:451) - `static constexpr uint` - log2(sizeof(ElementType))
- `kBufferIndexOffset`](Source/Falcor/Scene/SceneTypes.slang:452) - `static constexpr uint` - 32 - kBufferIndexBits
- `kElementIndexMask`](Source/Falcor/Scene/SceneTypes.slang:453) - `static constexpr uint` - (1u << kBufferIndexOffset) - 1

**Methods**:
- [`__subscript(uint index)`](Source/Falcor/Scene/SceneTypes.slang:465) - Get element at index
  - Returns: data[0][index]
- [`get()`](Source/Falcor/Scene/SceneTypes.slang:466) - Get element at index
  - Returns: data[0][index]
- [`set(ElementType newValue)`](Source/Falcor/Scene/SceneTypes.slang:475) - Set element at index
  - Parameters: newValue - New element value
  - Logic: bufferIndex = index >> kBufferIndexOffset, elementIndex = index & kElementIndexMask, data[bufferIndex][elementIndex] = newValue

## Technical Details

### Geometry Instance ID

**ID Composition**:
- InstanceID + GeometryIndex
- Sequential indexing with mesh instances first
- Unique identifier per instance

**Instance Indexing**:
- Mesh instances first, then curves, then SDF grids, then custom primitives
- Used for TLAS indexing
- Used for geometry instance data

### Geometry Types

**Type System**:
- Enum-based geometry types
- Bit field for presence checking
- Compile-time type detection

**Geometry Type Values**:
- None (0) - No geometry
- TriangleMesh (1) - Triangle mesh geometry
- DisplacedTriangleMesh (2) - Displaced triangle mesh geometry
- Curve (3) - Curve geometry
- SDFGrid (5) - SDF grid geometry
- Custom (6) - Custom primitive geometry

### Geometry Type Flags

**Bit Field Layout**:
- Bit 0: TriangleMesh (1 << 0)
- Bit 1: DisplacedTriangleMesh (1 << 1)
- Bit 2: Curve (1 << 2)
- Bit 3: Unused
- Bit 4: Unused
- Bit 5: SDFGrid (1 << 3)
- Bit 6: Custom (1 << 4)

**Flag Usage**:
- Check if specific geometry types are present
- Enable conditional compilation
- Support for multiple geometry types

### Mesh Flags

**Flag Values**:
- Use16BitIndices (0x1) - 16-bit index format
- IsDynamic (0x2) - Dynamic mesh (skinned or animated)
- IsFrontFaceCW (0x4) - Clockwise winding in object space
- IsWorldFrontFaceCW (0x10) - Clockwise winding in world space
- IsDisplaced (0x8) - Has displacement map

**Flag Usage**:
- Control mesh properties
- Enable conditional compilation
- Support for different mesh types

### Geometry Instance Data

**GPU Layout**:
- 32-bit packed data
- Efficient GPU memory usage
- Support for multiple geometry types

**Offset System**:
- Vertex buffer offset
- Index buffer offset
- Support for split buffers

**Indexing System**:
- Instance index for TLAS
- Geometry index for BLAS
- Sequential instance indexing

### Mesh Data

**Vertex Data**:
- Static vertex data with position, normal, tangent, texture coordinates, curve radius
- Packed format for 32B aligned access
- Efficient GPU memory layout

**Mesh Properties**:
- Vertex count and index count
- Material ID
- Mesh flags for dynamic/static/displaced/skinned/front-face detection
- Support for skinned meshes with bone IDs and weights
- Support for displaced meshes
- Support for animated meshes

### Curve Data

**Vertex Data**:
- Static curve vertex data with position, radius, texture coordinates
- Dynamic curve vertex data for animated curves

**Curve Properties**:
- Vertex count and index count
- Polynomial degree (linear by default)
- Material ID
- Segment count (index count)

### Custom Primitive Data

**User ID System**:
- User-defined ID for sub-type identification
- Mapped 1:1 to AABB list

**AABB System**:
- Offset into procedural primitive AABB list
- Support for custom primitive bounds

### Split Buffer System

**Byte Address Buffer**:
- GPU representation for SplitBuffer<uint32_T>
- Unlike SplitVertexBuffer which wraps StructuredBuffer
- Explicit instantiation required
- Functions as adapter for larger buffers

**Index Buffer**:
- 16-bit or 32-bit index loading
- Support for triangle index access
- Efficient buffer indexing

**Vertex Buffer**:
- StructuredBuffer for read-write access
- Support for dynamic geometry updates
- Efficient element access

## Integration Points

### Scene Integration

**Geometry Instance ID**:
- Used for TLAS indexing
- Unique identifier per instance
- Supports instance inclusion mask

**Geometry Type System**:
- Compile-time geometry type checking
- Runtime geometry type detection
- Support for multiple geometry types

### Mesh System

**Mesh Data**:
- 32B packed vertex data
- Efficient GPU memory layout
- Support for skinned and animated meshes
- Support for displaced meshes

### Curve System

**Curve Data**:
- Static and dynamic curve vertex data
- Support for animated curves
- Efficient GPU memory layout

### Custom Primitive System

**Custom Primitive Data**:
- User-defined IDs for sub-type identification
- AABB list for bounds
- Support for custom primitive types

### Split Buffer System

**GPU Representation**:
- ByteAddressBuffer for index buffers
- StructuredBuffer for vertex buffers
- RWStructuredBuffer for dynamic vertex buffers
- Efficient GPU memory usage
- Support for both HOST_CODE and non-HOST_CODE

## Architecture Patterns

### Packed Data Pattern

- 32-bit packed data structures
- Efficient GPU memory usage
- Aligned access patterns
- Bit packing for normal, tangent, sign, curve radius

### Split Buffer Pattern

- Template-based split buffer system
- Support for both ByteAddressBuffer and StructuredBuffer
- Adapter functions for larger buffers
- Efficient buffer indexing

### GPU Memory Layout

- 32B aligned data
- Offset-based indexing
- Support for multiple buffer types
- Efficient GPU memory usage

## Code Patterns

### Geometry Instance ID Pattern

```slang
struct GeometryInstanceID
{
    uint index;
    
    __init(uint instanceID, uint geometryIndex)
    {
        index = instanceID + geometryIndex;
    }
}
```

### Mesh Data Pattern

```slang
struct MeshDesc
{
    uint vbOffset;
    uint ibOffset;
    uint vertexCount;
    uint indexCount;
    uint skinningVbOffset;
    uint prevVbOffset;
    uint materialID;
    uint flags;
    
    uint getVertexCount()
    {
        return vertexCount;
    }
    
    uint getTriangleCount()
    {
        return (indexCount > 0 ? indexCount : vertexCount) / 3;
    }
    
    bool use16BitIndices()
    {
        return (flags & (uint)MeshFlags::Use16BitIndices) != 0;
    }
    
    bool useVertexIndices()
    {
        return indexCount > 0;
    }
    
    bool isSkinned()
    {
        return (flags & (uint)MeshFlags::IsSkinned) != 0;
    }
    
    bool isAnimated()
    {
        return isSkinned() || isAnimated();
    }
    
    bool isFrontFaceCW()
    {
        return (flags & (uint)MeshFlags::IsFrontFaceCW) != 0;
    }
    
    bool isDisplaced()
    {
        return (flags & (uint)MeshFlags::IsDisplaced) != 0;
    }
}
```

### Packed Vertex Data Pattern

```slang
struct PackedStaticVertexData
{
    float3 position;
    float3 packedNormalTangentCurveRadius;
    float2 texCrd;
}
```

### Split Buffer Pattern

```slang
struct SplitIndexBuffer
{
    ByteAddressBuffer data[kBufferCount];
    
    uint16_t3 Load16b(uint baseOffset, uint triangleIndex)
    {
        if (kBufferCount == 1)
            return data[0].Load<uint16_t3>(baseOffset * 4 + triangleIndex * 6);
        uint bufferIndex = baseOffset >> kBufferIndexOffset;
        uint byteOffset = (baseOffset & kElementIndexMask) * 4;
        return data[bufferIndex].Load<uint16_t3>(byteOffset + triangleIndex * 6);
    }
}
```

## Use Cases

### Geometry Instance Management

- **Instance ID Generation**:
  - Generate unique instance IDs
  - Combine instance ID and geometry index
  - Support for sequential indexing

- **Geometry Type Detection**:
  - Check which geometry types are present
  - Enable conditional compilation
  - Support for multiple geometry types

### Mesh Management

- **Mesh Data Storage**:
  - Store mesh vertex and index data
  - Support for skinned meshes
  - Support for displaced meshes
  - Support for animated meshes

- **Mesh Properties**:
  - Track mesh flags
  - Check mesh properties
  - Support for different mesh types

### Curve Management

- **Curve Data Storage**:
  - Store curve vertex and index data
  - Support for static and dynamic curves
  - Support for different curve degrees

- **Curve Properties**:
  - Track curve degree
  - Support for segment count calculation
  - Support for material assignment

### Custom Primitive Management

- **Custom Primitive Storage**:
  - Store custom primitive AABBs
  - Support for user-defined IDs
  - Support for sub-type identification

### Split Buffer Management

- **Index Buffer**:
  - Load 16-bit or 32-bit indices
  - Support for triangle index access
  - Efficient buffer indexing

- **Vertex Buffer**:
  - Read-write access for dynamic geometry
  - Support for animated meshes and curves
  - Efficient element access

## Performance Considerations

### GPU Memory Usage

**Packed Data**:
- 32-bit packed data structures
- Efficient GPU memory usage
- Aligned access patterns
- Bit packing for multiple attributes

**Split Buffers**:
- ByteAddressBuffer for efficient indexing
- StructuredBuffer for vertex data
- RWStructuredBuffer for dynamic updates
- Support for multiple buffer types

**Memory Efficiency**:
- Minimal padding
- Efficient data packing
- Offset-based indexing
- Shared buffer usage

### Compilation Performance

**HOST_CODE vs Non-HOST_CODE**:
- Conditional compilation based on HOST_CODE define
- Different implementations for packed data
- Support for both scenarios

### GPU Performance

**Indexing Performance**:
- Efficient buffer indexing
- Offset-based addressing
- Minimal buffer access overhead

## Limitations

### Feature Limitations

- **Geometry Types**:
- Limited to 7 predefined types
- Cannot add custom geometry types without modifying system
- Bit field limited to 32 geometry types

- **Mesh Flags**:
- Limited to predefined flags
- Cannot add custom mesh flags without modifying system
- Bit field limited to 32 flags

- **Instance Indexing**:
- Sequential indexing only
- Cannot support arbitrary instance ordering
- Mesh instances must be first

- **Split Buffers**:
- Limited to 1-2 buffer count (SCENE_VERTEX_BUFFER_COUNT)
- Cannot support more than 2 buffers without modification
- May need expansion for very large scenes

### Performance Limitations

- **Packed Data**:
- 32-bit alignment may waste space on some platforms
- Bit packing may reduce precision
- Limited to 32-bit components

- **Buffer Indexing**:
- Offset-based indexing may be complex
- May have overhead for buffer selection
- Limited to 1-2 buffer count

### Integration Limitations

- **Scene Coupling**:
- Tightly coupled to scene system
- Requires specific data structures
- Not suitable for standalone use

## Best Practices

### Geometry Instance Management

- **Instance ID Usage**:
  - Use GeometryInstanceID for unique identification
  - Combine instance ID and geometry index
  - Use sequential indexing

- **Geometry Type Detection**:
  - Use GeometryTypeFlags for presence checking
  - Enable conditional compilation
  - Support for multiple geometry types

### Mesh Management

- **Mesh Data Storage**:
  - Use packed data structures for efficiency
  - Support for skinned meshes
  - Support for displaced meshes
  - Use mesh flags for property checking

- **Mesh Properties**:
  - Use mesh flags for conditional compilation
  - Check mesh properties before use
  - Support for different mesh types

### Curve Management

- **Curve Data Storage**:
  - Use static and dynamic curve vertex data
  - Support for different curve degrees
  - Track curve degree and segment count

### Custom Primitive Management

- **Custom Primitive Storage**:
  - Use user-defined IDs for sub-type identification
  - Store custom primitive AABBs
  - Support for sub-type identification

### Split Buffer Management

- **Buffer Selection**:
  - Use appropriate buffer type for use case
  - ByteAddressBuffer for indices
  - StructuredBuffer for vertices
  - RWStructuredBuffer for dynamic updates

- **Memory Efficiency**:
  - Use packed data structures
  - Minimize padding
  - Use offset-based indexing
  - Share buffers when possible

## Progress Log

- **2026-01-08T00:44:00Z**: SceneTypes analysis completed. Analyzed SceneTypes.slang (491 lines) containing comprehensive shader data structure system. Documented GeometryInstanceID structure with index field and __init method, GeometryType enumeration with 6 values (None, TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid, Custom) and Count field, GeometryTypeFlags enumeration with 5 values (TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid, Custom) and bit field layout, GeometryInstanceData structure with kTypeBits, flags, globalMatrixID, materialID, geometryID, vbOffset, ibOffset, instanceIndex, geometryIndex fields and methods (GeometryInstanceData, getType, isDynamic, isWorldFrontFaceCW), MeshFlags enumeration with 5 values (Use16BitIndices, IsDynamic, IsFrontFaceCW, IsWorldFrontFaceCW, IsDisplaced), MeshDesc structure with vbOffset, ibOffset, vertexCount, indexCount, skinningVbOffset, prevVbOffset, materialID, flags fields and methods (getVertexCount, getTriangleCount, use16BitIndices, useVertexIndices, isSkinned, isAnimated, isFrontFaceCW, isDisplaced), StaticVertexData structure with position, normal, tangent, texCrd, curveRadius fields, PackedStaticVertexData structure with position, packedNormalTangentCurveRadius, texCrd fields and packing logic, PrevVertexData structure with position field, SkinningVertexData structure with boneID, boneWeight, staticIndex, bindMatrixID, skeletonMatrixID fields, CurveDesc structure with vbOffset, ibOffset, vertexCount, indexCount, degree, materialID fields and getSegmentCount method, StaticCurveVertexData structure with position, radius, texCrd fields, DynamicCurveVertexData structure with position field, CustomPrimitiveDesc structure with userID, aabbOffset fields and comment about custom primitives being mapped 1:1 to AABB list, SplitIndexBuffer structure with data array, kBufferIndexBits, kBufferIndexOffset, kElementIndexMask, kBufferCount fields and Load16b, Load32b methods, SplitVertexBuffer structure with data array, kBufferIndexBits, kBufferIndexOffset, kElementIndexMask, kBufferCount fields and __subscript, get, set methods, RWSplitVertexBuffer structure with data array, kBufferIndexBits, kBufferIndexOffset, kElementIndexMask, kBufferCount fields and __subscript, get, set methods, HOST_CODE and non-HOST_CODE conditional compilation. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The SceneTypes module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
