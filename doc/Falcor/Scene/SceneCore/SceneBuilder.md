# SceneBuilder - Scene Builder

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneBuilder.h** - Scene builder header (849 lines)
- [x] **SceneBuilder.cpp** - Scene builder implementation

### External Dependencies

- **Scene/Scene.h** - Scene main class
- **Scene/SceneCache.h** - Scene cache
- **Scene/SceneIDs.h** - Scene ID definitions
- **Scene/Transform.h** - Transform utilities
- **Scene/TriangleMesh.h** - Triangle mesh
- **Scene/VertexAttrib.slangh** - Vertex attribute definitions
- **Scene/SceneTypes.slang** - Scene type definitions
- **Scene/Material/MaterialTextureLoader.h** - Material texture loader
- **Core/Macros** - Macros
- **Core/AssetResolver** - Asset resolver
- **Core/API/VAO** - Vertex array object
- **Utils/Math/AABB** - Axis-aligned bounding box
- **Utils/Math/Vector** - Vector
- **Utils/Math/Matrix** - Matrix
- **Utils/Settings/Settings** - Settings
- **pybind11/pytypes.h** - Python binding types
- **filesystem** - Filesystem utilities
- **memory** - Memory utilities
- **string** - String utilities
- **vector** - Vector utilities

## Module Overview

SceneBuilder is a comprehensive scene construction system that builds Scene objects from imported data. It provides a flexible API for adding meshes, curves, SDF grids, custom primitives, materials, lights, cameras, volumes, animations, and scene graph nodes. The SceneBuilder handles scene optimization, mesh processing, material management, texture loading, scene graph construction, and scene caching. It supports various build flags to control scene construction behavior including material merging, tangent space generation, texture color space interpretation, mesh merging, ray tracing optimization, displacement mapping, hit info compression, and scene caching. The SceneBuilder is the primary interface for constructing scenes from imported files or in-memory data.

## Component Specifications

### Flags Enumeration

**Purpose**: Flags that control how the scene will be built. They can be combined together.

**Values**:
- [`None`](Source/Falcor/Scene/SceneBuilder.h:62) - None (default)
- [`DontMergeMaterials`](Source/Falcor/Scene/SceneBuilder.h:63) - Don't merge materials that have same properties. Use this option to preserve original material names
- [`UseOriginalTangentSpace`](Source/Falcor/Scene/SceneBuilder.h:64) - Use original tangent space that was loaded with mesh. By default, ignore it and use MikkTSpace to generate tangent space. Always generate tangent space if it is missing
- [`AssumeLinearSpaceTextures`](Source/Falcor/Scene/SceneBuilder.h:65) - By default, textures representing colors (diffuse/specular) are interpreted as sRGB data. Use this flag to force linear space for color textures
- [`DontMergeMeshes`](Source/Falcor/Scene/SceneBuilder.h:66) - Preserve original list of meshes in scene, don't merge meshes with same material. This flag only applies to scenes imported by 'AssimpImporter'
- [`UseSpecGlossMaterials`](Source/Falcor/Scene/SceneBuilder.h:67) - Set materials to use Spec-Gloss shading model. Otherwise default is Spec-Gloss for OBJ, Metal-Rough for everything else
- [`UseMetalRoughMaterials`](Source/Falcor/Scene/SceneBuilder.h:68) - Set materials to use Metal-Rough shading model. Otherwise default is Spec-Gloss for OBJ, Metal-Rough for everything else
- [`NonIndexedVertices`](Source/Falcor/Scene/SceneBuilder.h:69) - Convert meshes to use non-indexed vertices. This requires more memory but may increase performance
- [`Force32BitIndices`](Source/Falcor/Scene/SceneBuilder.h:70) - Force 32-bit indices for all meshes. By default, 16-bit indices are used for small meshes
- [`RTDontMergeStatic`](Source/Falcor/Scene/SceneBuilder.h:71) - For raytracing, don't merge all static non-instanced meshes into single pre-transformed BLAS
- [`RTDontMergeDynamic`](Source/Falcor/Scene/SceneBuilder.h:72) - For raytracing, don't merge dynamic non-instanced meshes with identical transforms into single BLAS
- [`RTDontMergeInstanced`](Source/Falcor/Scene/SceneBuilder.h:73) - For raytracing, don't merge instanced meshes with identical instances into single BLAS
- [`FlattenStaticMeshInstances`](Source/Falcor/Scene/SceneBuilder.h:74) - Flatten static mesh instances by duplicating mesh data and composing transformations. Animated instances are not affected. Can lead to a large increase in memory use
- [`DontOptimizeGraph`](Source/Falcor/Scene/SceneBuilder.h:75) - Don't optimize scene graph to remove unnecessary nodes
- [`DontOptimizeMaterials`](Source/Falcor/Scene/SceneBuilder.h:76) - Don't optimize materials by removing constant textures. The optimizations are lossless so should generally be enabled
- [`DontUseDisplacement`](Source/Falcor/Scene/SceneBuilder.h:77) - Don't use displacement mapping
- [`UseCompressedHitInfo`](Source/Falcor/Scene/SceneBuilder.h:78) - Use compressed hit info (on scenes with triangle meshes only)
- [`TessellateCurvesIntoPolyTubes`](Source/Falcor/Scene/SceneBuilder.h:79) - Tessellate curves into poly-tubes (the default is linear swept spheres)
- [`UseCache`](Source/Falcor/Scene/SceneBuilder.h:81) - Enable scene caching. This caches runtime scene representation on disk to reduce load time
- [`RebuildCache`](Source/Falcor/Scene/SceneBuilder.h:82) - Rebuild scene cache
- [`Default`](Source/Falcor/Scene/SceneBuilder.h:84) - Default (None)

**Usage**:
- Combine flags using bitwise OR
- Control scene construction behavior
- Optimize for specific use cases
- Enable/disable features

### Mesh Structure

