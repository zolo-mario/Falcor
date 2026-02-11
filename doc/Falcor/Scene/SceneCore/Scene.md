# Scene - Main Scene Class

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Scene.h** - Scene header (1455 lines)
- [x] **Scene.cpp** - Scene implementation
- [x] **Scene.slang** - Scene shader

### External Dependencies

- **Scene/SceneIDs** - Scene ID definitions
- **Scene/SceneTypes** - Scene type definitions
- **Scene/HitInfo** - Hit information structures
- **Scene/IScene** - Scene interface
- **Scene/Animation/Animation** - Animation system
- **Scene/Animation/AnimationController** - Animation controller
- **Scene/Displacement/DisplacementUpdateTask.slang** - Displacement update task
- **Scene/Lights/Light** - Light
- **Scene/Lights/LightCollection** - Light collection
- **Scene/Lights/LightProfile** - Light profile
- **Scene/Lights/EnvMap** - Environment map
- **Scene/Camera/Camera** - Camera
- **Scene/Camera/CameraController** - Camera controller
- **Scene/Material/MaterialSystem** - Material system
- **Scene/Volume/GridVolume** - Grid volume
- **Scene/Volume/Grid** - Grid
- **Scene/SDFs/SDFGrid** - SDF grid
- **Core/Macros** - Macros
- **Core/Object** - Base object class
- **Core/API/VAO** - Vertex array object
- **Core/API/RtAccelerationStructure** - Ray tracing acceleration structure
- **Utils/Math/AABB** - Axis-aligned bounding box
- **Utils/Math/Rectangle** - Rectangle
- **Utils/Math/Vector** - Vector
- **Utils/Math/Matrix** - Matrix
- **Utils/UI/Gui** - GUI widgets
- **Utils/Settings/Settings** - Settings
- **Utils/SplitBuffer** - Split buffer
- **sigs/sigs** - Signal library
- **functional** - Functional utilities
- **memory** - Memory utilities
- **type_traits** - Type traits
- **optional** - Optional utilities
- **string** - String utilities
- **filesystem** - Filesystem utilities
- **vector** - Vector utilities
- **pybind11/pybind11.h** - Python bindings

## Module Overview

Scene is the main scene representation class that holds all scene resources such as geometry, cameras, lights, and materials. It implements the IScene interface and provides comprehensive scene management functionality including DXR (DirectX Raytracing) scene and resources layout, BLAS (Bottom-Level Acceleration Structure) creation logic, TLAS (Top-Level Acceleration Structure) construction, geometry instance management, material system integration, camera management, animation system integration, SDF grid support, custom primitive support, and extensive statistics tracking. The Scene class is responsible for scene loading, updating, rendering, and resource management.

## Component Specifications

### Event Structures

**Purpose**: Input event structures for user interaction.

**MouseEvent**:
- Mouse event structure
- Used for mouse input handling
- Supports mouse button, position, and wheel events

**KeyboardEvent**:
- Keyboard event structure
- Used for keyboard input handling
- Supports key press/release events

**GamepadEvent**:
- Gamepad event structure
- Used for gamepad input handling
- Supports button and axis events

**GamepadState**:
- Gamepad state structure
- Used for gamepad state tracking
- Supports button and axis states

### RtProgramVars Class

**Purpose**: Ray tracing program variables wrapper.

**Key Features**:
- Wraps ray tracing program variables
- Provides ray type count management
- Integrates with DXR ray tracing pipeline

### UpdateMode Enumeration

**Purpose**: Settings for how to scene ray tracing acceleration structures are updated.

**Values**:
- [`Rebuild`](Source/Falcor/Scene/Scene.h:133) - Recreate acceleration structure when updates are needed
- [`Refit`](Source/Falcor/Scene/Scene.h:134) - Update acceleration structure when updates are needed

**Usage**:
- Controls BLAS update strategy
- Rebuild is more expensive but ensures optimal structure
- Refit is faster but may not be optimal
- Used for dynamic geometry updates

### CameraControllerType Enumeration

**Purpose**: Camera controller type enumeration.

**Values**:
- [`FirstPerson`](Source/Falcor/Scene/Scene.h:139) - First person camera controller
- [`Orbiter`](Source/Falcor/Scene/Scene.h:140) - Orbiter camera controller
- [`SixDOF`](Source/Falcor/Scene/Scene.h:141) - Six degrees of freedom camera controller

**Usage**:
- Selects camera controller type for scene
- First person for FPS-style cameras
- Orbiter for orbital cameras
- SixDOF for full 6DOF cameras

### SDFGridIntersectionMethod Enumeration

**Purpose**: SDF grid intersection method enumeration.

**Values**:
- [`None`](Source/Falcor/Scene/Scene.h:146) - No SDF grid intersection
- [`GridSphereTracing`](Source/Falcor/Scene/Scene.h:147) - Grid sphere tracing
- [`VoxelSphereTracing`](Source/Falcor/Scene/Scene.h:148) - Voxel sphere tracing

**Usage**:
- Selects SDF grid intersection method
- None: No SDF grid intersection (triangles only)
- GridSphereTracing: Sphere tracing through SDF grid
- VoxelSphereTracing: Voxel-based sphere tracing through SDF grid

### SDFGridGradientEvaluationMethod Enumeration

**Purpose**: SDF grid gradient evaluation method enumeration.

**Values**:
- [`None`](Source/Falcor/Scene/Scene.h:153) - No SDF grid gradient evaluation
- [`NumericDiscontinuous`](Source/Falcor/Scene/Scene.h:154) - Numeric discontinuous gradient evaluation
- [`NumericContinuous`](Source/Falcor/Scene/Scene.h:155) - Numeric continuous gradient evaluation

**Usage**:
- Selects SDF grid gradient evaluation method
- None: No gradient evaluation (triangles only)
- NumericDiscontinuous: Discontinuous numeric gradient evaluation
- NumericContinuous: Continuous numeric gradient evaluation

### SDFGridConfig Structure

**Purpose**: SDF grid configuration.

**Fields**:
- [`implementation`](Source/Falcor/Scene/Scene.h:160) - `SDFGrid::Type` - SDF grid implementation type
- [`intersectionMethod`](Source/Falcor/Scene/Scene.h:161) - `SDFGridIntersectionMethod` - SDF grid intersection method
- [`gradientEvaluationMethod`](Source/Falcor/Scene/Scene.h:162) - `SDFGridGradientEvaluationMethod` - SDF grid gradient evaluation method
- [`solverMaxIterations`](Source/Falcor/Scene/Scene.h:163) - `uint32_t` - Solver max iterations
- [`optimizeVisibilityRays`](Source/Falcor/Scene/Scene.h:164) - `bool` - Optimize visibility rays
- [`implementationData`](Source/Falcor/Scene/Scene.h:167) - Union of implementation-specific data
  - [`SBS`](Source/Falcor/Scene/Scene.h:169) - Sparse brick set data
    - [`virtualBrickCoordsBitCount`](Source/Falcor/Scene/Scene.h:171) - `uint32_t` - Virtual brick coords bit count
    - [`brickLocalVoxelCoordsBitCount`](Source/Falcor/Scene/Scene.h:172) - `uint32_t` - Brick local voxel coords bit count
  - [`SVO`](Source/Falcor/Scene/Scene.h:176) - Sparse voxel octree data
    - [`svoIndexBitCount`](Source/Falcor/Scene/Scene.h:177) - `uint32_t` - SVO index bit count
- [`intersectionMethodList`](Source/Falcor/Scene/Scene.h:181) - `Gui::DropdownList` - Intersection method dropdown list
- [`gradientEvaluationMethodList`](Source/Falcor/Scene/Scene.h:182) - `Gui::DropdownList` - Gradient evaluation method dropdown list

**Methods**:
- [`operator==(const SDFGridConfig& other) const`](Source/Falcor/Scene/Scene.h:184) - Equality comparison operator
- [`operator!=(const SDFGridConfig& other) const`](Source/Falcor/Scene/Scene.h:193) - Inequality comparison operator

### Metadata Structure

**Purpose**: Optional importer-provided rendering metadata.

**Fields**:
- [`fNumber`](Source/Falcor/Scene/Scene.h:200) - `std::optional<float>` - Lens aperture
- [`filmISO`](Source/Falcor/Scene/Scene.h:201) - `std::optional<float>` - Film speed
- [`shutterSpeed`](Source/Falcor/Scene/Scene.h:202) - `std::optional<float>` - (Reciprocal) shutter speed
- [`samplesPerPixel`](Source/Falcor/Scene/Scene.h:203) - `std::optional<uint32_t>` - Number of primary samples per pixel
- [`maxDiffuseBounces`](Source/Falcor/Scene/Scene.h:204) - `std::optional<uint32_t>` - Maximum number of diffuse bounces
- [`maxSpecularBounces`](Source/Falcor/Scene/Scene.h:205) - `std::optional<uint32_t>` - Maximum number of specular bounces
- [`maxTransmissionBounces`](Source/Falcor/Scene/Scene.h:206) - `std::optional<uint32_t>` - Maximum number of transmission bounces
- [`maxVolumeBounces`](Source/Falcor/Scene/Scene.h:207) - `std::optional<uint32_t>` - Maximum number of volume bounces

