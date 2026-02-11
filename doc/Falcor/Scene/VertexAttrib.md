# VertexAttrib, SceneTypes, SceneIDs - Scene Type Definitions

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **VertexAttrib** - Vertex attribute definitions
- [x] **SceneTypes** - Scene type definitions
- [x] **SceneIDs** - Scene ID type definitions

### External Dependencies

- **Utils/Math** - Mathematical utilities (PackedFormats)
- **Utils/SlangUtils** - Slang utilities
- **Utils/Attributes** - Attribute utilities
- **Utils/ObjectID** - Object ID system

## Module Overview

These systems provide comprehensive type definitions for scene vertices, geometry types, and scene object IDs. They define the data structures used throughout the scene system for efficient GPU representation and CPU-side management.

## Component Specifications

### VertexAttrib

**File**: [`VertexAttrib.slangh`](Source/Falcor/Scene/VertexAttrib.slangh:1)

**Purpose**: Vertex attribute definitions for scene rendering.

**Key Features**:
- Vertex location definitions
- Packed vertex data structures
- Curve vertex attributes
- Instance draw ID support

**Vertex Locations** (from VertexAttrib.slangh):
- **VERTEX_POSITION_LOC** (0) - Position location
- **VERTEX_PACKED_NORMAL_TANGENT_CURVE_RADIUS_LOC** (1) - Packed normal, tangent, curve radius location
- **VERTEX_TEXCOORD_LOC** (2) - Texture coordinate location
- **INSTANCE_DRAW_ID_LOC** (3) - Instance draw ID location
- **VERTEX_LOCATION_COUNT** (4) - Total vertex location count
- **VERTEX_USER_ELEM_COUNT** (4) - User element count
- **VERTEX_USER0_LOC** (VERTEX_LOCATION_COUNT) - User element 0 location

**Vertex Location Names**:
- **VERTEX_POSITION_NAME** - "POSITION"
- **VERTEX_PACKED_NORMAL_TANGENT_CURVE_RADIUS_NAME** - "PACKED_NORMAL_TANGENT_CURVE_RADIUS"
- **VERTEX_TEXCOORD_NAME** - "TEXCOORD"
- **INSTANCE_DRAW_ID_NAME** - "DRAW_ID"

**Curve Vertex Locations**:
- **CURVE_VERTEX_POSITION_LOC** (0) - Curve position location
- **CURVE_VERTEX_RADIUS_LOC** (1) - Curve radius location
- **CURVE_VERTEX_TEXCOORD_LOC** (2) - Curve texture coordinate location
- **CURVE_VERTEX_LOCATION_COUNT** (3) - Curve vertex location count

**Curve Vertex Names**:
- **CURVE_VERTEX_POSITION_NAME** - "POSITION"
- **CURVE_VERTEX_RADIUS_NAME** - "RADIUS"
- **CURVE_VERTEX_TEXCOORD_NAME** - "TEXCOORD"

### SceneTypes

**File**: [`SceneTypes.slang`](Source/Falcor/Scene/SceneTypes.slang:1)

**Purpose**: Scene type definitions for geometry and instances.

**Key Features**:
- Geometry instance ID system
- Geometry type enumeration
- Geometry instance flags
- Mesh data structures
- Curve data structures
- Custom primitive data structures
- Split buffer support for large data

**GeometryInstanceID** (from [`GeometryInstanceID`](Source/Falcor/Scene/SceneTypes.slang:49)):
- **Purpose**: Uniquely identifies a geometry instance in scene
- **index** - Global instance index (InstanceID() + GeometryIndex())
- **Construction**: `__init(uint instanceID, uint geometryIndex)`

**GeometryType** (from [`GeometryType`](Source/Falcor/Scene/SceneTypes.slang:67)):
- **None** (0) - No geometry
- **TriangleMesh** (1) - Triangle mesh geometry
- **DisplacedTriangleMesh** (2) - Displaced triangle mesh
- **Curve** (3) - Curve geometry
- **SDFGrid** (4) - SDF grid geometry
- **Custom** (5) - Custom primitive geometry
- **Count** - Total geometry type count