**Purpose**: Mesh description used by importers to add new meshes. The description is then processed by scene builder into an optimized runtime format. The frequency of each vertex attribute is specified individually, but note that an index list is always required.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:110) - `std::string` - The mesh's name
- [`faceCount`](Source/Falcor/Scene/SceneBuilder.h:111) - `uint32_t` - The number of primitives mesh has
- [`vertexCount`](Source/Falcor/Scene/SceneBuilder.h:112) - `uint32_t` - The number of vertices mesh has
- [`indexCount`](Source/Falcor/Scene/SceneBuilder.h:113) - `uint32_t` - The number of indices mesh has
- [`pIndices`](Source/Falcor/Scene/SceneBuilder.h:114) - `const uint32_t*` - Array of indices. The element count must match `indexCount`. This field is required
- [`topology`](Source/Falcor/Scene/SceneBuilder.h:115) - `Vao::Topology` - The primitive topology of mesh
- [`pMaterial`](Source/Falcor/Scene/SceneBuilder.h:116) - `ref<Material>` - The mesh's material. Can't be nullptr
- [`positions`](Source/Falcor/Scene/SceneBuilder.h:118) - `Attribute<float3>` - Array of vertex positions. This field is required
- [`normals`](Source/Falcor/Scene/SceneBuilder.h:119) - `Attribute<float3>` - Array of vertex normals. This field is required
- [`tangents`](Source/Falcor/Scene/SceneBuilder.h:120) - `Attribute<float4>` - Array of vertex tangents. This field is optional. If set to nullptr, or if BuildFlags::UseOriginalTangentSpace is not set, tangent space will be generated using MikkTSpace
- [`texCrds`](Source/Falcor/Scene/SceneBuilder.h:121) - `Attribute<float2>` - Array of vertex texture coordinates. This field is optional. If set to nullptr, all texCrds will be set to (0,0)
- [`curveRadii`](Source/Falcor/Scene/SceneBuilder.h:122) - `Attribute<float>` - Array of vertex curve radii. This field is optional
- [`boneIDs`](Source/Falcor/Scene/SceneBuilder.h:123) - `Attribute<uint4>` - Array of bone IDs. This field is optional. If it's set, that means that mesh is animated, in which case boneWeights is required
- [`boneWeights`](Source/Falcor/Scene/SceneBuilder.h:124) - `Attribute<float4>` - Array of bone weights. This field is optional. If it's set, that means that mesh is animated, in which case boneIDs is required
- [`isFrontFaceCW`](Source/Falcor/Scene/SceneBuilder.h:126) - `bool` - Indicate whether front-facing side has clockwise winding in object space
- [`isAnimated`](Source/Falcor/Scene/SceneBuilder.h:127) - `bool` - True if mesh vertices can be modified during rendering (e.g., skinning or inverse rendering)
- [`useOriginalTangentSpace`](Source/Falcor/Scene/SceneBuilder.h:128) - `bool` - Indicate whether to use original tangent space that was loaded with mesh. By default, ignore it and use MikkTSpace to generate tangent space
- [`mergeDuplicateVertices`](Source/Falcor/Scene/SceneBuilder.h:129) - `bool` - Indicate whether to merge identical vertices and adjust indices
- [`skeletonNodeId`](Source/Falcor/Scene/SceneBuilder.h:130) - `NodeID` - For skinned meshes, node ID of skeleton's world transform. If invalid, skeleton is based on mesh's own world position (Assimp behavior pre-multiplies instance transform)

**Nested Types**:

**AttributeFrequency Enumeration**:
- [`None`](Source/Falcor/Scene/SceneBuilder.h:96) - None
- [`Constant`](Source/Falcor/Scene/SceneBuilder.h:97) - Constant value for mesh. The element count must be 1
- [`Uniform`](Source/Falcor/Scene/SceneBuilder.h:98) - One value per face. The element count must match `faceCount`
- [`Vertex`](Source/Falcor/Scene/SceneBuilder.h:99) - One value per vertex. The element count must match `vertexCount`
- [`FaceVarying`](Source/Falcor/Scene/SceneBuilder.h:100) - One value per vertex per face. The element count must match `indexCount`

**Attribute<T> Template Structure**:
- [`pData`](Source/Falcor/Scene/SceneBuilder.h:106) - `const T*` - Pointer to attribute data
- [`frequency`](Source/Falcor/Scene/SceneBuilder.h:107) - `AttributeFrequency` - Attribute frequency

**Vertex Structure**:
- [`position`](Source/Falcor/Scene/SceneBuilder.h:198) - `float3` - Position
- [`normal`](Source/Falcor/Scene/SceneBuilder.h:199) - `float3` - Normal
- [`tangent`](Source/Falcor/Scene/SceneBuilder.h:200) - `float4` - Tangent
- [`texCrd`](Source/Falcor/Scene/SceneBuilder.h:201) - `float2` - Texture coordinate
- [`curveRadius`](Source/Falcor/Scene/SceneBuilder.h:202) - `float` - Curve radius
- [`boneIDs`](Source/Falcor/Scene/SceneBuilder.h:203) - `uint4` - Bone IDs
- [`boneWeights`](Source/Falcor/Scene/SceneBuilder.h:204) - `float4` - Bone weights

**VertexAttributeIndices Structure**:
- [`positionIdx`](Source/Falcor/Scene/SceneBuilder.h:209) - `uint32_t` - Position index
- [`normalIdx`](Source/Falcor/Scene/SceneBuilder.h:210) - `uint32_t` - Normal index
- [`tangentIdx`](Source/Falcor/Scene/SceneBuilder.h:211) - `uint32_t` - Tangent index
- [`texCrdIdx`](Source/Falcor/Scene/SceneBuilder.h:212) - `uint32_t` - Texture coordinate index
- [`curveRadiusIdx`](Source/Falcor/Scene/SceneBuilder.h:213) - `uint32_t` - Curve radius index
- [`boneIDsIdx`](Source/Falcor/Scene/SceneBuilder.h:214) - `uint32_t` - Bone IDs index
- [`boneWeightsIdx`](Source/Falcor/Scene/SceneBuilder.h:215) - `uint32_t` - Bone weights index