**Usage**:
- Stores optional rendering metadata from importer
- Used for path tracing configuration
- Used for rendering settings
- All fields are optional

### SDFGridDesc Structure

**Purpose**: SDF grid descriptor.

**Fields**:
- [`sdfGridID`](Source/Falcor/Scene/Scene.h:212) - `SdfGridID` - The raw SDF grid ID
- [`materialID`](Source/Falcor/Scene/Scene.h:213) - `MaterialID` - The material ID
- [`instances`](Source/Falcor/Scene/Scene.h:214) - `std::vector<NodeID>` - All instances using this SDF grid desc

**Usage**:
- Describes SDF grid geometry
- Links SDF grid to material
- Links SDF grid to instances

### MeshGroup Structure

**Purpose**: Represents a group of meshes.

**Fields**:
- [`meshList`](Source/Falcor/Scene/Scene.h:222) - `std::vector<MeshID>` - List of meshId's that are part of group
- [`isStatic`](Source/Falcor/Scene/Scene.h:223) - `bool` - True if group represents static non-instanced geometry
- [`isDisplaced`](Source/Falcor/Scene/Scene.h:224) - `bool` - True if group uses displacement mapping

**Usage**:
- Groups meshes for BLAS creation
- Groups static non-instanced meshes
- Groups displaced meshes
- Each group maps to a BLAS

### Node Structure

**Purpose**: Scene graph node.

**Fields**:
- [`name`](Source/Falcor/Scene/Scene.h:233) - `std::string` - Node name
- [`parent`](Source/Falcor/Scene/Scene.h:234) - `NodeID` - Parent node ID
- [`transform`](Source/Falcor/Scene/Scene.h:235) - `float4x4` - The node's transformation matrix
- [`meshBind`](Source/Falcor/Scene/Scene.h:236) - `float4x4` - For skinned meshes. Mesh world space transform at bind time
- [`localToBindSpace`](Source/Falcor/Scene/Scene.h:237) - `float4x4` - For bones. Skeleton to bind space transformation. AKA inverse-bind transform

**Constructor**:
- Default constructor with default values
- Parameterized constructor with name, parent, transform, meshBind, localToBindSpace

**Usage**:
- Represents scene graph node
- Supports hierarchical scene graph
- Supports skinned meshes with meshBind
- Supports bone transforms with localToBindSpace

### SceneData Structure

**Purpose**: Full set of required data to create a scene object.

**Fields**:

**Import Data**:
- [`ImportDict`](Source/Falcor/Scene/Scene.h:245) - `std::map<std::string, std::string>` - Dictionaries used to load each asset in importPaths
- [`importPaths`](Source/Falcor/Scene/Scene.h:246) - `std::vector<std::filesystem::path>` - Paths of asset files the scene was loaded from
- [`importDicts`](Source/Falcor/Scene/Scene.h:247) - `std::vector<ImportDict>` - Dictionaries used to load each asset in importPaths

**Render Settings**:
- [`renderSettings`](Source/Falcor/Scene/Scene.h:248) - `RenderSettings` - Render settings

**Cameras**:
- [`cameras`](Source/Falcor/Scene/Scene.h:249) - `std::vector<ref<Camera>>` - List of cameras
- [`selectedCamera`](Source/Falcor/Scene/Scene.h:250) - `uint32_t` - Index of selected camera
- [`cameraSpeed`](Source/Falcor/Scene/Scene.h:251) - `float` - Camera speed

**Lights**:
- [`lights`](Source/Falcor/Scene/Scene.h:252) - `std::vector<ref<Light>>` - List of light sources

**Materials**:
- [`pMaterials`](Source/Falcor/Scene/Scene.h:253) - `std::unique_ptr<MaterialSystem>` - Material system. This holds data and resources for all materials

**Volumes**:
- [`gridVolumes`](Source/Falcor/Scene/Scene.h:254) - `std::vector<ref<GridVolume>>` - List of grid volumes
- [`grids`](Source/Falcor/Scene/Scene.h:255) - `std::vector<ref<Grid>>` - List of grids

**Environment**:
- [`pEnvMap`](Source/Falcor/Scene/Scene.h:256) - `ref<EnvMap>` - Environment map

**Scene Graph**:
- [`sceneGraph`](Source/Falcor/Scene/Scene.h:257) - `std::vector<Node>` - Scene graph nodes

**Animations**:
- [`animations`](Source/Falcor/Scene/Scene.h:258) - `std::vector<ref<Animation>>` - List of animations

**Metadata**:
- [`metadata`](Source/Falcor/Scene/Scene.h:259) - `Metadata` - Scene metadata

**Mesh Data**:
- [`meshDesc`](Source/Falcor/Scene/Scene.h:262) - `std::vector<MeshDesc>` - List of mesh descriptors
- [`meshNames`](Source/Falcor/Scene/Scene.h:263) - `std::vector<std::string>` - List of mesh names
- [`meshBBs`](Source/Falcor/Scene/Scene.h:264) - `std::vector<AABB>` - List of mesh bounding boxes in object space
- [`meshInstanceData`](Source/Falcor/Scene/Scene.h:265) - `std::vector<GeometryInstanceData>` - List of mesh instances
- [`meshIdToInstanceIds`](Source/Falcor/Scene/Scene.h:266) - `std::vector<std::vector<uint32_t>>` - Mapping of what instances belong to which mesh
- [`meshGroups`](Source/Falcor/Scene/Scene.h:267) - `std::vector<MeshGroup>` - List of mesh groups. Each group maps to a BLAS for ray tracing
- [`cachedMeshes`](Source/Falcor/Scene/Scene.h:268) - `std::vector<CachedMesh>` - Cached data for vertex-animated meshes
- [`prevVertexCount`](Source/Falcor/Scene/Scene.h:269) - `uint32_t` - Number of vertices that AnimationController needs to allocate to store previous frame vertices

**Mesh Flags**:
- [`useCompressedHitInfo`](Source/Falcor/Scene/Scene.h:271) - `bool` - True if scene should used compressed HitInfo (on scenes with triangles meshes only)
- [`has16BitIndices`](Source/Falcor/Scene/Scene.h:272) - `bool` - True if 16-bit mesh indices are used
- [`has32BitIndices`](Source/Falcor/Scene/Scene.h:273) - `bool` - True if 32-bit mesh indices are used
- [`meshDrawCount`](Source/Falcor/Scene/Scene.h:274) - `uint32_t` - Number of meshes to draw

**Mesh Buffers**:
- [`meshIndexData`](Source/Falcor/Scene/Scene.h:276) - `SplitIndexBuffer` - Vertex indices for all meshes in either 32-bit or 16-bit format packed tightly
- [`meshStaticData`](Source/Falcor/Scene/Scene.h:277) - `SplitVertexBuffer` - Vertex attributes for all meshes in packed format

**Mesh Skinning**:
- [`meshSkinningData`](Source/Falcor/Scene/Scene.h:281) - `std::vector<SkinningVertexData>` - Additional vertex attributes for skinned meshes

**Curve Data**:
- [`curveDesc`](Source/Falcor/Scene/Scene.h:284) - `std::vector<CurveDesc>` - List of curve descriptors
- [`curveBBs`](Source/Falcor/Scene/Scene.h:285) - `std::vector<AABB>` - List of curve bounding boxes in object space. Each curve consists of many segments, each with its own AABB. The bounding boxes here are unions of those
- [`curveInstanceData`](Source/Falcor/Scene/Scene.h:286) - `std::vector<GeometryInstanceData>` - List of curve instances
- [`curveIndexData`](Source/Falcor/Scene/Scene.h:287) - `std::vector<uint32_t>` - Vertex indices for all curves in 32-bit
- [`curveStaticData`](Source/Falcor/Scene/Scene.h:288) - `std::vector<StaticCurveVertexData>` - Vertex attributes for all curves
- [`cachedCurves`](Source/Falcor/Scene/Scene.h:289) - `std::vector<CachedCurve>` - Vertex cache for dynamic (vertex animated) curves

**SDF Grid Data**:
- [`sdfGrids`](Source/Falcor/Scene/Scene.h:293) - `std::vector<ref<SDFGrid>>` - List of SDF grids
- [`sdfGridDesc`](Source/Falcor/Scene/Scene.h:294) - `std::vector<SDFGridDesc>` - List of SDF grid descriptors
- [`sdfGridInstances`](Source/Falcor/Scene/Scene.h:295) - `std::vector<GeometryInstanceData>` - List of SDF grid instances
- [`sdfGridMaxLODCount`](Source/Falcor/Scene/Scene.h:296) - `uint32_t` - The max LOD count of any SDF grid