**GeometryTypeFlags** (from [`GeometryTypeFlags`](Source/Falcor/Scene/SceneTypes.slang:81)):
- **TriangleMesh** - (1u << GEOMETRY_TYPE_TRIANGLE_MESH)
- **DisplacedTriangleMesh** - (1u << GEOMETRY_TYPE_DISPLACED_TRIANGLE_MESH)
- **Curve** - (1u << GEOMETRY_TYPE_CURVE)
- **SDFGrid** - (1u << GEOMETRY_TYPE_SDF_GRID)
- **Custom** - (1u << GEOMETRY_TYPE_CUSTOM)

**GeometryInstanceFlags** (from [`GeometryInstanceFlags`](Source/Falcor/Scene/SceneTypes.slang:94)):
- **None** (0x0) - No flags
- **Use16BitIndices** (0x1) - Indices are in 16-bit format (default: 32-bit)
- **IsDynamic** (0x2) - Mesh is dynamic (skinning or vertex animations)
- **TransformFlipped** (0x4) - Instance transform flips coordinate system handedness
- **IsObjectFrontFaceCW** (0x8) - Front-facing side has clockwise winding in object space
- **IsWorldFrontFaceCW** (0x10) - Front-facing side has clockwise winding in world space

**GeometryInstanceData** (from [`GeometryInstanceData`](Source/Falcor/Scene/SceneTypes.slang:106)):
- **flags** - Upper kTypeBits bits reserved for storing type (uint)
- **globalMatrixID** - CPU-side NodeID (confusing name)
- **materialID** - Material ID (uint)
- **geometryID** - Geometry ID (uint)
- **vbOffset** - Offset into vertex buffer (uint)
- **ibOffset** - Offset into index buffer, or zero if non-indexed (uint)
- **instanceIndex** - InstanceIndex in TLAS (uint)
- **geometryIndex** - GeometryIndex in BLAS (uint)
- **kTypeBits** (3) - Number of type bits
- **kTypeOffset** (32 - kTypeBits) - Type offset in flags
- **Construction**: `GeometryInstanceData(GeometryType type) : flags((uint32_t)type << kTypeOffset)`

**GeometryInstanceData Methods**:
- [`getType()`](Source/Falcor/Scene/SceneTypes.slang:130) - Get geometry type from flags
- [`isDynamic()`](Source/Falcor/Scene/SceneTypes.slang:135) - Check if geometry is dynamic
- [`isWorldFrontFaceCW()`](Source/Falcor/Scene/SceneTypes.slang:140) - Check if world front face is clockwise

**MeshFlags** (from [`MeshFlags`](Source/Falcor/Scene/SceneTypes.slang:146)):
- **None** (0x0) - No flags
- **Use16BitIndices** (0x1) - Indices are in 16-bit format (default: 32-bit)
- **IsSkinned** (0x2) - Mesh is skinned with vertex data
- **IsFrontFaceCW** (0x4) - Front-facing side has clockwise winding in object space
- **IsDisplaced** (0x8) - Mesh has displacement map
- **IsAnimated** (0x10) - Mesh is affected by vertex-animations

**MeshDesc** (from [`MeshDesc`](Source/Falcor/Scene/SceneTypes.slang:158)):
- **vbOffset** - Offset into global vertex buffer (uint)
- **ibOffset** - Offset into global index buffer, or zero if non-indexed (uint)
- **vertexCount** - Vertex count (uint)
- **indexCount** - Index count, or zero if non-indexed (uint)
- **skinningVbOffset** - Offset into skinning data buffer, or zero if no skinning data (uint)
- **prevVbOffset** - Offset into previous vertex data buffer, or zero if neither skinned nor animated (uint)
- **materialID** - Material ID (uint)
- **flags** - See MeshFlags (uint)

