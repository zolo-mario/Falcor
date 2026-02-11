# Scene - Scene Management System

## Module State Machine

-------

-------

-------

-------

-------

-------

-------

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **IScene** - Scene interface and base implementation
- [x] **SceneBuilder** - Scene construction and import system
- [x] **Animation** - Animation system
- [x] **Camera** - Camera system
- [x] **Curves** - Curve tessellation
- [x] **Lights** - Scene light management
- [x] **Material** - Scene material system
- [x] **SDFs** - Signed Distance Field systems
- [x] **Volume** - Volume data structures
- [x] **TriangleMesh** - Triangle mesh data structures
- [x] **SceneCache** - Scene caching system
- [x] **Transform** - Transform utilities
- [x] **VertexAttrib** - Vertex attribute definitions
- [x] **SceneTypes** - Scene type definitions
- [x] **SceneIDs** - Scene ID type definitions

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats, VAO, RtAccelerationStructure)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (Program, ProgramVars, DefineList)
- **Utils/Math** - Mathematical utilities (AABB, Vector, Matrix, Rectangle)
- **Utils/Settings** - Configuration management (Settings)
- **Scene/Lights** - Light system interfaces (ILightCollection, Light, LightProfile, EnvMap)
- **Scene/Animation** - Animation system (Animation, AnimationController)
- **Scene/Material** - Material system (MaterialSystem)
- **Scene/Volume** - Volume system (GridVolume, Grid)
- **Core/AssetResolver** - Asset path resolution

## Module Overview

The Scene module provides comprehensive scene management for real-time rendering. It handles geometry (meshes, curves, custom primitives), materials, lights, cameras, animations, SDF grids, and volume rendering. The module supports both ray tracing and rasterization with efficient acceleration structures.

## Component Specifications

### Scene

**File**: [`Scene.h`](Source/Falcor/Scene/Scene.h:1)

**Purpose**: Main scene representation and management class.

**Key Features**:
- Comprehensive scene data management
- Ray tracing and rasterization support
- Multiple geometry types (meshes, curves, SDF grids, custom primitives)
- Acceleration structures (BLAS, TLAS)
- Material system integration
- Animation system support
- Camera management
- Light collection management
- Environment map support
- SDF grid support
- Volume rendering support
- Scene graph for hierarchical transforms
- Scene caching for fast loading

**Core Data Structures**:

**SceneData** - Complete scene representation:
- [`meshDesc`](Source/Falcor/Scene/Scene.h:245) - Mesh descriptors
- [`meshNames`](Source/Falcor/Scene/Scene.h:253) - Mesh names
- [`meshBBs`](Source/Falcor/Scene/Scene.h:264) - Mesh bounding boxes
- [`meshInstanceData`](Source/Falcor/Scene/Scene.h:265) - Mesh instance data
- [`meshIdToInstanceIds`](Source/Falcor/Scene/Scene.h:266) - Instance mapping
- [`meshGroups`](Source/Falcor/Scene/Scene.h:267) - Mesh groups for BLAS
- [`cachedMeshes`](Source/Falcor/Scene/Scene.h:268) - Cached meshes for vertex animation
- [`prevVertexCount`](Source/Falcor/Scene/Scene.h:269) - Previous vertex count
- [`meshDrawCount`](Source/Falcor/Scene/Scene.h:274) - Mesh draw count
- [`useCompressedHitInfo`](Source/Falcor/Scene/Scene.h:271) - Use compressed hit info
- [`has16BitIndices`](Source/Falcor/Scene/Scene.h:272) - Has 16-bit indices
- [`has32BitIndices`](Source/Falcor/Scene/Scene.h:273) - Has 32-bit indices
- [`meshIndexData`](Source/Falcor/Scene/Scene.h:277) - Mesh index data
- [`meshStaticData`](Source/Falcor/Scene/Scene.h:279) - Mesh static data
- [`meshSkinningData`](Source/Falcor/Scene/Scene.h:281) - Mesh skinning data