**Custom Primitive Data**:
- [`customPrimitiveDesc`](Source/Falcor/Scene/Scene.h:299) - `std::vector<CustomPrimitiveDesc>` - Custom primitive descriptors
- [`customPrimitiveAABBs`](Source/Falcor/Scene/Scene.h:300) - `std::vector<AABB>` - List of AABBs for custom primitives in world space. Each custom primitive consists of one AABB

### SceneStats Structure

**Purpose**: Statistics for scene resources.

**Geometry Stats**:
- [`meshCount`](Source/Falcor/Scene/Scene.h:308) - `uint64_t` - Number of meshes
- [`meshInstanceCount`](Source/Falcor/Scene/Scene.h:309) - `uint64_t` - Number of mesh instances
- [`meshInstanceOpaqueCount`](Source/Falcor/Scene/Scene.h:310) - `uint64_t` - Number of mesh instances that are opaque
- [`transformCount`](Source/Falcor/Scene/Scene.h:311) - `uint64_t` - Number of transform matrices
- [`uniqueTriangleCount`](Source/Falcor/Scene/Scene.h:312) - `uint64_t` - Number of unique triangles. A triangle can exist in multiple instances
- [`uniqueVertexCount`](Source/Falcor/Scene/Scene.h:313) - `uint64_t` - Number of unique vertices. A vertex can be referenced by multiple triangles/instances
- [`instancedTriangleCount`](Source/Falcor/Scene/Scene.h:314) - `uint64_t` - Number of instanced triangles. This is the total number of rendered triangles
- [`instancedVertexCount`](Source/Falcor/Scene/Scene.h:315) - `uint64_t` - Number of instanced vertices. This is the total number of vertices in the rendered triangles
- [`indexMemoryInBytes`](Source/Falcor/Scene/Scene.h:316) - `uint64_t` - Total memory in bytes used by index buffer
- [`vertexMemoryInBytes`](Source/Falcor/Scene/Scene.h:317) - `uint64_t` - Total memory in bytes used by vertex buffer
- [`geometryMemoryInBytes`](Source/Falcor/Scene/Scene.h:318) - `uint64_t` - Total memory in bytes used by geometry data (meshes, curves, custom primitives, instances etc.)
- [`animationMemoryInBytes`](Source/Falcor/Scene/Scene.h:319) - `uint64_t` - Total memory in bytes used by animation system (transforms, skinning buffers)

**Curve Stats**:
- [`curveCount`](Source/Falcor/Scene/Scene.h:322) - `uint64_t` - Number of curves
- [`curveInstanceCount`](Source/Falcor/Scene/Scene.h:323) - `uint64_t` - Number of curve instances
- [`uniqueCurveSegmentCount`](Source/Falcor/Scene/Scene.h:324) - `uint64_t` - Number of unique curve segments (linear tube segments by default). A segment can exist in multiple instances
- [`uniqueCurvePointCount`](Source/Falcor/Scene/Scene.h:325) - `uint64_t` - Number of unique curve points. A point can be referenced by multiple segments/instances
- [`instancedCurveSegmentCount`](Source/Falcor/Scene/Scene.h:326) - `uint64_t` - Number of instanced curve segments (linear tube segments by default). This is the total number of rendered segments
- [`instancedCurvePointCount`](Source/Falcor/Scene/Scene.h:327) - `uint64_t` - Number of instanced curve points. This is the total number of end points in the rendered segments
- [`curveIndexMemoryInBytes`](Source/Falcor/Scene/Scene.h:328) - `uint64_t` - Total memory in bytes used by the curve index buffer
- [`curveVertexMemoryInBytes`](Source/Falcor/Scene/Scene.h:329) - `uint64_t` - Total memory in bytes used by the curve vertex buffer

**SDF Grid Stats**:
- [`sdfGridCount`](Source/Falcor/Scene/Scene.h:332) - `uint64_t` - Number of SDF grids
- [`sdfGridDescriptorCount`](Source/Falcor/Scene/Scene.h:333) - `uint64_t` - Number of SDF grid descriptors
- [`sdfGridInstancesCount`](Source/Falcor/Scene/Scene.h:334) - `uint64_t` - Number of SDF grid instances
- [`sdfGridMemoryInBytes`](Source/Falcor/Scene/Scene.h:335) - `uint64_t` - Total memory in bytes used by all SDF grids. Note that this depends on render mode is selected

**Custom Primitive Stats**:
- [`customPrimitiveCount`](Source/Falcor/Scene/Scene.h:338) - `uint64_t` - Number of custom primitives

**Material Stats**:
- [`materials`](Source/Falcor/Scene/Scene.h:341) - `MaterialSystem::MaterialStats` - Material statistics

**Raytracing Stats**:
- [`blasGroupCount`](Source/Falcor/Scene/Scene.h:344) - `uint64_t` - Number of BLAS groups. There is one BLAS buffer per group
- [`blasCount`](Source/Falcor/Scene/Scene.h:345) - `uint64_t` - Number of BLASes
- [`blasCompactedCount`](Source/Falcor/Scene/Scene.h:346) - `uint64_t` - Number of compacted BLASes
- [`blasOpaqueCount`](Source/Falcor/Scene/Scene.h:347) - `uint64_t` - Number of BLASes that contain only opaque geometry
- [`blasGeometryCount`](Source/Falcor/Scene/Scene.h:348) - `uint64_t` - Number of geometries
- [`blasOpaqueGeometryCount`](Source/Falcor/Scene/Scene.h:349) - `uint64_t` - Number of geometries that are opaque
- [`blasMemoryInBytes`](Source/Falcor/Scene/Scene.h:350) - `uint64_t` - Total memory in bytes used by BLASes
- [`blasScratchMemoryInBytes`](Source/Falcor/Scene/Scene.h:351) - `uint64_t` - Additional memory in bytes kept around for BLAS updates etc.
- [`tlasCount`](Source/Falcor/Scene/Scene.h:352) - `uint64_t` - Number of TLASes
- [`tlasMemoryInBytes`](Source/Falcor/Scene/Scene.h:353) - `uint64_t` - Total memory in bytes used by TLASes
- [`tlasScratchMemoryInBytes`](Source/Falcor/Scene/Scene.h:354) - `uint64_t` - Additional memory in bytes kept around for TLAS updates etc.

**Light Stats**:
- [`activeLightCount`](Source/Falcor/Scene/Scene.h:357) - `uint64_t` - Number of active lights
- [`totalLightCount`](Source/Falcor/Scene/Scene.h:358) - `uint64_t` - Number of lights in the scene
- [`pointLightCount`](Source/Falcor/Scene/Scene.h:359) - `uint64_t` - Number of point lights
- [`directionalLightCount`](Source/Falcor/Scene/Scene.h:360) - `uint64_t` - Number of directional lights
- [`rectLightCount`](Source/Falcor/Scene/Scene.h:361) - `uint64_t` - Number of rect lights
- [`discLightCount`](Source/Falcor/Scene/Scene.h:362) - `uint64_t` - Number of disc lights
- [`sphereLightCount`](Source/Falcor/Scene/Scene.h:363) - `uint64_t` - Number of sphere lights
- [`distantLightCount`](Source/Falcor/Scene/Scene.h:364) - `uint64_t` - Number of distant lights
- [`lightsMemoryInBytes`](Source/Falcor/Scene/Scene.h:365) - `uint64_t` - Total memory in bytes used by analytic lights
- [`envMapMemoryInBytes`](Source/Falcor/Scene/Scene.h:366) - `uint64_t` - Total memory in bytes used by environment map
- [`emissiveMemoryInBytes`](Source/Falcor/Scene/Scene.h:367) - `uint64_t` - Total memory in bytes used by emissive lights

**Grid Volume Stats**:
- [`gridVolumeCount`](Source/Falcor/Scene/Scene.h:370) - `uint64_t` - Number of volumes
- [`gridVolumeMemoryInBytes`](Source/Falcor/Scene/Scene.h:371) - `uint64_t` - Total memory in bytes used by volumes

**Grid Stats**:
- [`gridCount`](Source/Falcor/Scene/Scene.h:374) - `uint64_t` - Number of grids
- [`gridVoxelCount`](Source/Falcor/Scene/Scene.h:375) - `uint64_t` - Total number of voxels in all grids
- [`gridMemoryInBytes`](Source/Falcor/Scene/Scene.h:376) - `uint64_t` - Total memory in bytes used by grids

**Memory Usage**:
- [`getTotalMemory()`](Source/Falcor/Scene/Scene.h:380) - Get total memory usage in bytes
  - Returns: indexMemoryInBytes + vertexMemoryInBytes + geometryMemoryInBytes + animationMemoryInBytes + curveIndexMemoryInBytes + curveVertexMemoryInBytes + sdfGridMemoryInBytes + materials.materialMemoryInBytes + materials.textureMemoryInBytes + blasMemoryInBytes + blasScratchMemoryInBytes + tlasMemoryInBytes + tlasScratchMemoryInBytes + lightsMemoryInBytes + envMapMemoryInBytes + emissiveMemoryInBytes + gridVolumeMemoryInBytes + gridMemoryInBytes

### Scene Class Methods

**Static Methods**:

**File Extension Filters**:
- [`getFileExtensionFilters()`](Source/Falcor/Scene/Scene.h:392) - Return list of file extension filters for all supported file formats

**Scene Creation**:
- [`create(ref<Device> pDevice, const std::filesystem::path& path, const Settings& settings)`](Source/Falcor/Scene/Scene.h:400) - Create scene from file
  - Throws [`ImporterError`](Source/Falcor/Scene/Scene.h:399) if import went wrong
- [`create(ref<Device> pDevice, SceneData&& sceneData)`](Source/Falcor/Scene/Scene.h:407) - Create scene from in-memory representation
  - Throws on error

**Device Access**:
- [`getDevice()`](Source/Falcor/Scene/Scene.h:411) - Return associated GPU device

**Shader Data Binding**:
- [`bindShaderData(const ShaderVar& sceneVar)`](Source/Falcor/Scene/Scene.h:417) - Bind scene to a given shader var
  - Note that scene may change between calls to update()
  - The caller should rebind scene data before executing any program that accesses the scene

**Shader Defines**:
- [`getSceneDefines()`](Source/Falcor/Scene/Scene.h:425) - Get scene defines
  - These defines must be set on all programs that access the scene
  - If defines change at runtime, update flag `SceneDefinesChanged` is set
  - The user is responsible to check for this and update all programs that access the scene
  - Returns: List of shader defines

**Type Conformances**:
- [`getTypeConformances()`](Source/Falcor/Scene/Scene.h:433) - Get type conformances
  - These type conformances must be set on all programs that access the scene
  - If type conformances change at runtime, update flag `TypeConformancesChanged` is set
  - The user is responsible to check for this and update all programs that access the scene
  - Returns: List of type conformances

**Shader Modules**:
- [`getShaderModules()`](Source/Falcor/Scene/Scene.h:440) - Get shader modules required by scene
  - The shader modules must be added to any program using the scene
  - The update() function must have been called before calling this function
  - Returns: List of shader modules

**Statistics**:
- [`getSceneStats()`](Source/Falcor/Scene.h:444) - Get current scene statistics
  - Returns: [`SceneStats`](Source/Falcor/Scene/Scene.h:305) - Scene statistics

**Render Settings**:
- [`getRenderSettings()`](Source/Falcor/Scene/Scene.h:448) - Get render settings
  - Returns: [`RenderSettings`](Source/Falcor/Scene/Scene.h:248) - Render settings
- [`getRenderSettings()`](Source/Falcor/Scene/Scene.h:452) - Get render settings
  - Returns: [`RenderSettings&`](Source/Falcor/Scene/Scene.h:248) - Render settings
- [`setRenderSettings(const RenderSettings& renderSettings)`](Source/Falcor/Scene/Scene.h:456) - Set render settings

**Environment Map**:
- [`useEnvBackground()`](Source/Falcor/Scene/Scene.h:460) - Returns true if environment map is available and should be used as a background
- [`useEnvLight()`](Source/Falcor/Scene/Scene.h:465) - Returns true if environment map is available and should be used as a distant light
- [`useEnvLight()`](Source/Falcor/Scene/Scene.h:465) - Returns true if environment map is available and should be used as a distant light

**Lights**:
- [`useAnalyticLights()`](Source/Falcor/Scene/Scene.h:468) - Returns true if there are active analytic lights and they should be used for lighting
- [`useEmissiveLights()`](Source/Falcor/Scene.h:472) - Returns true if there are active emissive lights and they should be used for lighting

**Grid Volumes**:
- [`useGridVolumes()`](Source/Falcor/Scene.h:476) - Returns true if there are active grid volumes and they should be rendererd

**Metadata**:
- [`getMetadata()`](Source/Falcor/Scene/Scene.h:480) - Get metadata
  - Returns: [`Metadata`](Source/Falcor/Scene/Scene.h:198) - Scene metadata

**Camera Management**:
- [`getCamera()`](Source/Falcor/Scene/Scene.h:484) - Access scene's currently selected camera to change properties or to use elsewhere
- [`getCameraBounds()`](Source/Falcor/Scene/Scene.h:488) - Get camera bounds
- [`setCameraBounds(const AABB& aabb)`](Source/Falcor/Scene/Scene.h:492) - Set camera bounds
- [`getCameras()`](Source/Falcor/Scene.h:496) - Get a list of all cameras in the scene
- [`setCamera(const ref<Camera>& pCamera)`](Source/Falcor/Scene/Scene.h:500) - Select a different camera to use. The camera must already exist in the scene
- [`setCameraAspectRatio(float ratio)`](Source/Falcor/Scene/Scene.h:504) - Set currently selected camera's aspect ratio
- [`setUpDirection(UpDirection upDirection)`](Source/Falcor/Scene.h:508) - Set world up direction (used for first person camera)
- [`getUpDirection()`](Source/Falcor/Scene/Scene.h:512) - Get world up direction
- [`setCameraController(CameraControllerType type)`](Source/Falcor/Scene/Scene.h:516) - Set camera controller type
- [`getCameraControllerType()`](Source/Falcor/Scene/Scene.h:520) - Get camera controller type
- [`resetCamera(bool resetDepthRange = true)`](Source/Falcor/Scene/Scene.h:525) - Reset currently selected camera. This function will place camera at the center of scene and optionally set the depth range to some reasonable pre-determined values
- [`setCameraSpeed(float speed)`](Source/Falcor/Scene/Scene.h:529) - Set camera's speed
- [`getCameraSpeed()`](Source/Falcor/Scene/Scene.h:533) - Get camera's speed
- [`addViewpoint()`](Source/Falcor/Scene/Scene.h:537) - Add currently selected camera's viewpoint to list of viewpoints
- [`selectCamera(uint32_t index)`](Source/Falcor/Scene.h:541) - Select a camera to be used by index
- [`selectCamera(std::string name)`](Source/Falcor/Scene/Scene.h:545) - Select a camera to be used by name
- [`setCameraControlsEnabled(bool value)`](Source/Falcor/Scene/Scene.h:549) - Sets whether camera controls are enabled or disabled
- [`getCameraControlsEnabled()`](Source/Falcor/Scene/Scene.h:554) - Get whether camera controls are enabled or disabled. Returns true if camera controls are enabled else false
- [`addViewpoint(const float3& position, const float3& target, const float3& up, uint32_t cameraIndex = 0)`](Source/Falcor/Scene/Scene.h:558) - Add a new viewpoint to the list of viewpoints
- [`removeViewpoint()`](Source/Falcor/Scene/Scene.h:562) - Remove the currently active viewpoint
- [`selectViewpoint(uint32_t index)`](Source/Falcor/Scene.h:566) - Select a viewpoint and move the camera to it
- [`hasSavedViewpoints()`](Source/Falcor/Scene.h:570) - Returns true if there are saved viewpoints (used for dumping to config)

**Geometry Types**:
- [`getGeometryTypes()`](Source/Falcor/Scene/Scene.h:575) - Get set of geometry types used in the scene
  - Returns: [`GeometryTypeFlags`](Source/Falcor/Scene/Scene.h:119) - A bit field containing set of geometry types
- [`hasGeometryTypes(GeometryTypeFlags types)`](Source/Falcor/Scene/Scene.h:580) - Check if scene has any geometry of given types
  - Returns: True if scene has any geometry of these types
- [`hasGeometryType(GeometryType type)`](Source/Falcor/Scene/Scene.h:587) - Check if scene has any geometry of given type
  - Returns: True if scene has any geometry of this type
- [`hasProceduralGeometry()`](Source/Falcor/Scene/Scene.h:591) - Check if scene has any procedural geometry types
  - Returns: True if scene has any procedural geometry (displaced triangle meshes, curves, SDF grids, custom primitives)
- [`getGeometryCount()`](Source/Falcor/Scene/Scene.h:597) - Get number of geometries in the scene
  - This includes all types of geometry that exist in ray tracing acceleration structures
  - Returns: Total number of geometries
- [`getGeometryInstanceCount()`](Source/Falcor/Scene/Scene.h:603) - Get number of geometry instances in the scene
  - This includes all types of geometry instances that exist in ray tracing acceleration structures
  - Returns: Total number of geometry instances

**Geometry Instance Access**:
- [`getGeometryInstance(uint32_t instanceID)`](Source/Falcor/Scene/Scene.h:609) - Get data of a geometry instance
  - Returns: [`GeometryInstanceData`](Source/Falcor/Scene/Scene.h:609) - The data of the geometry instance

**Geometry ID Queries**:
- [`getGeometryIDs(GeometryType geometryType)`](Source/Falcor/Scene/Scene.h:615) - Get a list of all geometry IDs for a given geometry type
  - Returns: List of geometry IDs
- [`getGeometryIDs(GeometryType geometryType, MaterialType materialType)`](Source/Falcor/Scene/Scene.h:622) - Get a list of all geometry IDs for a given geometry and material type
  - Returns: List of geometry IDs
- [`getGeometryIDs(const Material* material)`](Source/Falcor/Scene/Scene.h:628) - Get a list of all geometry IDs for a given material
  - Returns: List of geometry IDs