**MeshDesc Methods**:
- [`getVertexCount()`](Source/Falcor/Scene/SceneTypes.slang:169) - Get vertex count
- [`getTriangleCount()`](Source/Falcor/Scene/SceneTypes.slang:174) - Get triangle count (indexCount > 0 ? indexCount : vertexCount) / 3
- [`use16BitIndices()`](Source/Falcor/Scene/SceneTypes.slang:179) - Check if using 16-bit indices
- [`useVertexIndices()`](Source/Falcor/Scene/SceneTypes.slang:184) - Check if using vertex indices (indexCount > 0)
- [`isSkinned()`](Source/Falcor/Scene/SceneTypes.slang:189) - Check if mesh is skinned
- [`isAnimated()`](Source/Falcor/Scene/SceneTypes.slang:194) - Check if mesh is animated
- [`isDynamic()`](Source/Falcor/Scene/SceneTypes.slang:199) - Check if mesh is dynamic (isSkinned() || isAnimated())
- [`isFrontFaceCW()`](Source/Falcor/Scene/SceneTypes.slang:204) - Check if front face is clockwise
- [`isDisplaced()`](Source/Falcor/Scene/SceneTypes.slang:209) - Check if mesh has displacement

**StaticVertexData** (from [`StaticVertexData`](Source/Falcor/Scene/SceneTypes.slang:215)):
- **position** - Position (float3)
- **normal** - Shading normal (float3)
- **tangent** - Shading tangent (float4)
  - Bitangent computed: cross(normal, tangent.xyz) * tangent.w
  - Note: Tangent is only valid when tangent.w != 0
- **texCoord** - Texture coordinates (float2)
- **curveRadius** - Curve cross-sectional radius (valid only for curves)

**PackedStaticVertexData** (from [`PackedStaticVertexData`](Source/Falcor/Scene/SceneTypes.slang:226)):
- **position** - Position (float3)
- **packedNormalTangentCurveRadius** - Packed normal, tangent, curve radius (float3)
- **texCoord** - Texture coordinates (float2)

**PackedStaticVertexData Packing**:
- Normal packed into 16-bit components
- Tangent sign and curve radius packed
- Efficient 32B representation
- Supports normal, tangent, and curve radius

**PackedStaticVertexData Methods**:
- [`pack()`](Source/Falcor/Scene/SceneTypes.slang:235) - Pack StaticVertexData into PackedStaticVertexData
- [`unpack()`](Source/Falcor/Scene/SceneTypes.slang:276) - Unpack PackedStaticVertexData into StaticVertexData

**PrevVertexData** (from [`PrevVertexData`](Source/Falcor/Scene/SceneTypes.slang:297)):
- **position** - Position (float3)

**SkinningVertexData** (from [`SkinningVertexData`](Source/Falcor/Scene/SceneTypes.slang:302)):
- **boneID** - Bone ID (uint4)
- **boneWeight** - Bone weight (float4)
- **staticIndex** - Index in static vertex buffer (uint)
- **bindMatrixID** - Bind matrix ID (uint)
- **skeletonMatrixID** - Skeleton matrix ID (uint)

**CurveDesc** (from [`CurveDesc`](Source/Falcor/Scene/SceneTypes.slang:311)):
- **vbOffset** - Offset into global curve vertex buffer (uint)
- **ibOffset** - Offset into global curve index buffer (uint)
- **vertexCount** - Vertex count (uint)
- **indexCount** - Index count (uint)
- **degree** - Polynomial degree of curve (uint)
  - Linear (1) by default
- **materialID** - Material ID (uint)

**CurveDesc Methods**:
- [`getSegmentCount()`](Source/Falcor/Scene/SceneTypes.slang:320) - Get segment count (indexCount)

**StaticCurveVertexData** (from [`StaticCurveVertexData`](Source/Falcor/Scene/SceneTypes.slang:326)):
- **position** - Position (float3)
- **radius** - Radius of sphere at curve ends (float)
- **texCoord** - Texture coordinates (float2)

**DynamicCurveVertexData** (from [`DynamicCurveVertexData`](Source/Falcor/Scene/SceneTypes.slang:333)):
- **position** - Position (float3)

**CustomPrimitiveDesc** (from [`CustomPrimitiveDesc`](Source/Falcor/Scene/SceneTypes.slang:339)):
- **userID** - User-defined ID for identifying different sub-types (uint)
- **aabbOffset** - Offset into list of custom primitive AABBs (uint)

