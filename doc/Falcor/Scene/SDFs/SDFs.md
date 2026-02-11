# SDFs - Signed Distance Field Systems

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SDFGrid** - SDF grid base class
- [x] **SDF3DPrimitive** - SDF 3D primitive structure
- [x] **SDF3DPrimitiveFactory** - SDF 3D primitive factory
- [x] **SDF3DPrimitiveCommon** - SDF 3D primitive common definitions
- [x] **NormalizedDenseSDFGrid** - Normalized dense SDF grid
- [x] **SparseVoxelSet** - Sparse voxel set SDF
- [x] **SparseBrickSet** - Sparse brick set SDF
- [x] **SparseVoxelOctree** - Sparse voxel octree SDF

### External Dependencies

- **Core/Object** - Base object class
- **Core/API** - Graphics API (Device, Buffer, Texture)
- **Core/Pass** - Compute pass (ComputePass)
- **Utils/Math** - Mathematical utilities (AABB, Vector)
- **Utils/SDF** - SDF utilities (SDFOperationType)
- **Scene/Transform** - Transform utilities

## Module Overview

The SDFs (Signed Distance Field) system provides comprehensive signed distance field representation for volumetric rendering. It supports four different SDF grid implementations: Normalized Dense Grid, Sparse Voxel Set, Sparse Brick Set, and Sparse Voxel Octree. The system handles SDF primitives, grid evaluation, AABB generation for acceleration structures, and GPU resource management.

## Component Specifications

### SDFGrid

**File**: [`SDFGrid.h`](Source/Falcor/Scene/SDFs/SDFGrid.h:1)

**Purpose**: SDF grid base class, stored by distance values at grid cell/voxel corners.

**Key Features**:
- Four SDF grid implementations (NormalizedDenseGrid, SparseVoxelSet, SparseBrickSet, SparseVoxelOctree)
- SDF primitive management (add, remove, update)
- Grid value management (set, load from file, generate cheese values)
- AABB generation for acceleration structures
- GPU resource creation and binding
- Resolution scaling support
- Primitive baking support

**Local Space**: The local space of the SDF grid is [-0.5, 0.5]^3 meaning that initial distances used to create SDF grid should be within range of [-sqrt(3), sqrt(3)].

**SDF Grid Implementations**:

1. **Normalized Dense Grid (NDSDFGrid)**:
   - Creates a dense hierarchy of volume textures
   - All widths of (2^l) - 1 (not mip-levels)
   - Values normalized to range [-1, 1]
   - Value of 1 represents whatever narrow band thickness is chosen half voxel diagonals
   - Example: Using narrow band thickness of 3, value of 1 represents grid distance of 1.5 voxel diagonals

2. **SDF Sparse Voxel Set (SDFSVS)**:
   - Creates a sparse set of voxels
   - Only voxels that overlap implicit surface formed from SDF are instantiated
   - Creates AABBs for all voxels
   - Buffer of AABBs suitable for use as procedural geometry when creating BLAS
   - Distances normalized to range [-1, 1]
   - Value of 1 represents half of a voxel diagonal

3. **SDF Sparse Brick Set (SDFSBS)**:
   - Creates a sparse set of NxNxN bricks
   - Each brick is a dense collection of voxels with normalized distances at corners
   - Distances normalized to range [-1, 1]
   - Value of 1 represents half of a voxel diagonal
   - If (N+1) is multiple of 4 then lossy compression can be enabled

4. **SDF Sparse Voxel Octree (SDFSVO)**:
   - Creates a sparse set of voxels and constructs octree out of them
   - Possible to build BLAS out of AABB buffer constructed by SDFSVO
   - Can be used to intersect rays against SDFSVO
   - Distances stored in voxels of octree normalized to range [-1, 1]
   - Value of 1 represents half of a voxel diagonal

**SDF Grid Types** (from [`SDFGrid::Type`](Source/Falcor/Scene/SDFs/SDFGrid.h:74)):
- **None** - No SDF grid
- **NormalizedDenseGrid** - Normalized dense grid
- **SparseVoxelSet** - Sparse voxel set
- **SparseBrickSet** - Sparse brick set
- **SparseVoxelOctree** - Sparse voxel octree

**Update Flags** (from [`SDFGrid::UpdateFlags`](Source/Falcor/Scene/SDFs/SDFGrid.h:85)):
- **None** - Nothing happened
- **AABBsChanged** - AABBs changed, requires BLAS update
- **BuffersReallocated** - Buffers were reallocated requiring them to be rebound
- **All** - All updates (AABBsChanged | BuffersReallocated)

**Core Methods**:

**Construction**:
- [`SDFGrid()`](Source/Falcor/Scene/SDFs/SDFGrid.h:94) - Constructor with device
- [`~SDFGrid()`](Source/Falcor/Scene/SDFs/SDFGrid.h:95) - Virtual destructor

**Primitive Management**:
- [`setPrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:102) - Set SDF primitives to construct SDF grid
- [`addPrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:108) - Add SDF primitives to SDF grid
- [`removePrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:113) - Remove SDF primitives
- [`updatePrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:118) - Update specified SDF primitives

**Grid Value Management**:
- [`setValues()`](Source/Falcor/Scene/SDFs/SDFGrid.h:124) - Set signed distance values at corners of voxels
- [`loadValuesFromFile()`](Source/Falcor/Scene/SDFs/SDFGrid.h:130) - Set signed distance values from file (.sdfg file)
- [`generateCheeseValues()`](Source/Falcor/Scene/SDFs/SDFGrid.h:136) - Set signed distance values to represent swiss cheese like shape

**File I/O**:
- [`writeValuesFromPrimitivesToFile()`](Source/Falcor/Scene/SDFs/SDFGrid.h:142) - Evaluate SDF grid primitives on grid and write to file
- [`loadPrimitivesFromFile()`](Source/Falcor/Scene/SDFs/SDFGrid.h:149) - Read primitives from file and initialize SDF grid
- [`writePrimitivesToFile()`](Source/Falcor/Scene/SDFs/SDFGrid.h:155) - Write primitives to file

**Update**:
- [`update()`](Source/Falcor/Scene/SDFs/SDFGrid.h:160) - Update SDF grid and apply changes

**Name Management**:
- [`getName()`](Source/Falcor/Scene/SDFs/SDFGrid.h:165) - Get name of SDF grid
- [`setName()`](Source/Falcor/Scene/SDFs/SDFGrid.h:170) - Set name of SDF grid

**Grid Properties**:
- [`getGridWidth()`](Source/Falcor/Scene/SDFs/SDFGrid.h:174) - Get width of grid in voxels
- [`getPrimitiveCount()`](Source/Falcor/Scene/SDFs/SDFGrid.h:178) - Get number of primitives in SDF grid
- [`getPrimitive()`](Source/Falcor/Scene/SDFs/SDFGrid.h:182) - Get primitive corresponding to given primitive ID

**Size and Type**:
- [`getSize()`](Source/Falcor/Scene/SDFs/SDFGrid.h:186) - Get byte size of SDF grid
- [`getMaxPrimitiveIDBits()`](Source/Falcor/Scene/SDFs/SDFGrid.h:190) - Get maximum number of bits for primitive ID field
- [`getType()`](Source/Falcor/Scene/SDFs/SDFGrid.h:194) - Get type of SDF grid

**GPU Resources**:
- [`createResources()`](Source/Falcor/Scene/SDFs/SDFGrid.h:198) - Create GPU data structures required to render SDF grid
- [`getAABBBuffer()`](Source/Falcor/Scene/SDFs/SDFGrid.h:202) - Get AABB buffer for acceleration structure
- [`getAABBCount()`](Source/Falcor/Scene/SDFs/SDFGrid.h:206) - Get number of AABBs
- [`bindShaderData()`](Source/Falcor/Scene/SDFs/SDFGrid.h:210) - Bind SDF grid into shader var

**Resolution Scaling**:
- [`getResolutionScalingFactor()`](Source/Falcor/Scene/SDFs/SDFGrid.h:214) - Get scaling factor representing how grid resolution has changed
- [`resetResolutionScalingFactor()`](Source/Falcor/Scene/SDFs/SDFGrid.h:218) - Set resolution scaling factor to 1.0

**Baking**:
- [`bakePrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:223) - Bake primitives into grid representation (sdfg-file)

**Initialization**:
- [`wasInitializedWithPrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:228) - Check if grid was initialized with primitives
- [`getBakedPrimitiveCount()`](Source/Falcor/Scene/SDFs/SDFGrid.h:232) - Get number of primitives baked into grid representation

**Static Methods**:
- [`getTypeName()`](Source/Falcor/Scene/SDFs/SDFGrid.h:234) - Get type name for given type