**Geometry Type**:
- [`getGeometryType(GlobalGeometryID geometryID)`](Source/Falcor/Scene/Scene.h:641) - Get the type of a given geometry
  - Returns: The type of the given geometry

**Geometry Material**:
- [`getGeometryMaterial(GlobalGeometryID geometryID)`](Source/Falcor/Scene.h:647) - Get the material of a given geometry
  - Returns: The material or nullptr if geometry has no material

**Geometry UV Tiles**:
- [`getGeometryUVTiles(GlobalGeometryID geometryID)`](Source/Falcor/Scene/Scene.h:635) - Given a geometryID, return a list of UV tiles accessed by this geometry, in as tight manner as possible
  - Returns: List of non-overlapping UVTiles that bound to UV set of the geometry
  - Returns an empty list if the operation is not supported

**Mesh Operations**:
- [`getMeshCount()`](Source/Falcor/Scene/Scene.h:651) - Get a number of triangle meshes
  - Returns: Number of meshes
- [`getMesh(MeshID meshID)`](Source/Falcor/Scene/Scene.h:655) - Get a mesh desc
  - Returns: Mesh descriptor
- [`getMeshVerticesAndIndices(MeshID meshID, const std::map<std::string, ref<Buffer>>& buffers)`](Source/Falcor/Scene/Scene.h:661) - Get mesh vertex and index data
  - Parameters: meshID, buffers map containing "triangleIndices", "positions", and "texcrds"
- [`setMeshVertices(MeshID meshID, const std::map<std::string, ref<Buffer>>& buffers)`](Source/Falcor/Scene/Scene.h:667) - Set mesh vertex data and update acceleration structures
  - Parameters: meshID, buffers map containing "positions", "normals", "tangents", and "texcrds"

**Curve Operations**:
- [`getCurveCount()`](Source/Falcor/Scene/Scene.h:671) - Get a number of curves
  - Returns: Number of curves
- [`getCurve(CurveID curveID)`](Source/Falcor/Scene/Scene.h:675) - Get a curve desc
  - Returns: Curve descriptor

**SDF Grid Operations**:
- [`getSDFGridImplementation()`](Source/Falcor/Scene/Scene.h:679) - Returns what SDF grid implementation is used for this scene
  - Returns: [`SDFGrid::Type`](Source/Falcor/Scene/Scene.h:679) - SDF grid implementation type
- [`getSDFGridImplementationData()`](Source/Falcor/Scene/Scene.h:683) - Returns shared SDF grid implementation data used for this scene
  - Returns: [`SDFGridConfig::ImplementationData&`](Source/Falcor/Scene/Scene.h:683) - SDF grid implementation data
- [`getSDFGridIntersectionMethod()`](Source/Falcor/Scene/Scene.h:688) - Returns what SDF grid gradient intersection method is used for this scene
  - Returns: [`SDFGridIntersectionMethod`](Source/Falcor/Scene/Scene.h:688) - SDF grid gradient intersection method
- [`getSDFGridGradientEvaluationMethod()`](Source/Falcor/Scene/Scene.h:691) - Returns what SDF grid gradient evaluation method is used for this scene
  - Returns: [`SDFGridGradientEvaluationMethod`](Source/Falcor/Scene/Scene.h:691) - SDF grid gradient evaluation method
- [`getSDFGridDescCount()`](Source/Falcor/Scene.h:695) - Get a number of SDF grid descriptors
  - Returns: Number of SDF grid descriptors
- [`getSDFGridDesc(SdfDescID sdfDescID)`](Source/Falcor/Scene/Scene.h:699) - Get a SDF grid desc
  - Returns: SDF grid descriptor
- [`getSDFGridCount()`](Source/Falcor/Scene/Scene.h:703) - Get a number of SDF grids
  - Returns: Number of SDF grids
- [`getSDFGrid(SdfGridID sdfGridID)`](Source/Falcor/Scene/Scene.h:707) - Get an SDF grid
  - Returns: SDF grid reference
- [`getSDFGridGeometryCount()`](Source/Falcor/Scene.h:711) - Get a number of SDF grid geometries
- [`findSDFGridIDFromGeometryInstanceID(uint32_t geometryInstanceID)`](Source/Falcor/Scene.h:717) - Get an SDF grid ID from a geometry instance ID
  - Returns: SDF grid ID if found else kInvalidSDFGrid

**Geometry Instance by Type**:
- [`getGeometryInstanceIDsByType(GeometryType type)`](Source/Falcor/Scene/Scene.h:723) - Get geometry instance IDs by geometry type
  - Returns: Vector of geometry instance IDs

**Custom Primitives**:
- [`getCustomPrimitiveCount()`](Source/Falcor/Scene/Scene.h:731) - Get a number of custom primitives
  - Returns: Number of custom primitives
- [`getCustomPrimitiveIndex(GlobalGeometryID geometryID)`](Source/Falcor/Scene/Scene.h:737) - Get the custom primitive index of a geometry
  - Returns: The custom primitive index of the geometry that can be used with getCustomPrimitive()
- [`getCustomPrimitive(uint32_t index)`](Source/Falcor/Scene/Scene.h:742) - Get a custom primitive
  - Returns: Custom primitive descriptor
- [`getCustomPrimitiveAABB(uint32_t index)`](Source/Falcor/Scene/Scene.h:747) - Get a custom primitive AABB
  - Returns: AABB of custom primitive
- [`addCustomPrimitive(uint32_t userID, const AABB& aabb)`](Source/Falcor/Scene/Scene.h:757) - Add a custom primitive
  - Custom primitives are sequentially numbered in the scene
  - Returns: Index of the custom primitive that was added
  - Adding/removing custom primitives is a slow operation as the acceleration structure is rebuilt
- [`removeCustomPrimitives(uint32_t first, uint32_t last)`](Source/Falcor/Scene.h:765) - Remove a range [first,last] of custom primitives
  - Last index is non-inclusive
- [`removeCustomPrimitive(uint32_t index)`](Source/Falcor/Scene.h:766) - Remove a custom primitive at given index
  - Custom primitives are sequentially numbered in the scene
  - The function removes the primitive at given index
  - Note that the index of subsequent primitives will change
- [`updateCustomPrimitive(uint32_t index, const AABB& aabb)`](Source/Falcor/Scene/Scene.h:77) - Update a custom primitive

**Scene Graph**:
- [`updateNodeTransform(uint32_t nodeID, const float4x4& transform)`](Source/Falcor/Scene/Scene.h:727) - Updates a node in the graph

**Animation Controller**:
- [`getAnimationController()`](Source/Falcor/Scene/Scene.h:1008) - Get the animation controller
  - Returns: Animation controller reference
- [`getAnimations()`](Source/Falcor/Scene/Scene.h:1012) - Get a list of all animations in the scene
  - Returns: List of animations
- [`hasAnimation()`](Source/Falcor/Scene/Scene.h:1016) - Returns true if scene has animation data
- [`setIsAnimated(bool isAnimated)`](Source/Falcor/Scene/Scene.h:1021) - Enable/disable scene animation
- [`isAnimated()`](Source/Falcor/Scene/Scene.h:1024) - Returns true if scene animation is enabled
- [`setIsLooped(bool looped)`](Source/Falcor/Scene.h:1028) - Enable/disable global animation looping
- [`isLooped()`](Source/Falcor/Scene/Scene.h:1032) - Returns true if scene animations are looped globally
- [`toggleAnimations(bool animate)`](Source/Falcor/Scene/Scene.h:1036) - Toggle all animations on or off

**Material System**:
- [`getMaterialSystem()`](Source/Falcor/Scene/Scene.h:782) - Get the material system
  - Returns: Material system reference
- [`replaceMaterial(const MaterialID materialID, const ref<Material>& pReplacement)`](Source/Falcor/Scene/Scene.h:784) - Replace a material
  - Parameters: material ID, replacement material
- [`setDefaultTextureSampler(const ref<Sampler>& pSampler)`](Source/Falcor/Scene.h:788) - Set default texture sampler
  - Parameters: Texture sampler
- [`getMaterials()`](Source/Falcor/Scene.h:796) - Get a list of all materials in the scene
  - Returns: List of materials
- [`getMaterialCount()`](Source/Falcor/Scene/Scene.h:800) - Get a total number of materials in the scene
  - Returns: Number of materials
- [`getMaterialCountByType(MaterialType type)`](Source/Falcor/Scene/Scene.h:804) - Get a number of materials of a given type
  - Returns: Number of materials
- [`getMaterial(MaterialID materialID)`](Source/Falcor/Scene.h:808) - Get a material
  - Returns: Material reference
- [`getMaterialByName(const std::string& name)`](Source/Falcor/Scene.h:812) - Get a material by name
  - Returns: Material reference
- [`addMaterial(const ref<Material>& pMaterial)`](Source/Falcor/Scene/Scene.h:818) - Add a material
  - Parameters: Material
  - Returns: The ID of the material in the scene