**SplitIndexBuffer** (from [`SplitIndexBuffer`](Source/Falcor/Scene/SceneTypes.slang:366)):
- **Purpose**: GPU representation for SplitBuffer<uint32_T>
- **ElementType**: uint (typedef)
- **kBufferIndexBits**: log2(sizeof(ElementType))
- **kBufferIndexOffset**: 32 - kBufferIndexBits
- **kElementIndexMask**: (1u << kBufferIndexOffset) - 1
- **kBufferCount**: SCENE_INDEX_BUFFER_COUNT (default: 1)
- **data**: ByteAddressBuffer data[kBufferCount] (or [root] ByteAddressBuffer data[kBufferCount])

**SplitIndexBuffer Methods**:
- **Load16b()** - Load 16-bit triangle index
  - Parameters: baseOffset, triangleIndex
  - Returns: uint16_t3
  - Usage: LoadUint16_t3(baseOffset, baseOffset*4 + triangleIndex * 6)
- **Load32b()** - Load 32-bit triangle index
  - Parameters: baseOffset, triangleIndex
  - Returns: uint3
  - Usage: Load3(baseOffset * 4 + triangleIndex * 12)

**SplitVertexBuffer** (from [`SplitVertexBuffer`](Source/Falcor/Scene/SceneTypes.slang:422)):
- **Purpose**: GPU representation for SplitBuffer<PackedStaticVertexData>
- **ElementType**: PackedStaticVertexData (typedef)
- **kBufferIndexBits**: SCENE_VERTEX_BUFFER_INDEX_BITS (default: 1)
- **kBufferIndexOffset**: 32 - kBufferIndexBits
- **kElementIndexMask**: (1u << kBufferIndexOffset) - 1
- **kBufferCount**: SCENE_VERTEX_BUFFER_COUNT (default: 1)
- **data**: StructuredBuffer<ElementType> data[ArrayMax<1, kBufferCount>.value] (or [root] StructuredBuffer<ElementType> data[1])

**SplitVertexBuffer Subscript**:
- **get**: Get element at index
- **set**: Set element at index

**RWSplitVertexBuffer** (from [`RWSplitVertexBuffer`](Source/Falcor/Scene/SceneTypes.slang:449)):
- **Purpose**: Read-write split buffer for vertex data
- **data**: RWStructuredBuffer<ElementType> data[1] (or [root] RWStructuredBuffer<ElementType> data[1])

**RWSplitVertexBuffer Subscript**:
- **get**: Get element at index
- **set**: Set element at index

### SceneIDs

**File**: [`SceneIDs.h`](Source/Falcor/Scene/SceneIDs.h:1)

**Purpose**: Scene ID type definitions.

**Key Features**:
- Scene1 namespace for distinguishing multiple MeshIDs
- ObjectID system for scene objects
- Type-safe ID definitions

**SceneObjectKind** (from [`SceneObjectKind`](Source/Falcor/Scene/SceneIDs.h:35)):
- **kNode** - NodeID, also for MatrixID for animation
- **kMesh** - MeshID, also curves that tessellate into triangle mesh
- **kCurve** - CurveID
- **kCurveOrMesh** - Used when ID in curves is aliased based on tessellation mode
- **kSdfDesc** - The user-facing ID
- **kSdfGrid** - The internal ID, can get deduplicated
- **kMaterial** - Material ID
- **kLight** - Light ID
- **kCamera** - Camera ID
- **kVolume** - Volume ID
- **kGlobalGeometry** - The linearized global ID (mesh, curve, sdf, custom)
  - Not to be confused with geometryID in curves (which is "either Mesh or Curve, depending on tessellation mode")