**Protected Members**:
- [`mpDevice`](Source/Falcor/Scene/SDFs/SDFGrid.h:243) - Device reference
- [`mName`](Source/Falcor/Scene/SDFs/SDFGrid.h:245) - SDF grid name
- [`mGridWidth`](Source/Falcor/Scene/SDFs/SDFGrid.h:246) - Grid width in voxels
- [`mPrimitives`](Source/Falcor/Scene/SDFs/SDFGrid.h:249) - SDF primitives
- [`mPrimitiveIDToIndex`](Source/Falcor/Scene/SDFs/SDFGrid.h:250) - Primitive ID to index mapping
- [`mNextPrimitiveID`](Source/Falcor/Scene/SDFs/SDFGrid.h:251) - Next primitive ID
- [`mPrimitivesDirty`](Source/Falcor/Scene/SDFs/SDFGrid.h:252) - Primitives changed flag
- [`mpPrimitivesBuffer`](Source/Falcor/Scene/SDFs/SDFGrid.h:253) - Primitives buffer for rendering
- [`mPrimitivesExcludedFromBuffer`](Source/Falcor/Scene/SDFs/SDFGrid.h:254) - Number of primitives to exclude from buffer
- [`mBakedPrimitiveCount`](Source/Falcor/Scene/SDFs/SDFGrid.h:255) - Number of baked primitives
- [`mBakePrimitives`](Source/Falcor/Scene/SDFs/SDFGrid.h:256) - Bake primitives flag
- [`mHasGridRepresentation`](Source/Falcor/Scene/SDFs/SDFGrid.h:257) - Grid representation exists flag
- [`mInitializedWithPrimitives`](Source/Falcor/Scene/SDFs/SDFGrid.h:258) - Initialized with primitives flag
- [`mpSDFGridTexture`](Source/Falcor/Scene/SDFs/SDFGrid.h:260) - SDF grid texture on GPU
- [`mpEvaluatePrimitivesPass`](Source/Falcor/Scene/SDFs/SDFGrid.h:261) - Evaluate primitives compute pass

### SDF3DPrimitive

**File**: [`SDF3DPrimitiveCommon.slang`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveCommon.slang:1)

**Purpose**: SDF 3D primitive structure.

**Shape Types** (from [`SDF3DShapeType`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveCommon.slang:40)):
- **Sphere** - Sphere, defined by radius (1 float)
- **Ellipsoid** - Ellipsoid, defined by three axis aligned radii (3 floats)
- **Box** - Box, defined by three axis aligned half extents (3 floats)
- **Torus** - Torus, defined by radius (1 float). Note: requires primitive blobbing to have thickness
- **Cone** - Cone, defined by tan of cone angle and height (1 floats)
- **Capsule** - Capsule, defined by half length (1 float). Note: requires primitive blobbing to have thickness
- **Count** - Total number of shape types

**SDF3DPrimitive Structure** (from [`SDF3DPrimitive`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveCommon.slang:52)):
- **shapeType** - The shape type (SDF3DShapeType)
- **shapeData** - The shape data used to create shape (float3)
- **shapeBlobbing** - Blobbing that should be applied to shape (float)
- **operationType** - The operation type (SDFOperationType)
- **operationSmoothing** - Smoothing that should be applied to operation (float)
- **translation** - Translation (float3)
- **invRotationScale** - Inverted rotation and scale matrix (float3x3)

### SDF3DPrimitiveFactory

**File**: [`SDF3DPrimitiveFactory.h`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveFactory.h:1)

**Purpose**: SDF 3D primitive factory.

**Core Methods**:
- [`initCommon()`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveFactory.h:41) - Initialize common SDF3DPrimitive
- [`computeAABB()`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveFactory.h:43) - Compute AABB for primitive

## Technical Details

### SDF Grid Local Space

**Coordinate System**:
- Local space: [-0.5, 0.5]^3
- Initial distances should be within range of [-sqrt(3), sqrt(3)]
- Normalized distances stored in range [-1, 1]
- Value of 1 represents half of a voxel diagonal

### SDF Grid Implementations

**Normalized Dense Grid (NDSDFGrid)**:
- Dense hierarchy of volume textures
- All widths of (2^l) - 1
- Not mip-levels
- Values normalized to [-1, 1]
- Narrow band thickness support

**Sparse Voxel Set (SDFSVS)**:
- Sparse set of voxels
- Only voxels overlapping implicit surface instantiated
- AABBs for all voxels
- Suitable for BLAS procedural geometry
- Distances normalized to [-1, 1]

**Sparse Brick Set (SDFSBS)**:
- Sparse set of NxNxN bricks
- Each brick is dense collection of voxels
- Normalized distances at corners
- Distances normalized to [-1, 1]
- Lossy compression support if (N+1) is multiple of 4

**Sparse Voxel Octree (SDFSVO)**:
- Sparse set of voxels with octree structure
- BLAS can be built from AABB buffer
- Ray tracing support
- Distances normalized to [-1, 1]

### SDF Primitive Shapes

**Sphere**:
- Defined by radius (1 float)
- Simplest primitive type

**Ellipsoid**:
- Defined by three axis aligned radii (3 floats)
- Generalized sphere