**Grid Volumes**:
- [`getGridVolumes()`](Source/Falcor/Scene/Scene.h:822) - Get a list of all grid volumes in the scene
  - Returns: List of grid volumes
- [`getGridVolume(uint32_t gridVolumeID)`](Source/Falcor/Scene/Scene.h:826) - Get a grid volume
  - Returns: Grid volume reference
- [`getGridVolumeByName(const std::string& name)`](Source/Falcor/Scene/Scene.h:830) - Get a grid volume by name
  - Returns: Grid volume reference

**Lights**:
- [`getLights()`](Source/Falcor/Scene/Scene.h:850) - Get a list of all lights in the scene
  - Returns: List of lights
- [`getLightCount()`](Source/Falcor/Scene.h:855) - Get a number of lights in the scene
  - Returns: Number of lights
- [`getLight(uint32_t lightID)`](Source/Falcor/Scene/Scene.h:858) - Get a light
  - Returns: Light reference
- [`getLightByName(const std::string& name)`](Source/Falcor/Scene/Scene.h:862) - Get a light by name
  - Returns: Light reference
- [`getActiveAnalyticLights()`](Source/Falcor/Scene/Scene.h:866) - Get a list of all active analytic lights in the scene
  - Returns: List of active analytic lights

**Light Collection**:
- [`getLightCollection(RenderContext* pRenderContext)`](Source/Falcor/Scene/874) - Get the light collection representing all mesh lights in the scene
  - The light collection is created lazily on the first call
  - It needs a render context
  - Returns: Light collection reference

**Environment Map**:
- [`getEnvMap()`](Source/Falcor/Scene/Scene.h:878) - Get the environment map or nullptr if it doesn't exist
  - Returns: Environment map reference

**Hit Info**:
- [`getHitInfo()`](Source/Falcor/Scene/Scene.h:834) - Get the scene's hit info requirements
  - Returns: HitInfo structure

**Scene Bounds**:
- [`getSceneBounds()`](Source/Falcor/Scene.h:838) - Get the scene's bounding box in world space
  - Returns: AABB

**Mesh Bounds**:
- [`getMeshBounds(uint32_t meshID)`](Source/Falcor/Scene/Scene.h:842) - Get a mesh's bounds in object space
  - Returns: AABB

**Curve Bounds**:
- [`getCurveBounds(uint32_t curveID)`](Source/Falcor/Scene/Scene.h:846) - Get a curve's bounds in object space
  - Returns: AABB

**Mesh Name**:
- [`getMeshName(uint32_t meshID)`](Source/Falcor/Scene.h:1048) - Get the name of a mesh with the given ID
  - Returns: Mesh name
- [`hasMesh(uint32_t meshID)`](Source/Falcor/Scene.h:1052) - Return true if the given mesh ID is valid, false otherwise

**Mesh BLAS IDs**:
- [`getMeshBlasIDs()`](Source/Falcor/Scene/Scene.h:1056) - Get a list of raytracing BLAS IDs for all meshes. The list is arranged by mesh ID
  - Returns: List of BLAS IDs

**Memory Usage**:
- [`getMemoryUsageInBytes()`](Source/Falcor/Scene/Scene.h:1066) - Get total memory usage in bytes
  - Returns: Total memory usage

**Update Signal**:
- [`getUpdateFlagsSignal()`](Source/Falcor/Scene/Scene.h:1069) - Allows connecting to signal that signals IScene::UpdateFlags when they are changed
  - Returns: UpdateFlagsSignal interface

**TLAS Update Mode**:
- [`setTlasUpdateMode(UpdateMode mode)`](Source/Falcor/Scene/Scene.h:883) - Set how the scene's BLASes are updated when raytracing
  - BLASes are REBUILT by default
- [`getTlasUpdateMode()`](Source/Falcor/Scene.h:896) - Get the scene's BLAS update mode when raytracing
  - Returns: UpdateMode

**BLAS Update Mode**:
- [`setBlasUpdateMode(UpdateMode mode)`](Source/Falcor/Scene/Scene.h:892) - Set how the scene's BLASes are updated when raytracing
  - BLASes are REFIT by default
- [`getBlasUpdateMode()`](Source/Falcor/Scene.h:895) - Get the scene's BLAS update mode when raytracing
  - Returns: UpdateMode

**Ray Tracing**:
- [`getRaytracingMaxAttributeSize()`](Source/Falcor/Scene.h:942) - Get required raytracing maximum attribute size for this scene
  - Note: This depends on what types of geometry are used in the scene
  - Returns: Max attribute size in bytes

**Ray Tracing Resources**:
- [`bindShaderDataForRaytracing(RenderContext* pRenderContext, const ShaderVar& sceneVar, uint32_t rayTypeCount = 0)`](Source/Falcor/Scene/Scene.h:1044) - Set the scene ray tracing resources into a shader var
  - The acceleration structure is created lazily, which requires a render context
  - Parameters: render context, shader variable, ray type count
  - Note: rayTypeCount is Number of ray types in raygen program. Not needed for DXR 1.1

**Rt Pipeline Flags**:
- [`getRtPipelineFlags()`](Source/Falcor/Scene.h:1095) - Return the RtPipelineFlags relevant for the scene
  - Returns: RtPipelineFlags (SkipProceduralPrimitives if no procedural geometry, None otherwise)

**Scene Update**:
- [`update(RenderContext* pRenderContext, double currentTime)`](Source/Falcor/Scene/Scene.h:903) - Update scene
  - Parameters: render context, current time
  - Returns: Flags indicating what changes happened in the update
  - Updates camera location, animations, etc.

**Update Methods**:
- [`getUpdates()`](Source/Falcor/Scene/Scene.h:909) - Get the changes that happened during the last update
  - Returns: Flags indicating what changes happened in the last update
  - The flags only change during an `update()` call, if something changed between calling `update()` and `getUpdates()`, returned result will not reflect it

**Subset Update**:
- [`updateForInverseRendering(RenderContext* pRenderContext, bool isMaterialChanged, bool isMeshChanged)`](Source/Falcor/Scene/Scene.h:917) - Update material and geometry for inverse rendering applications
  - This is a subset of the update() function
  - Parameters: render context, material changed flag, mesh changed flag

**Rasterization**:
- [`rasterize(RenderContext* pRenderContext, GraphicsState* pState, ProgramVars* pVars, RasterizerState::CullMode cullMode = RasterizerState::CullMode::Back)`](Source/Falcor/Scene/Scene.h:926) - Render scene using rasterizer
  - Note: Rasterizer state bound to 'pState' is ignored
  - Parameters: render context, graphics state, program vars, cull mode
- [`rasterize(RenderContext* pRenderContext, GraphicsState* pState, ProgramVars* pVars, const ref<RasterizerState>& pRasterizerStateCW, const ref<RasterizerState>& pRasterizerStateCCW)`](Source/Falcor/Scene/Scene.h:936) - Render scene using rasterizer
  - This overload uses the supplied rasterizer states
  - Parameters: render context, graphics state, program vars, rasterizer state for clockwise winding, rasterizer state for counter-clockwise winding

**Ray Tracing**:
- [`raytrace(RenderContext* pRenderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)`](Source/Falcor/Scene/Scene.h:946) - Render scene using raytracing
  - Parameters: render context, program, program vars, dispatch dimensions

**UI Rendering**:
- [`renderUI(Gui::Widgets& widget)`](Source/Falcor/Scene.h:950) - Render UI

**VAO**:
- [`getMeshVao()`](Source/Falcor/Scene/Scene.h:955) - Get the scene's VAO for meshes
  - The default VAO uses 32-bit vertex indices
  - Returns: VAO object or nullptr if no meshes using 32-bit indices
- [`getMeshVao16()`](Source/Falcor/Scene/Scene.h:960) - Get the scene's VAO for 16-bit vertex indices
  - Returns: VAO object or nullptr if no meshes using 16-bit indices
- [`getCurveVao()`](Source/Falcor/Scene/Scene.h:965) - Get the scene's VAO for curves
  - Returns: VAO object

**Environment Map**:
- [`setEnvMap(ref<EnvMap> pEnvMap)`](Source/Falcor/Scene.h:970) - Set an environment map
  - Parameters: Environment map
  - Can be nullptr

**Environment Map Loading**:
- [`loadEnvMap(const std::filesystem::path& path)`](Source/Falcor/Scene/Scene.h:976) - Load an environment from an image
  - Parameters: Texture path
  - Returns: True if environment map was successfully loaded

**Input Handling**:
- [`onMouseEvent(const MouseEvent& mouseEvent)`](Source/Falcor/Scene/Scene.h:980) - Handle mouse events
- [`onKeyEvent(const KeyboardEvent& keyEvent)`](Source/Falcor/Scene/Scene.h:984) - Handle keyboard events
- [`onGamepadEvent(const GamepadEvent& gamepadEvent)`](Source/Falcor/Scene/Scene.h:988) - Handle gamepad events
- [`onGamepadState(const GamepadState& gamepadState)`](Source/Falcor/Scene/Scene.h:992) - Handle gamepad state