**Curve Data**:
- [`curveDesc`](Source/Falcor/Scene/Scene.h:284) - Curve descriptors
- [`curveBBs`](Source/Falcor/Scene/Scene.h:285) - Curve bounding boxes
- [`curveInstanceData`](Source/Falcor/Scene/Scene.h:286) - Curve instance data
- [`curveIndexData`](Source/Falcor/Scene/Scene.h:287) - Curve index data
- [`curveStaticData`](Source/Falcor/Scene/Scene.h:289) - Curve static data
- [`cachedCurves`](Source/Falcor/Scene/Scene.h:290) - Cached curves for vertex animation

**SDF Grid Data**:
- [`sdfGrids`](Source/Falcor/Scene/Scene.h:293) - SDF grid references
- [`sdfGridDesc`](Source/Falcor/Scene/Scene.h:294) - SDF grid descriptors
- [`sdfGridInstances`](Source/Falcor/Scene/Scene.h:295) - SDF grid instances
- [`sdfGridMaxLODCount`](Source/Falcor/Scene/Scene.h:296) - Max LOD count

**Custom Primitives**:
- [`customPrimitiveDesc`](Source/Falcor/Scene/Scene.h:299) - Custom primitive descriptors
- [`customPrimitiveAABBs`](Source/Falcor/Scene/Scene.h:300) - Custom primitive AABBs

**Animation Data**:
- [`animations`](Source/Falcor/Scene/Scene.h:258) - Animation references

**Material System**:
- [`pMaterials`](Source/Falcor/Scene/Scene.h:253) - Material system reference

**Light System**:
- [`lights`](Source/Falcor/Scene/Scene.h:252) - Light references
- [`activeLights`](Source/Falcor/Scene/Scene.h:253) - Active analytic lights
- [`pEnvMap`](Source/Falcor/Scene/Scene.h:257) - Environment map reference

**Volume System**:
- [`gridVolumes`](Source/Falcor/Scene/Scene.h:254) - Grid volume references
- [`grids`](Source/Falcor/Scene/Scene.h:255) - Grid references

**Camera System**:
- [`cameras`](Source/Falcor/Scene/Scene.h:249) - Camera references
- [`mSelectedCamera`](Source/Falcor/Scene/Scene.h:249) - Selected camera index
- [`mCameraSpeed`](Source/Falcor/Scene/Scene.h:250) - Camera speed
- [`mCameraControlsEnabled`](Source/Falcor/Scene/Scene.h:251) - Camera controls enabled
- [`mCameraBounds`](Source/Falcor/Scene/Scene.h:252) - Camera bounds
- [`mCameraList`](Source/Falcor/Scene.h:249) - Camera list
- [`mUpDirection`](Source/Falcor/Scene.h:251) - World up direction
- [`mCamCtrlType`](Source/Falcor/Scene/Scene.h:252) - Camera controller type
- [`mpCamCtrl`](Source/Falcor/Scene/Scene.h:252) - Animation controller reference

**Scene Graph**:
- [`sceneGraph`](Source/Falcor/Scene/Scene.h:257) - Scene graph nodes
- [`mSceneBB`](Source/Falcor/Scene/Scene.h:258) - Scene bounding box

**Render Settings**:
- [`renderSettings`](Source/Falcor/Scene/Scene.h:259) - Render settings

**Metadata**:
- [`metadata`](Source/Falcor/Scene/Scene.h:259) - Importer-provided metadata

**Scene Statistics**:
- [`mSceneStats`](Source/Falcor/Scene/Scene.h:305) - Comprehensive scene statistics

**Update Flags**:
- [`mUpdates`](Source/Falcor/Scene/Scene.h:136) - Update flags tracking

**Acceleration Structures**:
- [`mBlasData`](Source/Falcor/Scene.h:124) - BLAS data
- [`mBlasGroups`](Source/Falcor/Scene.h:125) - BLAS groups
- [`mBlasObjects`](Source/Falcor/Scene/Scene.h:142) - BLAS API objects
- [`mBlasScratch`](Source/Falcor/Scene/Scene.h:143) - Scratch buffer
- [`mTlasCache`](Source/Falcor/Scene.h:137) - TLAS cache

### SceneBuilder