**Box**:
- Defined by three axis aligned half extents (3 floats)
- Axis-aligned bounding box

**Torus**:
- Defined by radius (1 float)
- Requires primitive blobbing to have thickness

**Cone**:
- Defined by tan of cone angle and height (1 floats)
- Conical shape

**Capsule**:
- Defined by half length (1 float)
- Requires primitive blobbing to have thickness

### Primitive Operations

**Operation Types** (from SDFOperationType):
- Union
- Intersection
- Subtraction
- Smooth union
- Smooth intersection
- Smooth subtraction

**Operation Smoothing**:
- Smoothing parameter for smooth operations
- Controls blend between primitives

**Primitive Blobbing**:
- Blobbing parameter for torus and capsule
- Controls thickness of primitive

### Update Flags

**AABBsChanged**:
- AABBs changed
- Requires BLAS update
- Triggered when primitives are modified

**BuffersReallocated**:
- Buffers were reallocated
- Requires rebinding
- Triggered when grid size changes

### GPU Resource Management

**SDF Grid Texture**:
- Volume texture on GPU
- Holds value representation
- Normalized distances

**Primitives Buffer**:
- Structured buffer of primitives
- Used for rendering
- Updated when primitives change

**AABB Buffer**:
- Buffer of AABBs
- Suitable for BLAS creation
- Used for ray tracing

**Compute Pass**:
- Evaluate primitives pass
- Bakes primitives into grid
- Merges with existing SDF field

### Resolution Scaling

**Purpose**:
- Track how grid resolution has changed
- Can change if loaded by SBS grid and then edited by SDFEditor
- Scaling factor applied to distance values

**Reset**:
- [`resetResolutionScalingFactor()`](Source/Falcor/Scene/SDFs/SDFGrid.h:218) - Set scaling factor to 1.0
- Called when resolution is restored

### Primitive Baking

**Purpose**:
- Bake primitives into grid representation (sdfg-file)
- Reduces runtime evaluation cost
- Creates precomputed distance field

**Batch Processing**:
- [`bakePrimitives()`](Source/Falcor/Scene/SDFs/SDFGrid.h:223) - Bake primitives in batches
- Controls memory usage during baking

### File Formats

**.sdfg File**:
- SDF grid value representation
- Corner values for all voxels
- Grid width metadata
- Normalized distances

**Primitive File**:
- SDF primitive definitions
- Shape types and parameters
- Transforms and operations

**Swiss Cheese Generation**:
- [`generateCheeseValues()`](Source/Falcor/Scene/SDFs/SDFGrid.h:136) - Generate swiss cheese like shape
- Random holes based on seed
- For testing and demonstration

## Integration Points

### Scene Integration

**Scene Builder**:
- SDF grid loading from scene files
- SDF grid initialization
- SDF grid assignment to geometry

**Scene Cache**:
- SDF grid serialization
- SDF grid deserialization

### Rendering Integration

**Ray Tracing**:
- AABB generation for BLAS
- Ray-SDF intersection
- Gradient evaluation

**Path Tracing**:
- SDF grid evaluation at shading points
- Gradient computation for normal estimation

### Acceleration Structures

**BLAS Construction**:
- AABB buffer from SDF grids
- Procedural geometry support
- Efficient ray traversal

### SDF Editor Integration

**Resolution Scaling**:
- Track resolution changes
- Apply scaling factor
- Restore original resolution

## Architecture Patterns

### Factory Pattern
- [`SDF3DPrimitiveFactory`](Source/Falcor/Scene/SDFs/SDF3DPrimitiveFactory.h:38) creates primitives
- Consistent initialization
- AABB computation

### Strategy Pattern
- Four different SDF grid implementations
- Common interface via [`SDFGrid`](Source/Falcor/Scene/SDFs/SDFGrid.h:70)
- Polymorphic grid evaluation

### Builder Pattern
- Primitive-based grid construction
- Add/remove/update primitives
- Batch baking support

### Template Method Pattern
- Virtual [`update()`](Source/Falcor/Scene/SDFs/SDFGrid.h:160) method
- Subclasses implement specific update logic
- Common update tracking

## Progress Log

- **2026-01-07T20:07:00Z**: Scene SDFs subsystem analysis completed. Analyzed SDFGrid base class, SDF3DPrimitive structure, SDF3DPrimitiveFactory, and four SDF grid implementations (NormalizedDenseGrid, SparseVoxelSet, SparseBrickSet, SparseVoxelOctree). Documented SDF grid local space, primitive shapes, operations, update flags, GPU resource management, resolution scaling, primitive baking, and file formats. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (Volume, TriangleMesh, SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs) to complete Scene module analysis.