**Scene1 Namespace Type Definitions**:
- **NodeID** - ObjectID<SceneObjectKind, SceneObjectKind::kNode, uint32_t>
- **MeshID** - ObjectID<SceneObjectKind, SceneObjectKind::kMesh, uint32_t>
- **CurveID** - ObjectID<SceneObjectKind, SceneObjectKind::kCurve, uint32_t>
- **CurveOrMeshID** - ObjectID<SceneObjectKind, SceneObjectKind::kCurveOrMesh, uint32_t>
- **SdfDescID** - ObjectID<SceneObjectKind, SceneObjectKind::kSdfDesc, uint32_t>
- **SdfGridID** - ObjectID<SceneID<SceneObjectKind, SceneObjectKind::kSdfGrid, uint32_t>
- **MaterialID** - ObjectID<SceneObjectKind, SceneObjectKind::kMaterial, uint32_t>
- **LightID** - ObjectID<SceneObjectKind, SceneObjectKind::kLight, uint32_t>
- **CameraID** - ObjectID<SceneObjectKind, SceneObjectKind::kCamera, uint32_t>
- **VolumeID** - ObjectID<SceneObjectKind, SceneObjectKind::kVolume, uint32_t>
- **GlobalGeometryID** - ObjectID<SceneObjectKind, SceneObjectKind::kGlobalGeometry, uint32_t>

**Falcor Namespace**:
- **using namespace scene1** - Imports scene1 namespace into Falcor namespace

## Technical Details

### Vertex Attribute System

**Vertex Locations**:
- Four standard locations for triangle vertices
- Three locations for curve vertices
- Instance draw ID location
- User element support for custom attributes

**Location Mapping**:
- VERTEX_POSITION_LOC → Position data
- VERTEX_PACKED_NORMAL_TANGENT_CURVE_RADIUS_LOC → Packed normal, tangent, curve radius
- VERTEX_TEXCOORD_LOC → Texture coordinates
- INSTANCE_DRAW_ID_LOC → Instance draw ID

**Packed Data**:
- 32B representation for efficient GPU access
- Normal packed into 16-bit components
- Tangent sign and curve radius packed
- Supports normal, tangent, and curve radius in single 32B value

### Geometry Type System

**Geometry Types**:
- **None**: No geometry
- **TriangleMesh**: Standard triangle mesh
- **DisplacedTriangleMesh**: Triangle mesh with displacement
- **Curve**: Curve geometry
- **SDFGrid**: SDF grid geometry
- **Custom**: Custom primitive geometry

**Geometry Instance Identification**:
- Global instance index: InstanceID() + GeometryIndex()
- Unique identification per instance
- Used for BLAS and TLAS indexing

**Dynamic Geometry**:
- IsDynamic flag indicates mesh is animated
- Can be skinned or vertex-animated
- Affects buffer management and update strategy

**Transform Handedness**:
- TransformFlipped flag indicates coordinate system handedness flip
- Affects winding direction in world space
- IsObjectFrontFaceCW: Object-space winding
- IsWorldFrontFaceCW: World-space winding (combination of mesh winding and transform)

### Mesh Data Structures

**Mesh Description**:
- Vertex and index buffer offsets
- Vertex and index counts
- Skinning data offset
- Previous vertex data offset
- Material ID
- Mesh flags

**Mesh Flags**:
- Use16BitIndices: 16-bit vs 32-bit indices
- IsSkinned: Skinned mesh with bone weights
- IsDisplaced: Mesh with displacement map
- IsAnimated: Vertex-animated mesh
- IsFrontFaceCW: Clockwise front face winding

**Triangle Count Calculation**:
- `indexCount > 0 ? indexCount : vertexCount) / 3`
- Handles both indexed and non-indexed meshes

### Curve Data Structures

**Curve Description**:
- Vertex and index buffer offsets
- Vertex and index counts
- Polynomial degree
- Material ID

**Curve Vertex Attributes**:
- Static: Position, radius, texture coordinates
- Dynamic: Position only

**Curve Degree**:
- Linear (1) by default
- Polynomial representation
- Affects curve tessellation

### Skinning System

**Skinning Vertex Data**:
- Bone ID for bone reference
- Bone weight (4-component for up to 4 bones)
- Static vertex index for base pose
- Bind matrix ID for bone transforms
- Skeleton matrix ID for skeleton transforms

**Skinning Support**:
- IsSkinned flag in MeshFlags
- SkinningVbOffset in MeshDesc
- PrevVbOffset in MeshDesc for base pose

### Custom Primitive System

**Custom Primitive Description**:
- User-defined ID for sub-type identification
- AABB offset for procedural geometry
- 1:1 mapping to list of custom primitive AABBs