**File**: [`SceneBuilder.h`](Source/Falcor/Scene/SceneBuilder.h:1)

**Purpose**: Scene construction and import system.

**Key Features**:
- Scene/model file import
- In-memory scene construction
- Flexible build flags
- Material system integration
- Geometry optimization
- Animation support
- Curve tessellation
- SDF grid support
- Custom primitives
- Asset resolver integration

**Build Flags** (from [`Flags`](Source/Falcor/SceneBuilder.h:60)):
- Material merging options
- Tangent space options
- Texture space options
- Vertex format options
- BLAS optimization options
- Geometry optimization options

**Core Methods**:
- [`create()`](Source/Falcor/Scene/SceneBuilder.h:356) - Create from file
- [`import()`](Source/Falcor/SceneBuilder.h:349) - Import scene file
- [`importFromMemory()`](Source/Falcor/SceneBuilder.h:360) - Import from memory
- [`getScene()`](Source/Falcor/Scene/Scene.h:374) - Get constructed scene
- [`getAssetResolver()`](Source/Falcor/SceneBuilder.h:362) - Get asset resolver
- [`setRenderSettings()`](Source/Falcor/SceneBuilder.h:388) - Set render settings
- [`setMetadata()`](Source/Falcor/SceneBuilder.h:399) - Set metadata
- [`getScene()`](Source/Falcor/SceneBuilder.h:374) - Get scene (static method)

**Mesh Description** (from [`Mesh`](Source/Falcor/SceneBuilder.h:92)):
- [`name`](Source/Falcor/Scene/Scene.h:110) - Mesh name
- [`faceCount`](Source/Falcor/Scene/Scene.h:111) - Number of primitives
- [`vertexCount`](Source/Falcor/Scene/Scene.h:112) - Number of vertices
- [`indexCount`](Source/Falcor/SceneBuilder.h:113) - Number of indices
- [`pIndices`](Source/Falcor/Scene/Scene.h:114) - Index array
- [`topology`](Source/Falcor/SceneBuilder.h:115) - Primitive topology
- [`pMaterial`](Source/Falcor/SceneBuilder.h:116) - Material reference
- [`attributes`](Source/Falcor/SceneBuilder.h:117) - Vertex attributes

**Curve Description** (from [`Curve`](Source/Falcor/SceneBuilder.h:289)):
- [`name`](Source/Falcor/Scene/Scene.h:289) - Curve name
- [`degree`](Source/Falcor/SceneBuilder.h:296) - Polynomial degree
- [`vertexCount`](Source/Falcor/SceneBuilder.h:297) - Number of vertices
- [`indexCount`](Source/Falcor/SceneBuilder.h:298) - Number of indices
- [`pIndices`](Source/Falcor/SceneBuilder.h:299) - Index array
- [`pMaterial`](Source/Falcor/SceneBuilder.h:300) - Material reference
- [`attributes`](Source/Falcor/SceneBuilder.h:301) - Vertex attributes

**Node Structure** (from [`Node`](Source/Falcor/Scene.h:229)):
- [`name`](Source/Falcor/Scene/Scene.h:231) - Node name
- [`parent`](Source/Falcor/Scene/Scene.h:234) - Parent node ID
- [`transform`](Source/Falcor/Scene.h:235) - Transform matrix
- [`meshBind`](Source/Falcor/Scene/Scene.h:236) - Mesh world transform at bind time
- [`localToBindSpace`](Source/Falcor/Scene/Scene.h:237) - Skeleton to bind space transform

## Architecture Patterns

### Builder Pattern
- [`SceneBuilder`](Source/Falcor/SceneBuilder.h:55) constructs scenes from various sources
- Separate import from construction
- Flexible build flags for optimization
- Material system integration
- Geometry optimization support

### Scene Graph Pattern
- Hierarchical node-based scene graph
- Transform composition
- Instance management
- Material and light binding

### Acceleration Structure Pattern
- BLAS (Bottom Level Acceleration Structure) for meshes
- TLAS (Top Level Acceleration Structure) for instances
- Efficient traversal and ray tracing

### Resource Management Pattern
- Explicit buffer allocation
- Memory pooling
- Lazy resource creation
- Update-based invalidation