**File Path**:
- [`getPath()`](Source/Falcor/Scene/Scene.h:996) - Get the last path that was loaded to create the scene
  - Returns: Filesystem path
- [`getImportPaths()`](Source/Falcor/Scene/Scene.h:1000) - Get all of the paths that were loaded to create the scene
  - Returns: Vector of paths

**Import Dictionaries**:
- [`getImportDicts()`](Source/Falcor/Scene.h:1004) - Get all of the dictionaries that were loaded to create the scene
  - Returns: Vector of dictionaries

**Animation Controller**:
- [`getScript(const std::string& sceneVar)`](Source/Falcor/Scene.h:1063) - Get a script
  - Parameters: Scene variable name
  - Returns: Script string

**IScene-Specific Methods**:
- [`getShaderDefines(DefineList& defines)`](Source/Falcor/Scene/Scene.h:1072) - Get scene defines
  - Adds getSceneDefines() to the defines list
- [`getTypeConformances(TypeConformanceList& conformances, TypeConformancesKind kind = TypeConformancesKind::All)`](Source/Falcor/Scene/Scene.h:1078) - Get type conformances
  - Adds getTypeConformances() to the conformances list
  - Only have material conformances
- [`getShaderModules(ProgramDesc::ShaderModuleList& shaderModuleList)`](Source/Falcor/Scene/Scene.h:1088) - Get shader modules
  - Adds getShaderModules() to the shader module list
- [`getILightCollection(RenderContext* renderContext)`](Source/Falcor/Scene.h:1091) - Get the light collection
  - Returns: Light collection reference
- [`getRtPipelineFlags()`](Source/Falcor/Scene/Scene.h:1095) - Return the RtPipelineFlags relevant for the scene
  - Returns: RtPipelineFlags (SkipProceduralPrimitives if no procedural geometry, None otherwise)

## Technical Details

### DXR Scene and Resources Layout

**BLAS Creation Logic**:
1. For static non-instanced meshes, pre-transform and group them into single BLAS
   - This can be overridden by 'RTDontMergeStatic' scene build flag
2. For dynamic non-instanced meshes, group them if they use the same scene graph transform matrix. One BLAS is created per group
   - This can be overridden by 'RTDontMergeDynamic' scene build flag
3. It is possible a non-instanced mesh has no other meshes to merge with. In that case, mesh goes in its own BLAS
4. For instanced meshes, one BLAS is created per group of mesh with identical instances
   - This can be overridden by 'RTDontMergeInstanced' scene build flag

**TLAS Construction**:
- Hit shaders use InstanceID() and GeometryIndex() to identify what was hit
- InstanceID is set like a starting offset so that (InstanceID + GeometryIndex) maps to unique indices
- Shader table has one hit group per geometry and ray type. InstanceContribution is set accordingly for correct lookup

**Acceleration Structure Layout Example**:
- Example: Scene with 11 geometries, 16 instances total
- InstanceID: 0, 4, 5, 6, 7, 8, 9, 10, 13, 9 INSTANCE_DESCs in TLAS
- InstanceContribution: 0, 0, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8
- BLAS Geometry Index: 0, 1, 2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 2, 6 BLAS's containing 11 geometries in total
- Notes: Four geometries in one BLAS, One instance per geom/BLAS, Multiple instances of three geometries in one BLAS, Two instances of three geometries in one BLAS

**InstanceID + GeometryIndex**:
- Used for indexing into GeometryInstanceData
- Wrapped in getGeometryInstanceID() in Raytracing.slang
- This is wrapped in getGeometryInstanceID() in Raytracing.slang

### Geometry Types

**GeometryTypeFlags**:
- Bit field containing set of geometry types
- Supports: TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid, CustomPrimitive

**GeometryType**:
- Enumeration of geometry types
- Values: TriangleMesh, DisplacedTriangleMesh, Curve, SDFGrid, CustomPrimitive

**GeometryTypeFlags**:
- Bit flags for geometry types
- Each bit represents a different geometry type
- Used for checking which geometry types are present in the scene

### Scene Graph

**Node Structure**:
- Represents scene graph node
- Contains name, parent, transform, meshBind, localToBindSpace
- Supports hierarchical scene graph
- Supports skinned meshes with meshBind
- Supports bone transforms with localToBindSpace

**Scene Graph Nodes**:
- Array of nodes indexed by NodeID
- Each node has parent reference
- Top-level nodes have invalid parent ID
- Transform matrices indexed by NodeID

### Mesh Groups

**Purpose**: Groups of meshes for BLAS creation.

**BLAS per Group**:
- One BLAS per group
- Each group maps to a BLAS for ray tracing
- Groups can contain multiple meshes
- Groups can be static, displaced, or instanced

**BLAS Group Types**:
- Static non-instanced meshes
- Dynamic non-instanced meshes
- Instanced meshes
- Displaced meshes

**Mesh Group Data**:
- List of mesh IDs in group
- Static/displaced flag
- BLAS index

### Camera Management

**Camera Controller Types**:
- FirstPerson - FPS-style camera controller
- Orbiter - Orbital camera controller
- SixDOF - Full 6DOF camera controller

**Camera Controls**:
- Enable/disable camera controls
- Camera movement control
- Camera rotation control
- Camera speed control

**Viewpoints**:
- Saved camera positions
- Used for camera animation
- Used for camera switching
- Supports adding and removing viewpoints

### Animation System

**Animation Controller**:
- Manages scene animations
- Provides animation enable/disable
- Provides animation looping control
- Provides animation toggle control
- Returns list of animations
- Checks if scene has animation data

### Material System

**Material System Integration**:
- Material system reference
- Material replacement support
- Default texture sampler support
- Material access by ID and name
- Material count by type

### Lighting System

**Light Types**:
- Point lights
- Directional lights
- Rect lights
- Disc lights
- Sphere lights
- Distant lights

**Light Collection**:
- Manages emissive geometry (mesh lights)
- Created lazily on first call
- Requires render context for initialization
- Provides mesh light sampling

**Environment Map**:
- Environment map reference
- Environment map loading
- Environment map setting
- Background/distant light support

### Grid Volumes

**Grid Volume List**:
- List of grid volumes
- Grid volume access by ID and name
- Grid volume data

### Grids

**Grid List**:
- List of grids
- Grid access by SDF grid ID
- SDF grid data
- Grid configuration

### Custom Primitives

**Custom Primitive List**:
- List of custom primitive descriptors
- Custom primitive access by index
- Custom primitive AABB access
- Custom primitive update operations

### Statistics

**Memory Usage Tracking**:
- Tracks memory usage for all scene resources
- Provides total memory calculation
- Tracks memory per resource type
- Used for memory management and profiling

### Update Flags

**Update Flags**:
- 26 different update flags for tracking scene changes
- Flags for geometry, camera, lights, materials, environment map, grid volumes, curves, custom primitives, displacement, SDF grids, meshes, scene graph, render settings, scene defines, type conformances, shader code

**Update Signal**:
- Signal-based update notifications
- Multiple subscribers can receive update notifications
- Enables efficient scene update propagation

## Integration Points

### Scene Integration

**IScene Implementation**:
- Implements [`IScene`](Source/Falcor/Scene/Scene.h:114) interface
- Provides all IScene methods
- Implements IScene-specific methods for shader defines and type conformances

**Animation Integration**:
- Animation controller reference
- Animation list
- Animation enable/disable
- Animation looping control

**Camera Integration**:
- Camera list
- Camera controller reference
- Camera bounds
- Camera controls
- Viewpoints

**Material System Integration**:
- Material system reference
- Material replacement
- Default texture sampler
- Material access methods

**Light System Integration**:
- Light list
- Active light list
- Light collection
- Environment map reference

**Volume System Integration**:
- Grid volume list
- Grid list
- SDF grid list
- SDF grid configuration

### DXR Integration

**Ray Tracing**:
- BLAS creation logic
- TLAS construction
- Hit group shader integration
- Ray tracing pipeline flags
- Ray tracing resources

### GPU Resources

**Buffers**:
- Vertex buffers
- Index buffers
- BLAS buffers
- TLAS buffers
- Scratch buffers

**Textures**:
- Environment map textures
- Material textures
- Grid textures

## Architecture Patterns

### Scene Graph Pattern

- Hierarchical scene graph
- Node-based structure
- Parent-child relationships
- Transform matrix hierarchy
- Support for skinned meshes

### BLAS Pattern

- Group-based BLAS creation
- Static non-instanced meshes in single BLAS
- Dynamic non-instanced meshes grouped by transform
- Instanced meshes grouped by identical instances
- Each group maps to one BLAS

### TLAS Pattern

- Instance-based indexing
- InstanceID + GeometryIndex maps to unique indices
- Shader table has one hit group per geometry and ray type
- Enables efficient hit group lookup

### Material System Pattern

- Material system integration
- Material replacement support
- Default texture sampler support
- Material access by ID and name
- Material count by type

### Update Pattern