**Custom Primitive Types**:
- User-defined geometry
- Procedural AABBs
- Ray tracing support via AABB buffer

### Split Buffer System

**Purpose**: Handle large data that exceeds buffer limits

**Index Buffer**:
- SplitBuffer<uint32_T> representation
- ByteAddressBuffer access
- Supports 16-bit and 32-bit indices
- Top bits of index select buffer
- Lower bits address item inside buffer

**Vertex Buffer**:
- SplitBuffer<PackedStaticVertexData> representation
- StructuredBuffer access
- Read-write support via RWSplitVertexBuffer
- Efficient 32B vertex data

**Buffer Selection**:
- kBufferIndexBits: log2(sizeof(ElementType))
- kBufferCount: SCENE_INDEX_BUFFER_COUNT or SCENE_VERTEX_BUFFER_COUNT
- Top bits: buffer index
- Lower bits: element index

**Large Buffer Support**:
- Splits data across multiple buffers
- Handles >4GB data
- Efficient GPU access pattern

### Scene ID System

**Scene1 Namespace**:
- Distinguishes multiple MeshIDs
- Type-safe ID definitions
- ObjectID-based type system

**Scene Object Kinds**:
- kNode: Node/MatrixID
- kMesh: MeshID (including tessellated curves)
- kCurve: CurveID
- kCurveOrMesh: Aliased based on tessellation
- kSdfDesc: User-facing SDF descriptor ID
- kSdfGrid: Internal SDF grid ID
- kMaterial: Material ID
- kLight: Light ID
- kCamera: Camera ID
- kVolume: Volume ID
- kGlobalGeometry: Linearized global geometry ID

**ID Type Definitions**:
- Strongly-typed IDs for type safety
- ObjectID template with SceneObjectKind
- uint32_t underlying type
- Namespace isolation (scene1 vs Falcor)

## Integration Points

### Scene Integration

**Scene Builder**:
- Type definitions for scene construction
- Geometry type assignment
- Instance identification

**Scene Data**:
- Geometry instance data structures
- Mesh descriptions
- Curve descriptions
- Custom primitive descriptions

### Rendering Integration

**Vertex Buffer Management**:
- Vertex location definitions
- Packed vertex data structures
- Split buffer support for large data

**Index Buffer Management**:
- 16-bit and 32-bit index support
- Split buffer support for large index buffers
- Efficient GPU access patterns

### Ray Tracing Integration

**BLAS Construction**:
- Geometry instance identification
- InstanceIndex and GeometryIndex
- GeometryType flags for dynamic geometry
- AABB generation for custom primitives

**TLAS Construction**:
- Global instance index
- Instance transform handling
- Geometry instance data

### Animation Integration

**Skinning System**:
- Bone ID and weight data
- Bind matrix and skeleton matrix IDs
- Static and dynamic vertex data
- Base pose support via PrevVbOffset

## Architecture Patterns

**Type Safety Pattern**:
- ObjectID-based type system
- Strongly-typed IDs
- Namespace isolation
- Compile-time type checking

**Bit Flag Pattern**:
- Packed flags for efficiency
- Bit-based type encoding
- Efficient flag testing

**Split Buffer Pattern**:
- Multi-buffer support for large data
- Top/bottom bit addressing
- Type-safe buffer access

**Packed Data Pattern**:
- 32B representation for GPU efficiency
- Normal and tangent packing
- Curve radius packing
- Efficient unpacking

**Enum Pattern**:
- GeometryType enumeration
- GeometryTypeFlags bit flags
- MeshFlags bit flags
- SceneObjectKind enumeration

## Progress Log

- **2026-01-07T20:13:00Z**: Scene VertexAttrib, SceneTypes, and SceneIDs subsystems analysis completed. Analyzed vertex attribute definitions, scene type definitions, geometry instance system, mesh data structures, curve data structures, skinning system, custom primitive system, split buffer system, and scene ID system. Documented vertex locations, packed data structures, geometry types, instance identification, dynamic geometry support, transform handedness, and buffer management strategies. Ready to complete Scene module analysis.

## Next Steps

Update Scene Folder Note to mark all sub-modules as complete and finalize Scene module analysis.