## Technical Details

### BLAS Construction Strategy

**Static Non-Instanced Meshes**:
- Pre-transform and group into single BLAS
- Reduces draw calls
- Efficient for large static geometry

**Dynamic Non-Instanced Meshes**:
- Group by transform matrix
- One BLAS per group
- Supports animation and instancing

**Instanced Meshes**:
- One BLAS per group of identical instances
- Efficient for instanced rendering

**Procedural Primitives**:
- Placed in own BLAS at end
- Simplified ray tracing

**BLAS Groups**:
- Each group maps to a BLAS
- Supports up to 11 geometries per BLAS
- InstanceID and GeometryIndex for shader lookup

### TLAS Construction

**Purpose**: Top-level acceleration for all geometry instances.

**Features**:
- Single TLAS for entire scene
- InstanceContribution mapping
- Efficient instance traversal
- Supports dynamic scene updates

**Instance Data**:
- [`mTlasCache`](Source/Falcor/Scene/Scene.h:137) - Cache of TLAS data
- [`mTlasScratch`](Source/Falcor/Scene/Scene.h:143) - Scratch buffer for updates
- [`mTlasPrebuildInfo`](Source/Falcor/Scene/Scene.h:138) - Pre-build information

### Scene Graph

**Purpose**: Hierarchical organization of scene elements.

**Node Types**:
- Transform nodes
- Mesh instances
- Curve instances
- SDF grid instances
- Custom primitives

**Transform Composition**:
- Local-to-world matrix
- Mesh bind matrix (for skinning)
- Local-to-bind-space matrix (for bones)

### Material System Integration

**Purpose**: Manage materials for scene geometry.

**Features**:
- Material system reference
- Material replacement
- Default texture sampler
- Per-material texture loading

### Light System Integration

**Purpose**: Manage lights for scene.

**Light Types**:
- Analytic lights (point, directional, distant, rect, disc, sphere)
- Emissive lights (triangle meshes)
- Environment map

**Light Collection**:
- Lazy creation of light collection
- Efficient light sampling
- Scene update tracking

### Camera System

**Purpose**: Manage cameras for scene.

**Camera Types**:
- First-person camera
- Orbit camera
- Six-DOF camera

**Features**:
- Multiple camera support
- Camera switching
- Viewpoint management
- Camera speed control
- Camera controls enable/disable

### Animation System

**Purpose**: Manage animations for scene.

**Animation Controller**:
- First-person controller
- Orbit controller
- Six-DOF controller

**Features**:
- Multiple animation support
- Animation playback
- Animation speed control
- Loop enable/disable
- Global animation toggle

### SDF Grid System

**Purpose**: Manage signed distance field grids.

**SDF Grid Types**:
- Normalized Dense SDF Grid
- Sparse Brick Set (SBS)
- Sparse Voxel Octree (SVO)
- Sparse Voxel Set (SVS)

**Features**:
- Multiple SDF implementations
- Grid-based intersection
- Gradient evaluation
- Volume rendering
- LOD support

### Volume System

**Purpose**: Manage volume rendering.

**Grid Volume Types**:
- GridVolume

**Features**:
- Grid-based volume sampling
- Homogeneous volume support
- Phase function integration

## Progress Log

- **2026-01-07T18:54:10Z**: Scene module analysis initiated. High-level overview created. Sub-modules identified: IScene, SceneBuilder, Animation, Camera, Curves, Lights, Material, SDFs, Volume, TriangleMesh, SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs. Ready to begin detailed analysis of each sub-module.
- **2026-01-07T20:18:00Z**: Scene module analysis completed. All 15 sub-modules analyzed and documented: IScene, SceneBuilder, Animation, Camera, Curves, Lights, Material, SDFs (SparseBrickSet, SparseVoxelOctree, SparseVoxelSet, NormalizedDenseSDFGrid), Volume, TriangleMesh, SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs. Comprehensive technical specifications created for each component. Scene module analysis is complete.

## Next Steps

Scene module analysis is complete. All sub-modules have been analyzed and documented. Ready to proceed with other modules or finalize the overall analysis.