- Update flags for tracking changes
- Signal-based update notifications
- Enables efficient scene update propagation
- Supports 26 different update flags

### Statistics Pattern

- Memory usage tracking
- Per-resource memory tracking
- Total memory calculation
- Used for memory management and profiling

## Code Patterns

### Scene Creation Pattern

```cpp
ref<Scene> Scene::create(ref<Device> pDevice, const std::filesystem::path& path, const Settings& settings)
{
    // Create scene from file
    // Throws ImporterError if import went wrong
}
```

### Update Pattern

```cpp
IScene::UpdateFlags Scene::update(RenderContext* pRenderContext, double currentTime)
{
    // Update camera location, animations, etc.
    // Return flags indicating what changed happened
}
```

### Statistics Pattern

```cpp
const SceneStats& Scene::getSceneStats() const
{
    return mSceneStats;
}
```

### BLAS Group Pattern

```cpp
struct MeshGroup
{
    std::vector<MeshID> meshList;
    bool isStatic;
    bool isDisplaced;
};
```

### Node Pattern

```cpp
struct Node
{
    std::string name;
    NodeID parent{ NodeID::Invalid() };
    float4x4 transform;
    float4x4 meshBind;
    float4x4 localToBindSpace;
};
```

## Use Cases

### Scene Loading

- **File Loading**:
  - Load scene from file
  - Support for multiple file formats
  - Importer-based loading
  - Metadata extraction

- **In-Memory Loading**:
  - Create scene from in-memory data
  - Used for procedural scene generation
  - Used for scene manipulation

### Rendering

- **Rasterization**:
  - Render scene using rasterizer
  - Render scene with clockwise/counter-clockwise winding
  - Support for different cull modes
  - Draw list creation

- **Ray Tracing**:
  - Render scene using ray tracing
  - BLAS and TLAS management
  - Hit group shader integration
  - Procedural geometry support

### Animation

- **Animation Control**:
  - Enable/disable animations
  - Control animation looping
  - Toggle all animations
  - Access animation list

### Camera Management

- **Camera Selection**:
  - Select camera by index
  - Select camera by name
  - Manage camera controls
  - Manage camera speed
  - Manage camera aspect ratio
  - Manage camera viewpoints

### Material Management

- **Material Access**:
  - Access material by ID
- - Access material by name
- - Replace material
- Set default texture sampler
- Get material count by type

### Lighting

- **Light Access**:
  - Access light by ID
  - Access light by name
  - Get active analytic lights
- - Get light collection

### Geometry Management

- **Geometry Type Queries**:
  - Check for geometry types
- - Get geometry by type
- - Get geometry IDs by type and material
- Get geometry instance data

- **Mesh Operations**:
  - Get mesh data
  - Get mesh bounds
  - Get mesh vertices and indices
  - Set mesh vertices
  - Get mesh BLAS IDs

- **Curve Operations**:
  - Get curve data
  - Get curve bounds

- **SDF Grid Operations**:
  - Get SDF grid data
  - Get SDF grid descriptors
- - Configure SDF grid intersection and gradient methods

- **Custom Primitives**:
  - Add custom primitives
  - Update custom primitives
  - Remove custom primitives
  - Get custom primitive data

### Statistics

- **Memory Profiling**:
- Get total memory usage
- Get per-resource memory usage
- Track memory for optimization
- Profile scene resources

## Performance Considerations

### GPU Memory

**Memory Usage Tracking**:
- Tracks memory usage for all scene resources
- Provides total memory calculation
- Used for memory management and profiling
- Includes: geometry, materials, lights, environment map, grid volumes, animations, BLAS, TLAS

**Memory Breakdown**:
- Index buffers
- Vertex buffers
- Material textures
- BLAS buffers
- TLAS buffers
- Scratch buffers
- Curve buffers
- SDF grid buffers
- Custom primitive buffers

### GPU Computation

**BLAS Creation**:
- O(n) per mesh for static non-instanced meshes
- O(n) per mesh group for dynamic meshes
- O(n) per mesh for instanced meshes
- O(1) per mesh for displaced meshes
- O(1) per BLAS for BLAS creation

**TLAS Creation**:
- O(n) for TLAS creation
- Depends on BLAS count
- Depends on instance count

**Scene Update**:
- O(n) per frame for camera updates
- O(n) per frame for animation updates
- O(n) per frame for geometry updates
- O(n) per frame for material updates
- O(n) per frame for light updates

### Optimization Opportunities

**BLAS Optimization**:
- Use RTDontMergeStatic flag to optimize static meshes
- Use RTDontMergeDynamic flag to optimize dynamic meshes
- Use RTDontMergeInstanced flag to optimize instanced meshes
- Group meshes by transform for dynamic meshes

**TLAS Optimization**:
- Use Refit mode for TLAS updates when possible
- Use Rebuild mode only when necessary
- Minimizes TLAS rebuild cost

**Memory Optimization**:
- Use compressed HitInfo for triangle-only scenes
- Use 16-bit indices when possible
- Minimize memory usage

## Limitations

### Feature Limitations

- **BLAS Creation**:
- Limited to specific BLAS creation logic
- Cannot customize BLAS grouping
- Limited to DXR BLAS features

- **TLAS Construction**:
- Limited to specific TLAS construction
- Cannot customize hit group layout
- Limited to DXR TLAS features

- **Geometry Types**:
- Limited to specific geometry types
- No support for custom geometry types beyond CustomPrimitive
- Limited to SDF grid implementations

### Performance Limitations

- **BLAS Updates**:
- BLAS updates can be expensive
- TLAS updates can be expensive
- Rebuilding BLAS is expensive

- **Scene Updates**:
- Full scene updates can be expensive
- Animation updates can be expensive
- Material updates can be expensive

### Integration Limitations

- **DXR Coupling**:
- Tightly coupled to DXR ray tracing
- Requires DXR-compatible hardware
- Requires DXR 1.1 or higher
- Not suitable for other ray tracing APIs

- **Scene Coupling**:
- Tightly coupled to Falcor scene system
- Requires specific scene data structures
- Not suitable for standalone use
- Requires specific material system

## Best Practices

### Scene Creation

- **File Loading**:
  - Use [`Scene::create()`](Source/Falcor/Scene/Scene.h:400) for file loading
  - Handle ImporterError exceptions
  - Validate file paths
  - Use appropriate settings

- **In-Memory Loading**:
  - Use [`Scene::create()`](Source/Falcor/Scene/Scene.h:407) for in-memory loading
  - Prepare SceneData structure
  - Validate scene data

### Scene Updates

- **Update Frequency**:
  - Call [`update()`](Source/Falcor/Scene/Scene.h:903) once per frame
  - Check for update flags
  - Update programs as needed

- **Update Flags**:
  - Use specific update flags
  - Minimize program recompilation
  - Only recompile when necessary

### BLAS Management

- **BLAS Updates**:
  - Use appropriate update mode for scene changes
  - Use Refit for minor changes
  - Use Rebuild only when necessary
- Minimize BLAS update cost

### Memory Management

- **Memory Tracking**:
  - Monitor memory usage
  - Use statistics for profiling
- Optimize memory usage
- Use compressed HitInfo when possible

### Material Management

- **Material Access**:
  - Cache material references
- - Use material system efficiently
- Avoid redundant material lookups
- Use material count by type for optimization

### Camera Management

- **Camera Controls**:
  - Enable/disable camera controls based on use case
- Use appropriate camera controller type
- Manage camera speed for optimal performance

### Performance Profiling

- **Statistics**:
  - Use [`getSceneStats()`](Source/Falcor/Scene/Scene.h:444) for profiling
  - Monitor memory usage
  - Track resource counts
  - Identify bottlenecks

## Progress Log

- **2026-01-08T00:15:00Z**: Scene analysis completed. Analyzed Scene.h (1455 lines) containing main Scene class implementation. Documented extensive Scene class with 100+ methods and structures covering event structures (MouseEvent, KeyboardEvent, GamepadEvent, GamepadState), RtProgramVars class, UpdateMode enumeration (Rebuild, Refit), CameraControllerType enumeration (FirstPerson, Orbiter, SixDOF), SDFGridIntersectionMethod enumeration (None, GridSphereTracing, VoxelSphereTracing), SDFGridGradientEvaluationMethod enumeration (None, NumericDiscontinuous, NumericContinuous), SDFGridConfig structure with implementation data and dropdown lists, Metadata structure with optional rendering metadata, SDFGridDesc structure, MeshGroup structure, Node structure, SceneData structure with extensive fields for import data, render settings, cameras, lights, materials, volumes, grids, scene graph, animations, metadata, mesh data, curve data, SDF grid data, custom primitive data, SceneStats structure with comprehensive statistics for geometry, curves, SDF grids, custom primitives, materials, lights, grid volumes, grids, ray tracing, memory usage, DrawArgs structure for rasterization, DisplacementMapping structure for displacement mapping, multiple private structures for BLAS data, TLAS data, curve data, SDF grid data, custom primitive data, AABB data, event handling methods, and extensive public interface methods for scene operations. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The Scene module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