**Methods**:
- [`getAttributeIndex<T>(const Attribute<T>& attribute, uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:133) - Get attribute index based on frequency
- [`get<T>(const Attribute<T>& attribute, uint32_t index) const`](Source/Falcor/Scene/SceneBuilder.h:152) - Get attribute value by index
- [`get<T>(const Attribute<T>& attribute, uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:162) - Get attribute value by face and vertex
- [`getAttributeCount<T>(const Attribute<T>& attribute)`](Source/Falcor/Scene/SceneBuilder.h:172) - Get attribute count based on frequency
- [`getPosition(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:190) - Get position
- [`getNormal(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:191) - Get normal
- [`getTangent(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:192) - Get tangent
- [`getTexCrd(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:193) - Get texture coordinate
- [`getCurveRadii(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:194) - Get curve radius
- [`getVertex(uint32_t face, uint32_t vert) const`](Source/Falcor/Scene/SceneBuilder.h:218) - Get vertex by face and vertex
- [`getVertex(const VertexAttributeIndices& attributeIndices)`](Source/Falcor/Scene/SceneBuilder.h:231) - Get vertex by attribute indices
- [`getAttributeIndices(uint32_t face, uint32_t vert)`](Source/Falcor/Scene/SceneBuilder.h:244) - Get attribute indices by face and vertex
- [`hasBones() const`](Source/Falcor/Scene/SceneBuilder.h:257) - Check if mesh has bones

### ProcessedMesh Structure

**Purpose**: Pre-processed mesh data formatted such that it can directly be copied to global scene buffers.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:269) - `std::string` - Mesh name
- [`topology`](Source/Falcor/Scene/SceneBuilder.h:270) - `Vao::Topology` - Primitive topology
- [`pMaterial`](Source/Falcor/Scene/SceneBuilder.h:271) - `ref<Material>` - Material
- [`skeletonNodeId`](Source/Falcor/Scene/SceneBuilder.h:272) - `NodeID` - Forwarded from Mesh struct
- [`indexCount`](Source/Falcor/Scene/SceneBuilder.h:274) - `uint64_t` - Number of indices, or zero if non-indexed
- [`use16BitIndices`](Source/Falcor/Scene/SceneBuilder.h:275) - `bool` - True if indices are in 16-bit format
- [`isFrontFaceCW`](Source/Falcor/Scene/SceneBuilder.h:276) - `bool` - Indicate whether front-facing side has clockwise winding in object space
- [`isAnimated`](Source/Falcor/Scene/SceneBuilder.h:277) - `bool` - True if mesh vertices can be modified during rendering (e.g., skinning or inverse rendering)
- [`indexData`](Source/Falcor/Scene/SceneBuilder.h:278) - `std::vector<uint32_t>` - Vertex indices in either 32-bit or 16-bit format packed tightly, or empty if non-indexed
- [`staticData`](Source/Falcor/Scene/SceneBuilder.h:279) - `std::vector<StaticVertexData>` - Static vertex data
- [`skinningData`](Source/Falcor/Scene/SceneBuilder.h:280) - `std::vector<SkinningVertexData>` - Skinning vertex data

### Curve Structure

**Purpose**: Curve description.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:295) - `std::string` - The curve's name
- [`degree`](Source/Falcor/Scene/SceneBuilder.h:296) - `uint32_t` - Polynomial degree of curve; linear (1) by default
- [`vertexCount`](Source/Falcor/Scene/SceneBuilder.h:297) - `uint32_t` - The number of vertices
- [`indexCount`](Source/Falcor/Scene/SceneBuilder.h:298) - `uint32_t` - The number of indices (i.e., tube segments)
- [`pIndices`](Source/Falcor/Scene/SceneBuilder.h:299) - `const uint32_t*` - Array of indices. The element count must match `indexCount`. This field is required
- [`pMaterial`](Source/Falcor/Scene/SceneBuilder.h:300) - `ref<Material>` - The curve's material. Can't be nullptr
- [`positions`](Source/Falcor/Scene/SceneBuilder.h:302) - `Attribute<float3>` - Array of vertex positions. This field is required
- [`radius`](Source/Falcor/Scene/SceneBuilder.h:303) - `Attribute<float>` - Array of sphere radius. This field is required
- [`texCrds`](Source/Falcor/Scene/SceneBuilder.h:304) - `Attribute<float2>` - Array of vertex texture coordinates. This field is optional. If set to nullptr, all texCrds will be set to (0,0)

**Nested Types**:

**Attribute<T> Template Structure**:
- [`pData`](Source/Falcor/Scene/SceneBuilder.h:292) - `const T*` - Pointer to attribute data

### ProcessedCurve Structure

**Purpose**: Pre-processed curve data formatted such that it can directly be copied to global scene buffers.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:313) - `std::string` - Curve name
- [`topology`](Source/Falcor/Scene/SceneBuilder.h:314) - `Vao::Topology` - Primitive topology (LineStrip)
- [`pMaterial`](Source/Falcor/Scene/SceneBuilder.h:315) - `ref<Material>` - Material
- [`indexData`](Source/Falcor/Scene/SceneBuilder.h:317) - `std::vector<uint32_t>` - Index data
- [`staticData`](Source/Falcor/Scene/SceneBuilder.h:318) - `std::vector<StaticCurveVertexData>` - Static curve vertex data

### Node Structure

**Purpose**: Scene graph node.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:323) - `std::string` - Node name
- [`transform`](Source/Falcor/Scene/SceneBuilder.h:324) - `float4x4` - Transform matrix
- [`meshBind`](Source/Falcor/Scene/SceneBuilder.h:325) - `float4x4` - For skinned meshes. World transform at bind time
- [`localToBindPose`](Source/Falcor/Scene/SceneBuilder.h:326) - `float4x4` - For bones. Inverse bind transform
- [`parent`](Source/Falcor/Scene/SceneBuilder.h:327) - `NodeID` - Parent node ID

### SceneBuilder Class

**Purpose**: Scene builder for constructing scenes from imported data.

**Constructors**:
- [`SceneBuilder(ref<Device> pDevice, const Settings& settings, Flags flags = Flags::Default)`](Source/Falcor/Scene/SceneBuilder.h:332) - Constructor with device, settings, and flags
- [`SceneBuilder(ref<Device> pDevice, const std::filesystem::path& path, const Settings& settings, Flags flags = Flags::Default)`](Source/Falcor/Scene/SceneBuilder.h:337) - Constructor with device, path, settings, and flags. Creates a new builder and imports a scene/model file. Throws an ImporterError if importing went wrong
- [`SceneBuilder(ref<Device> pDevice, const void* buffer, size_t byteSize, std::string_view extension, const Settings& settings, Flags flags = Flags::Default)`](Source/Falcor/Scene/SceneBuilder.h:342) - Constructor with device, buffer, extension, settings, and flags. Creates a new builder and imports a scene/model from memory. Throws an ImporterError if importing went wrong
- [`~SceneBuilder()`](Source/Falcor/Scene/SceneBuilder.h:344) - Destructor

**Import Methods**:
- [`import(const std::filesystem::path& path, const pybind11::dict& dict = pybind11::dict())`](Source/Falcor/Scene/SceneBuilder.h:350) - Import a scene/model file. Throws an ImporterError if something went wrong
- [`importFromMemory(const void* buffer, size_t byteSize, std::string_view extension, const pybind11::dict& dict = pybind11::dict())`](Source/Falcor/Scene/SceneBuilder.h:359) - Import a scene/model file from memory. Throws an ImporterError if something went wrong

**Asset Resolver Methods**:
- [`getAssetResolver()`](Source/Falcor/Scene/SceneBuilder.h:362) - Access current asset resolver (on top of stack)
- [`getAssetResolver() const`](Source/Falcor/Scene/SceneBuilder.h:363) - Access current asset resolver (on top of stack)
- [`pushAssetResolver()`](Source/Falcor/Scene/SceneBuilder.h:366) - Push state of asset resolver to stack
- [`popAssetResolver()`](Source/Falcor/Scene/SceneBuilder.h:369) - Pop state of asset resolver from stack

**Scene Creation**:
- [`getScene()`](Source/Falcor/Scene/SceneBuilder.h:374) - Get the scene. Make sure to add all objects before calling this function. Returns nullptr if something went wrong, otherwise a new Scene object

**Device and Settings**:
- [`getDevice() const`](Source/Falcor/Scene/SceneBuilder.h:376) - Get the device
- [`getSettings() const`](Source/Falcor/Scene/SceneBuilder.h:378) - Get the settings
- [`getSettings()`](Source/Falcor/Scene/SceneBuilder.h:379) - Get the settings
- [`getFlags() const`](Source/Falcor/Scene/SceneBuilder.h:383) - Get build flags

**Render Settings**:
- [`setRenderSettings(const Scene::RenderSettings& renderSettings)`](Source/Falcor/Scene/SceneBuilder.h:387) - Set render settings
- [`getRenderSettings()`](Source/Falcor/Scene/SceneBuilder.h:391) - Get render settings
- [`getRenderSettings() const`](Source/Falcor/Scene/SceneBuilder.h:395) - Get render settings

**Metadata**:
- [`setMetadata(const Scene::Metadata& metadata)`](Source/Falcor/Scene/SceneBuilder.h:399) - Set metadata
- [`getMetadata()`](Source/Falcor/Scene/SceneBuilder.h:403) - Get metadata
- [`getMetadata() const`](Source/Falcor/Scene/SceneBuilder.h:407) - Get metadata

**Mesh Methods**:
- [`addMesh(const Mesh& mesh)`](Source/Falcor/Scene/SceneBuilder.h:416) - Add a mesh. Throws an exception if something went wrong. Returns the ID of mesh in scene. Note that all instances share the same mesh ID
- [`addTriangleMesh(const ref<TriangleMesh>& pTriangleMesh, const ref<Material>& pMaterial, bool isAnimated = false)`](Source/Falcor/Scene/SceneBuilder.h:424) - Add a triangle mesh. Returns the ID of mesh in scene
- [`processMesh(const Mesh& mesh, MeshAttributeIndices* pAttributeIndices = nullptr, std::vector<float4>* pTangents = nullptr) const`](Source/Falcor/Scene/SceneBuilder.h:433) - Pre-process a mesh into data format that is used in global scene buffers. Throws an exception if something went wrong. Returns the pre-processed mesh
- [`generateTangents(Mesh& mesh, std::vector<float4>& tangents)`](Source/Falcor/Scene/SceneBuilder.h:439) - Generate tangents for a mesh. If successful, tangent attribute on mesh will be set to output vector
- [`addProcessedMesh(const ProcessedMesh& mesh)`](Source/Falcor/Scene/SceneBuilder.h:445) - Add a pre-processed mesh. Returns the ID of mesh in scene. Note that all instances share the same mesh ID
- [`addCachedMeshes(std::vector<CachedMesh>&& cachedMeshes)`](Source/Falcor/Scene/SceneBuilder.h:450) - Add mesh vertex cache for animation
- [`addCachedMesh(CachedMesh&& cachedMesh)`](Source/Falcor/Scene/SceneBuilder.h:451) - Add mesh vertex cache for animation

**Custom Primitive Methods**:
- [`addCustomPrimitive(uint32_t userID, const AABB& aabb)`](Source/Falcor/Scene/SceneBuilder.h:459) - Add an AABB defining a custom primitive. User-defined ID can be used to identify different sub-types of custom primitives

**Curve Methods**:
- [`addCurve(const Curve& curve)`](Source/Falcor/Scene/SceneBuilder.h:468) - Add a curve. Throws an exception if something went wrong. Returns the ID of curve in scene. Note that all instances share the same curve ID
- [`processCurve(const Curve& curve) const`](Source/Falcor/Scene/SceneBuilder.h:475) - Pre-process a curve into data format that is used in global scene buffers. Throws an exception if something went wrong. Returns the pre-processed curve
- [`addProcessedCurve(const ProcessedCurve& curve)`](Source/Falcor/Scene/SceneBuilder.h:481) - Add a pre-processed curve. Returns the ID of curve in scene. Note that all instances share the same curve ID
- [`addCachedCurves(std::vector<CachedCurve>&& cachedCurves)`](Source/Falcor/Scene/SceneBuilder.h:486) - Set curve vertex cache for animation
- [`addCachedCurve(CachedCurve&& cachedCurves)`](Source/Falcor/Scene/SceneBuilder.h:487) - Set curve vertex cache for animation

**SDF Grid Methods**:
- [`addSDFGrid(const ref<SDFGrid>& pSDFGrid, const ref<Material>& pMaterial)`](Source/Falcor/Scene/SceneBuilder.h:496) - Add an SDF grid. Returns the ID of SDF grid desc in scene

**Material Methods**:
- [`getMaterials() const`](Source/Falcor/Scene/SceneBuilder.h:502) - Get list of materials
- [`getMaterial(const std::string& name) const`](Source/Falcor/Scene/SceneBuilder.h:509) - Get a material by name. Note: This returns first material found with a matching name. Returns first material with a matching name or nullptr if none was found
- [`addMaterial(const ref<Material>& pMaterial)`](Source/Falcor/Scene/SceneBuilder.h:515) - Add a material. Returns the ID of material in scene
- [`replaceMaterial(const ref<Material>& pMaterial, const ref<Material>& pReplacement)`](Source/Falcor/Scene/SceneBuilder.h:521) - Replace a material
- [`loadMaterialTexture(const ref<Material>& pMaterial, Material::TextureSlot slot, const std::filesystem::path& path)`](Source/Falcor/Scene/SceneBuilder.h:528) - Request loading a material texture
- [`waitForMaterialTextureLoading()`](Source/Falcor/Scene/SceneBuilder.h:532) - Wait until all material textures are loaded

**Volume Methods**:
- [`getGridVolumes() const`](Source/Falcor/Scene/SceneBuilder.h:538) - Get list of grid volumes
- [`getGridVolume(const std::string& name) const`](Source/Falcor/Scene/SceneBuilder.h:545) - Get a grid volume by name. Note: This returns first volume found with a matching name. Returns first volume with a matching name or nullptr if none was found
- [`addGridVolume(const ref<GridVolume>& pGridVolume, NodeID nodeID = NodeID{ NodeID::Invalid() })`](Source/Falcor/Scene/SceneBuilder.h:552) - Add a grid volume. Returns the ID of volume in scene

**Light Methods**:
- [`getLights() const`](Source/Falcor/Scene/SceneBuilder.h:558) - Get list of lights
- [`getLight(const std::string& name) const`](Source/Falcor/Scene/SceneBuilder.h:565) - Get a light by name. Note: This returns first light found with a matching name. Returns first light with a matching name or nullptr if none was found
- [`addLight(const ref<Light>& pLight)`](Source/Falcor/Scene/SceneBuilder.h:571) - Add a light source. Returns the light ID
- [`loadLightProfile(const std::string& filename, bool normalize = true)`](Source/Falcor/Scene/SceneBuilder.h:575) - DEMO21: Load global light profile

**Environment Map Methods**:
- [`getEnvMap() const`](Source/Falcor/Scene/SceneBuilder.h:581) - Get environment map
- [`setEnvMap(ref<EnvMap> pEnvMap)`](Source/Falcor/Scene/SceneBuilder.h:586) - Set environment map. Can be nullptr

**Camera Methods**:
- [`getCameras() const`](Source/Falcor/Scene/SceneBuilder.h:592) - Get list of cameras
- [`addCamera(const ref<Camera>& pCamera)`](Source/Falcor/Scene/SceneBuilder.h:598) - Add a camera. Returns the camera ID
- [`getSelectedCamera() const`](Source/Falcor/Scene/SceneBuilder.h:602) - Get selected camera
- [`setSelectedCamera(const ref<Camera>& pCamera)`](Source/Falcor/Scene/SceneBuilder.h:607) - Set selected camera. Camera needs to be added first
- [`getCameraSpeed() const`](Source/Falcor/Scene/SceneBuilder.h:611) - Get camera speed
- [`setCameraSpeed(float speed)`](Source/Falcor/Scene/SceneBuilder.h:615) - Set camera speed

**Animation Methods**:
- [`getAnimations() const`](Source/Falcor/Scene/SceneBuilder.h:621) - Get list of animations
- [`addAnimation(const ref<Animation>& pAnimation)`](Source/Falcor/Scene/SceneBuilder.h:626) - Add an animation
- [`createAnimation(ref<Animatable> pAnimatable, const std::string& name, double duration)`](Source/Falcor/Scene/SceneBuilder.h:634) - Create an animation for an animatable object. Returns a new animation or nullptr if an animation already exists

**Scene Graph Methods**:
- [`addNode(const Node& node)`](Source/Falcor/Scene/SceneBuilder.h:641) - Adds a node to graph. Returns the node ID
- [`getNodeCount() const`](Source/Falcor/Scene/SceneBuilder.h:646) - Get how many nodes have been added to scene graph. Returns the node count
- [`getNode(NodeID nodeID)`](Source/Falcor/Scene/SceneBuilder.h:648) - Get node by ID
- [`addMeshInstance(NodeID nodeID, MeshID meshID)`](Source/Falcor/Scene/SceneBuilder.h:652) - Add a mesh instance to a node
- [`addCurveInstance(NodeID nodeID, CurveID curveID)`](Source/Falcor/Scene/SceneBuilder.h:656) - Add a curve instance to a node
- [`addSDFGridInstance(NodeID nodeID, SdfDescID sdfGridID)`](Source/Falcor/Scene/SceneBuilder.h:660) - Add an SDF grid instance to a node
- [`isNodeAnimated(NodeID nodeID) const`](Source/Falcor/Scene/SceneBuilder.h:665) - Check if a scene node is animated. This check is done recursively through parent nodes. Returns true if node is animated
- [`setNodeInterpolationMode(NodeID nodeID, Animation::InterpolationMode interpolationMode, bool enableWarping)`](Source/Falcor/Scene/SceneBuilder.h:669) - Set animation interpolation mode for a given scene node. This sets the mode recursively for all parent nodes

**Texture Loader**:
- [`getMaterialTextureLoader()`](Source/Falcor/Scene/SceneBuilder.h:673) - Returns texture manager used by material system

### InternalNode Structure

**Purpose**: Internal node structure extending Node with additional fields for scene graph management.

**Fields**:
- [`children`](Source/Falcor/Scene/SceneBuilder.h:679) - `std::vector<NodeID>` - Node IDs of all child nodes
- [`meshes`](Source/Falcor/Scene/SceneBuilder.h:680) - `std::vector<MeshID>` - Mesh IDs of all meshes this node transforms
- [`curves`](Source/Falcor/Scene/SceneBuilder.h:681) - `std::vector<CurveID>` - Curve IDs of all curves this node transforms
- [`sdfGrids`](Source/Falcor/Scene/SceneBuilder.h:682) - `std::vector<SdfGridID>` - SDF grid IDs of all SDF grids this node transforms
- [`animatable`](Source/Falcor/Scene/SceneBuilder.h:683) - `std::vector<Animatable*>` - Pointers to all animatable objects attached to this node
- [`dontOptimize`](Source/Falcor/Scene/SceneBuilder.h:684) - `bool` - Whether node should be ignored in optimization passes

**Methods**:
- [`hasObjects() const`](Source/Falcor/Scene/SceneBuilder.h:688) - Returns true if node has any attached scene objects

### MeshSpec Structure

**Purpose**: Mesh specification for internal mesh management.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:692) - `std::string` - Mesh name
- [`topology`](Source/Falcor/Scene/SceneBuilder.h:693) - `Vao::Topology` - Primitive topology
- [`materialId`](Source/Falcor/Scene/SceneBuilder.h:694) - `MaterialID` - Global material ID
- [`staticVertexOffset`](Source/Falcor/Scene/SceneBuilder.h:695) - `uint32_t` - Offset into shared 'staticData' array. This is calculated in createGlobalBuffers()
- [`staticVertexCount`](Source/Falcor/Scene/SceneBuilder.h:696) - `uint32_t` - Number of static vertices
- [`skinningVertexOffset`](Source/Falcor/Scene/SceneBuilder.h:697) - `uint32_t` - Offset into shared 'skinningData' array. This is calculated in createGlobalBuffers()
- [`skinningVertexCount`](Source/Falcor/Scene/SceneBuilder.h:698) - `uint32_t` - Number of skinned vertices
- [`prevVertexOffset`](Source/Falcor/Scene/SceneBuilder.h:699) - `uint32_t` - Offset into shared `prevVertices` array. This is calculated in createGlobalBuffers()
- [`prevVertexCount`](Source/Falcor/Scene/SceneBuilder.h:700) - `uint32_t` - Number of previous vertices stored. This can be static or skinned vertex count depending on animation type
- [`indexOffset`](Source/Falcor/Scene/SceneBuilder.h:701) - `uint32_t` - Offset into shared 'indexData' array. This is calculated in createGlobalBuffers()
- [`indexCount`](Source/Falcor/Scene/SceneBuilder.h:702) - `uint32_t` - Number of indices, or zero if non-indexed
- [`vertexCount`](Source/Falcor/Scene/SceneBuilder.h:703) - `uint32_t` - Number of vertices
- [`skeletonNodeID`](Source/Falcor/Scene/SceneBuilder.h:704) - `NodeID` - Node ID of skeleton world transform. Forwarded from Mesh struct
- [`use16BitIndices`](Source/Falcor/Scene/SceneBuilder.h:705) - `bool` - True if indices are in 16-bit format
- [`hasSkinningData`](Source/Falcor/Scene/SceneBuilder.h:706) - `bool` - True if mesh has skinned vertices
- [`isStatic`](Source/Falcor/Scene/SceneBuilder.h:707) - `bool` - True if mesh is non-instanced and static (not dynamic or animated)
- [`isFrontFaceCW`](Source/Falcor/Scene/SceneBuilder.h:708) - `bool` - Indicate whether front-facing side has clockwise winding in object space
- [`isDisplaced`](Source/Falcor/Scene/SceneBuilder.h:709) - `bool` - True if mesh has displacement map
- [`isAnimated`](Source/Falcor/Scene/SceneBuilder.h:710) - `bool` - True if mesh vertices can be modified during rendering (e.g., skinning or inverse rendering)
- [`boundingBox`](Source/Falcor/Scene/SceneBuilder.h:711) - `AABB` - Mesh bounding-box in object space
- [`instances`](Source/Falcor/Scene/SceneBuilder.h:712) - `std::set<NodeID>` - IDs of all nodes that instantiate this mesh
- [`indexData`](Source/Falcor/Scene/SceneBuilder.h:715) - `std::vector<uint32_t>` - Vertex indices in either 32-bit or 16-bit format packed tightly, or empty if non-indexed
- [`staticData`](Source/Falcor/Scene/SceneBuilder.h:716) - `std::vector<StaticVertexData>` - Static vertex data
- [`skinningData`](Source/Falcor/Scene/SceneBuilder.h:717) - `std::vector<SkinningVertexData>` - Skinning vertex data

**Methods**:
- [`getTriangleCount() const`](Source/Falcor/Scene/SceneBuilder.h:719) - Get triangle count
- [`getIndex(const size_t i) const`](Source/Falcor/Scene/SceneBuilder.h:725) - Get index
- [`isSkinned() const`](Source/Falcor/Scene/SceneBuilder.h:731) - Check if mesh is skinned
- [`isDynamic() const`](Source/Falcor/Scene/SceneBuilder.h:736) - Check if mesh is dynamic

### CurveSpec Structure

**Purpose**: Curve specification for internal curve management.

**Fields**:
- [`name`](Source/Falcor/Scene/SceneBuilder.h:745) - `std::string` - Curve name
- [`topology`](Source/Falcor/Scene/SceneBuilder.h:746) - `Vao::Topology` - Primitive topology
- [`materialId`](Source/Falcor/Scene/SceneBuilder.h:747) - `MaterialID` - Global material ID
- [`staticVertexOffset`](Source/Falcor/Scene/SceneBuilder.h:748) - `uint32_t` - Offset into shared 'staticData' array. This is calculated in createCurveGlobalBuffers()
- [`staticVertexCount`](Source/Falcor/Scene/SceneBuilder.h:749) - `uint32_t` - Number of static curve vertices
- [`indexOffset`](Source/Falcor/Scene/SceneBuilder.h:750) - `uint32_t` - Offset into shared 'indexData' array. This is calculated in createCurveGlobalBuffers()
- [`indexCount`](Source/Falcor/Scene/SceneBuilder.h:751) - `uint32_t` - Number of indices
- [`vertexCount`](Source/Falcor/Scene/SceneBuilder.h:752) - `uint32_t` - Number of vertices
- [`degree`](Source/Falcor/Scene/SceneBuilder.h:753) - `uint32_t` - Polynomial degree of curve; linear (1) by default
- [`instances`](Source/Falcor/Scene/SceneBuilder.h:754) - `std::set<NodeID>` - IDs of all nodes that instantiate this curve
- [`indexData`](Source/Falcor/Scene/SceneBuilder.h:757) - `std::vector<uint32_t>` - Vertex indices in 32-bit
- [`staticData`](Source/Falcor/Scene/SceneBuilder.h:758) - `std::vector<StaticCurveVertexData>` - Static curve vertex data

## Technical Details

### Build Flags

**Material Merging**:
- [`DontMergeMaterials`](Source/Falcor/Scene/SceneBuilder.h:63) - Preserves original material names by not merging materials with same properties
- [`DontOptimizeMaterials`](Source/Falcor/Scene/SceneBuilder.h:76) - Disables material optimization by removing constant textures

**Tangent Space**:
- [`UseOriginalTangentSpace`](Source/Falcor/Scene/SceneBuilder.h:64) - Uses original tangent space loaded with mesh
- Default: Uses MikkTSpace to generate tangent space
- Always generates tangent space if missing

**Texture Color Space**:
- [`AssumeLinearSpaceTextures`](Source/Falcor/Scene/SceneBuilder.h:65) - Forces linear space for color textures
- Default: Interprets color textures (diffuse/specular) as sRGB data

**Mesh Merging**:
- [`DontMergeMeshes`](Source/Falcor/Scene/SceneBuilder.h:66) - Preserves original list of meshes
- Default: Merges meshes with same material
- Only applies to AssimpImporter

**Material Shading Models**:
- [`UseSpecGlossMaterials`](Source/Falcor/Scene/SceneBuilder.h:67) - Sets materials to Spec-Gloss shading model
- [`UseMetalRoughMaterials`](Source/Falcor/Scene/SceneBuilder.h:68) - Sets materials to Metal-Rough shading model
- Default: Spec-Gloss for OBJ, Metal-Rough for everything else

**Vertex Indexing**:
- [`NonIndexedVertices`](Source/Falcor/Scene/SceneBuilder.h:69) - Converts meshes to use non-indexed vertices
- [`Force32BitIndices`](Source/Falcor/Scene/SceneBuilder.h:70) - Forces 32-bit indices for all meshes
- Default: 16-bit indices for small meshes

**Ray Tracing Optimization**:
- [`RTDontMergeStatic`](Source/Falcor/Scene/SceneBuilder.h:71) - Don't merge static non-instanced meshes into single BLAS
- [`RTDontMergeDynamic`](Source/Falcor/Scene/SceneBuilder.h:72) - Don't merge dynamic non-instanced meshes with identical transforms into single BLAS
- [`RTDontMergeInstanced`](Source/Falcor/Scene/SceneBuilder.h:73) - Don't merge instanced meshes with identical instances into single BLAS

**Mesh Instance Flattening**:
- [`FlattenStaticMeshInstances`](Source/Falcor/Scene/SceneBuilder.h:74) - Flattens static mesh instances by duplicating mesh data
- Animated instances are not affected
- Can lead to large increase in memory use

**Scene Graph Optimization**:
- [`DontOptimizeGraph`](Source/Falcor/Scene/SceneBuilder.h:75) - Don't optimize scene graph to remove unnecessary nodes

**Displacement Mapping**:
- [`DontUseDisplacement`](Source/Falcor/Scene/SceneBuilder.h:77) - Don't use displacement mapping

**Hit Info Compression**:
- [`UseCompressedHitInfo`](Source/Falcor/Scene/SceneBuilder.h:78) - Uses compressed hit info (on scenes with triangle meshes only)

**Curve Tessellation**:
- [`TessellateCurvesIntoPolyTubes`](Source/Falcor/Scene/SceneBuilder.h:79) - Tessellates curves into poly-tubes
- Default: Linear swept spheres

**Scene Caching**:
- [`UseCache`](Source/Falcor/Scene/SceneBuilder.h:81) - Enables scene caching
- Caches runtime scene representation on disk to reduce load time
- [`RebuildCache`](Source/Falcor/Scene/SceneBuilder.h:82) - Rebuilds scene cache

### Mesh Attribute Frequencies

**Constant**:
- Single value for entire mesh
- Element count must be 1

**Uniform**:
- One value per face
- Element count must match faceCount

**Vertex**:
- One value per vertex
- Element count must match vertexCount

**FaceVarying**:
- One value per vertex per face
- Element count must match indexCount

**None**:
- No attribute data
- Returns Scene::kInvalidAttributeIndex

### Mesh Processing

**Process Mesh**:
- Converts Mesh to ProcessedMesh format
- Handles attribute frequency conversion
- Generates tangents if needed
- Merges duplicate vertices if requested
- Supports 16-bit and 32-bit indices

**Generate Tangents**:
- Uses MikkTSpace algorithm
- Generates tangent space for mesh
- Supports original tangent space preservation
- Always generates tangent space if missing

**Add Mesh**:
- Adds mesh to scene
- Returns MeshID
- All instances share same MeshID
- Throws exception on error

### Curve Processing

**Process Curve**:
- Converts Curve to ProcessedCurve format
- Handles attribute conversion
- Supports linear curves by default
- Supports higher-degree curves

**Add Curve**:
- Adds curve to scene
- Returns CurveID
- All instances share same CurveID
- Throws exception on error

### Scene Graph Management

**Node Structure**:
- Hierarchical scene graph
- Parent-child relationships
- Transform matrices
- Mesh/curve/SDF grid attachments
- Animation support

**Add Node**:
- Adds node to graph
- Returns NodeID
- Supports transform matrices
- Supports parent-child relationships

**Add Instance**:
- [`addMeshInstance`](Source/Falcor/Scene/SceneBuilder.h:652) - Adds mesh instance to node
- [`addCurveInstance`](Source/Falcor/Scene/SceneBuilder.h:656) - Adds curve instance to node
- [`addSDFGridInstance`](Source/Falcor/Scene/SceneBuilder.h:660) - Adds SDF grid instance to node

**Node Animation**:
- [`isNodeAnimated`](Source/Falcor/Scene/SceneBuilder.h:665) - Checks if node is animated recursively
- [`setNodeInterpolationMode`](Source/Falcor/Scene/SceneBuilder.h:669) - Sets animation interpolation mode recursively

### Material Management

**Add Material**:
- Adds material to scene
- Returns MaterialID
- Supports material merging
- Supports material optimization

**Replace Material**:
- Replaces existing material
- Updates all instances
- Preserves material ID

**Load Material Texture**:
- Loads texture asynchronously
- Supports texture slots
- Supports asset resolver

**Wait for Material Texture Loading**:
- Blocks until all textures loaded
- Ensures textures ready before scene creation

### Scene Optimization

**Mesh Grouping**:
- Groups meshes for BLAS creation
- Groups by material
- Groups by static/dynamic
- Groups by instancing

**Scene Graph Optimization**:
- Removes unnecessary nodes
- Collapses nodes with single child
- Merges nodes with same transform

**Material Optimization**:
- Removes constant textures
- Lossless optimization
- Reduces memory usage

**Mesh Optimization**:
- Removes unused meshes
- Flattens static instances
- Pre-transforms static meshes
- Unifies triangle winding

**Geometry Optimization**:
- Sorts meshes for cache locality
- Optimizes vertex layout
- Reduces memory usage

## Integration Points

### Scene Integration

**Scene Creation**:
- [`getScene()`](Source/Falcor/Scene/SceneBuilder.h:374) - Creates Scene object from built data
- Transfers all data to Scene
- Finalizes scene structure
- Returns nullptr on error

**Scene Data**:
- [`Scene::SceneData`](Source/Falcor/Scene/SceneBuilder.h:776) - Local copy of scene data
- Used for scene construction
- Transferred to Scene on getScene()

### Material System Integration

**Material Texture Loader**:
- [`MaterialTextureLoader`](Source/Falcor/Scene/SceneBuilder.h:788) - Texture manager
- Handles texture loading
- Supports async loading
- Integrates with asset resolver

**Material Management**:
- [`addMaterial()`](Source/Falcor/Scene/SceneBuilder.h:515) - Adds material to scene
- [`replaceMaterial()`](Source/Falcor/Scene/SceneBuilder.h:521) - Replaces material
- [`loadMaterialTexture()`](Source/Falcor/Scene/SceneBuilder.h:528) - Loads material texture

### Scene Cache Integration

**Scene Cache Key**:
- [`SceneCache::Key`](Source/Falcor/Scene/SceneBuilder.h:778) - Scene cache key
- Used for scene caching
- Based on build flags and settings

**Write Scene Cache**:
- [`mWriteSceneCache`](Source/Falcor/Scene/SceneBuilder.h:779) - Flag to write scene cache
- Set when UseCache flag is set
- Written after scene import

### Asset Resolver Integration

**Asset Resolver Stack**:
- [`mAssetResolver`](Source/Falcor/Scene/SceneBuilder.h:773) - Current asset resolver
- [`mAssetResolverStack`](Source/Falcor/Scene/SceneBuilder.h:774) - Stack of asset resolvers
- [`pushAssetResolver()`](Source/Falcor/Scene/SceneBuilder.h:366) - Pushes asset resolver
- [`popAssetResolver()`](Source/Falcor/Scene/SceneBuilder.h:369) - Pops asset resolver
- Supports nested asset resolution

## Architecture Patterns

### Builder Pattern

- Incremental scene construction
- Add objects one at a time
- Finalize with getScene()
- Supports multiple import sources

### Optimization Pattern

- Multi-stage optimization
- Mesh optimization
- Material optimization
- Scene graph optimization
- Geometry optimization

### Caching Pattern

- Scene caching support
- Disk-based cache
- Cache key based on build flags
- Reduces load time

### Processing Pattern

- Pre-processing of imported data
- Conversion to runtime format
- Attribute frequency handling
- Tangent space generation

## Code Patterns

### Mesh Attribute Access Pattern

```cpp
template<typename T>
T get(const Attribute<T>& attribute, uint32_t face, uint32_t vert) const
{
    if (attribute.pData)
    {
        return get(attribute, getAttributeIndex(attribute, face, vert));
    }
    return T{};
}
```

### Mesh Processing Pattern

```cpp
ProcessedMesh processMesh(const Mesh& mesh, MeshAttributeIndices* pAttributeIndices, std::vector<float4>* pTangents) const
{
    // Convert mesh to ProcessedMesh format
    // Handle attribute frequencies
    // Generate tangents if needed
    // Merge duplicate vertices if requested
}
```

### Scene Graph Construction Pattern

```cpp
NodeID addNode(const Node& node)
{
    // Add node to scene graph
    // Set parent-child relationships
    // Return node ID
}
```

### Material Management Pattern

```cpp
MaterialID addMaterial(const ref<Material>& pMaterial)
{
    // Add material to scene
    // Check for duplicates
    // Merge if needed
    // Return material ID
}
```

## Use Cases

### Scene Importing

- **File Import**:
  - Import scene from file
  - Support for multiple file formats
  - Error handling with ImporterError

- **Memory Import**:
  - Import scene from memory
  - Support for in-memory data
  - Error handling with ImporterError

### Scene Construction

- **Mesh Addition**:
  - Add meshes to scene
  - Support for triangle meshes
  - Support for processed meshes
  - Support for mesh vertex cache

- **Curve Addition**:
  - Add curves to scene
  - Support for processed curves
  - Support for curve vertex cache

- **SDF Grid Addition**:
  - Add SDF grids to scene
  - Link to materials
  - Support for SDF grid instances

- **Custom Primitive Addition**:
  - Add custom primitives to scene
  - Define by AABB
  - User-defined IDs

- **Scene Graph Construction**:
  - Add nodes to graph
  - Add instances to nodes
  - Support for hierarchical transforms
  - Support for animation

### Material Management

- **Material Addition**:
  - Add materials to scene
  - Support for material merging
  - Support for material optimization

- **Material Replacement**:
  - Replace existing materials
  - Update all instances
  - Preserve material IDs

- **Texture Loading**:
  - Load material textures
  - Support for async loading
  - Wait for completion

### Scene Optimization

- **Mesh Optimization**:
  - Remove unused meshes
  - Flatten static instances
  - Pre-transform static meshes
  - Unify triangle winding

- **Material Optimization**:
  - Remove constant textures
  - Lossless optimization
  - Reduce memory usage

- **Scene Graph Optimization**:
  - Remove unnecessary nodes
  - Collapse nodes with single child
  - Merge nodes with same transform

### Scene Caching

- **Cache Loading**:
  - Load scene from cache
  - Check cache validity
  - Rebuild if needed

- **Cache Writing**:
  - Write scene to cache
  - Based on build flags
  - Reduce future load time

## Performance Considerations

### Memory Usage

**Mesh Data**:
- Vertex data storage
- Index data storage
- Attribute data storage
- Skinning data storage
- Previous frame vertex storage

**Curve Data**:
- Vertex data storage
- Index data storage
- Attribute data storage

**Scene Graph**:
- Node storage
- Transform matrix storage
- Instance tracking

**Material Data**:
- Material storage
- Texture storage
- Material optimization reduces memory

### Processing Performance

**Mesh Processing**:
- O(n) per mesh for processing
- O(n) per mesh for tangents
- O(n) per mesh for vertex merging

**Scene Graph Optimization**:
- O(n) for node traversal
- O(n) for node collapsing
- O(n) for node merging

**Material Optimization**:
- O(n) for material comparison
- O(n) for texture optimization
- O(n) for material merging

### Caching Performance

**Cache Loading**:
- O(1) for cache lookup
- O(n) for cache validation
- Reduces load time significantly

**Cache Writing**:
- O(n) for cache serialization
- O(n) for disk write
- Increases next load time

## Limitations

### Feature Limitations

- **Mesh Merging**:
- Limited to AssimpImporter
- Not all meshes can be merged
- Material-based merging only

- **Tangent Space**:
- Limited to MikkTSpace algorithm
- Original tangent space may not be compatible
- Always generates tangent space if missing

- **Scene Graph**:
- Limited to hierarchical structure
- No support for non-hierarchical structures
- Limited to single parent per node

- **Material Optimization**:
- Lossless only
- Cannot optimize based on usage
- Limited to constant texture removal

### Performance Limitations

- **Mesh Processing**:
- Single-threaded processing
- No GPU acceleration
- May be slow for large meshes

- **Scene Graph Optimization**:
- Single-pass optimization
- May not find all optimizations
- Limited to simple patterns

- **Caching**:
- Disk-based only
- No in-memory caching
- Limited to single cache per scene

### Integration Limitations

- **Scene Coupling**:
- Tightly coupled to Scene class
- Requires specific data structures
- Not suitable for standalone use

- **Material System Coupling**:
- Tightly coupled to MaterialTextureLoader
- Requires specific material system
- Not suitable for other material systems

## Best Practices

### Scene Construction

- **Build Flags**:
  - Use appropriate build flags for use case
  - Enable optimizations when possible
  - Disable features not needed
  - Use caching for repeated loads

- **Mesh Addition**:
  - Add all meshes before calling getScene()
  - Use processed meshes when possible
  - Set appropriate flags for mesh properties
  - Use correct attribute frequencies

- **Scene Graph Construction**:
  - Build hierarchical scene graph
  - Use appropriate transforms
  - Set parent-child relationships correctly
  - Add instances after nodes

### Material Management

- **Material Addition**:
  - Add materials before meshes
  - Use appropriate shading models
  - Load textures asynchronously
  - Wait for texture loading

- **Material Optimization**:
  - Enable material optimization
  - Remove constant textures
  - Merge similar materials
  - Use appropriate shading models

### Scene Optimization

- **Enable Optimizations**:
  - Enable scene graph optimization
  - Enable material optimization
  - Enable mesh optimization
  - Use appropriate ray tracing flags

- **Disable Optimizations**:
  - Disable when debugging needed
  - Disable when original structure required
  - Disable when optimization causes issues
  - Use specific flags for specific cases

### Scene Caching

- **Cache Usage**:
  - Use caching for repeated loads
  - Rebuild cache when needed
  - Check cache validity
  - Use appropriate cache keys

### Performance Profiling

- **Memory Usage**:
  - Monitor memory usage
  - Use appropriate build flags
  - Enable compression when possible
  - Remove unused data

- **Processing Time**:
  - Profile mesh processing time
  - Profile optimization time
  - Profile cache load/save time
  - Identify bottlenecks

## Progress Log

- **2026-01-08T00:32:00Z**: SceneBuilder analysis completed. Analyzed SceneBuilder.h (849 lines) containing comprehensive scene construction system. Documented Flags enumeration with 20+ build flags for controlling scene construction behavior (DontMergeMaterials, UseOriginalTangentSpace, AssumeLinearSpaceTextures, DontMergeMeshes, UseSpecGlossMaterials, UseMetalRoughMaterials, NonIndexedVertices, Force32BitIndices, RTDontMergeStatic, RTDontMergeDynamic, RTDontMergeInstanced, FlattenStaticMeshInstances, DontOptimizeGraph, DontOptimizeMaterials, DontUseDisplacement, UseCompressedHitInfo, TessellateCurvesIntoPolyTubes, UseCache, RebuildCache), Mesh structure with AttributeFrequency enumeration (None, Constant, Uniform, Vertex, FaceVarying), Attribute<T> template structure, Vertex structure, VertexAttributeIndices structure, and methods for attribute access (getAttributeIndex, get, getAttributeCount, getPosition, getNormal, getTangent, getTexCrd, getCurveRadii, getVertex, getVertex, getAttributeIndices, hasBones), ProcessedMesh structure with pre-processed mesh data, Curve structure with Attribute<T> template structure, ProcessedCurve structure with pre-processed curve data, Node structure with transform matrices and parent-child relationships, SceneBuilder class with constructors (device/settings/flags, device/path/settings/flags, device/buffer/extension/settings/flags), import methods (import, importFromMemory), asset resolver methods (getAssetResolver, pushAssetResolver, popAssetResolver), scene creation (getScene), device and settings access (getDevice, getSettings, getFlags), render settings (setRenderSettings, getRenderSettings), metadata (setMetadata, getMetadata), mesh methods (addMesh, addTriangleMesh, processMesh, generateTangents, addProcessedMesh, addCachedMeshes, addCachedMesh), custom primitive methods (addCustomPrimitive), curve methods (addCurve, processCurve, addProcessedCurve, addCachedCurves, addCachedCurve), SDF grid methods (addSDFGrid), material methods (getMaterials, getMaterial, addMaterial, replaceMaterial, loadMaterialTexture, waitForMaterialTextureLoading), volume methods (getGridVolumes, getGridVolume, addGridVolume), light methods (getLights, getLight, addLight, loadLightProfile), environment map methods (getEnvMap, setEnvMap), camera methods (getCameras, addCamera, getSelectedCamera, setSelectedCamera, getCameraSpeed, setCameraSpeed), animation methods (getAnimations, addAnimation, createAnimation), scene graph methods (addNode, getNodeCount, getNode, addMeshInstance, addCurveInstance, addSDFGridInstance, isNodeAnimated, setNodeInterpolationMode), and texture loader (getMaterialTextureLoader). Documented InternalNode structure with children, meshes, curves, sdfGrids, animatable, dontOptimize fields and hasObjects method, MeshSpec structure with mesh specification fields and methods (getTriangleCount, getIndex, isSkinned, isDynamic), CurveSpec structure with curve specification fields, and private members including device, settings, flags, asset resolver stack, scene data, scene, scene cache key, write scene cache flag, scene graph, mesh list, mesh group list, curve list, material texture loader, and helper methods (doesNodeHaveAnimation, updateLinkedObjects, collapseNodes, mergeNodes, flipTriangleWinding, updateSDFGridID, splitMesh, splitIndexedMesh, splitNonIndexedMesh, countTriangles, calculateBoundingBox, needsSplit, splitMeshGroupSimple, splitMeshGroupMedian, splitMeshGroupMidpointMeshes, prepareDisplacementMaps, prepareSceneGraph, prepareMeshes, removeUnusedMeshes, flattenStaticMeshInstances, optimizeSceneGraph, pretransformStaticMeshes, unifyTriangleWinding, calculateMeshBoundingBoxes, createMeshGroups, optimizeGeometry, sortMeshes, createGlobalBuffers, createCurveGlobalBuffers, optimizeMaterials, removeDuplicateMaterials, collectVolumeGrids, quantizeTexCoords, removeDuplicateSDFGrids, createMeshData, createMeshInstanceData, createCurveData, createCurveInstanceData, createSceneGraph, createMeshBoundingBoxes, calculateCurveBoundingBoxes). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The SceneBuilder module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
