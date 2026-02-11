# IScene - Scene Interface

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **IScene.h** - Scene interface header
- [x] **IScene.cpp** - Scene interface implementation

### External Dependencies

- **Core/Object** - Base object class
- **Core/Program/Program** - Shader program
- **Core/API/Raytracing** - Ray tracing API
- **Core/API/RenderContext** - Render context
- **Core/Program/ShaderVar** - Shader variable
- **Core/Program/ProgramVars** - Program variables
- **Utils/Math/AABB** - Axis-aligned bounding box
- **Scene/EnvMap** - Environment map
- **Scene/Lights/ILightCollection** - Light collection interface
- **Scene/Camera** - Camera
- **Scene/Lights/Light** - Light
- **Scene/Materials/MaterialSystem** - Material system
- **Core/Sampler** - Texture sampler
- **sigs/sigs** - Signal library

## Module Overview

IScene is the abstract base class for scene implementations in Falcor. It defines the interface for scene operations including update flags, render settings, shader data binding, ray tracing support, light collection management, camera access, material system access, and environment map access. The interface is designed to be polymorphic, allowing different scene implementations to be used interchangeably.

## Component Specifications

### UpdateFlags Enumeration

**Purpose**: Flags indicating if and what was updated in the scene.

**Key Features**:
- 26 different update flags for tracking scene changes
- Flags for geometry, camera, lights, materials, environment map, grid volumes, curves, custom primitives, displacement, SDF grids, meshes, scene defines, type conformances, shader code, and emissive materials
- RecompileNeeded flag combination for tracking changes that require shader recompilation
- All flag for tracking any change

**Flags**:
- [`None`](Source/Falcor/Scene/IScene.h:56) - Nothing happened
- [`GeometryMoved`](Source/Falcor/Scene/IScene.h:57) - Geometry moved
- [`CameraMoved`](Source/Falcor/Scene/IScene.h:58) - The camera moved
- [`CameraPropertiesChanged`](Source/Falcor/Scene/IScene.h:59) - Some camera properties changed, excluding position
- [`CameraSwitched`](Source/Falcor/Scene/IScene.h:60) - Selected a different camera
- [`LightsMoved`](Source/Falcor/Scene/IScene.h:61) - Lights were moved
- [`LightIntensityChanged`](Source/Falcor/Scene/IScene.h:62) - Light intensity changed
- [`LightPropertiesChanged`](Source/Falcor/Scene/IScene.h:63) - Other light changes not included in LightIntensityChanged and LightsMoved
- [`SceneGraphChanged`](Source/Falcor/Scene/IScene.h:64) - Any transform in the scene graph changed
- [`LightCollectionChanged`](Source/Falcor/Scene/IScene.h:65) - Light collection changed (mesh lights)
- [`MaterialsChanged`](Source/Falcor/Scene/IScene.h:66) - Materials changed
- [`EnvMapChanged`](Source/Falcor/Scene/IScene.h:67) - Environment map changed
- [`EnvMapPropertiesChanged`](Source/Falcor/Scene/IScene.h:68) - Environment map properties changed (check EnvMap::getChanges() for more specific information)
- [`LightCountChanged`](Source/Falcor/Scene/IScene.h:69) - Number of active lights changed
- [`RenderSettingsChanged`](Source/Falcor/Scene/IScene.h:70) - Render settings changed
- [`GridVolumesMoved`](Source/Falcor/Scene/IScene.h:71) - Grid volumes were moved
- [`GridVolumePropertiesChanged`](Source/Falcor/Scene/IScene.h:72) - Grid volume properties changed
- [`GridVolumeGridsChanged`](Source/Falcor/Scene/IScene.h:73) - Grid volume grids changed
- [`GridVolumeBoundsChanged`](Source/Falcor/Scene/IScene.h:74) - Grid volume bounds changed
- [`CurvesMoved`](Source/Falcor/Scene/IScene.h:75) - Curves moved
- [`CustomPrimitivesMoved`](Source/Falcor/Scene/IScene.h:76) - Custom primitives moved
- [`GeometryChanged`](Source/Falcor/Scene/IScene.h:77) - Scene geometry changed (added/removed)
- [`DisplacementChanged`](Source/Falcor/Scene/IScene.h:78) - Displacement mapping parameters changed
- [`SDFGridConfigChanged`](Source/Falcor/Scene/IScene.h:79) - SDF grid config changed
- [`SDFGeometryChanged`](Source/Falcor/Scene/IScene.h:80) - SDF grid geometry changed
- [`MeshesChanged`](Source/Falcor/Scene/IScene.h:81) - Mesh data changed (skinning or vertex animations)
- [`SceneDefinesChanged`](Source/Falcor/Scene/IScene.h:82) - Scene defines changed. All programs that access the scene must be updated!
- [`TypeConformancesChanged`](Source/Falcor/Scene/IScene.h:83) - Type conformances changed. All programs that access the scene must be updated!
- [`ShaderCodeChanged`](Source/Falcor/Scene/IScene.h:84) - Shader code changed. All programs that access the scene must be updated!
- [`EmissiveMaterialsChanged`](Source/Falcor/Scene/IScene.h:85) - Emissive materials changed
- [`RecompileNeeded`](Source/Falcor/Scene/IScene.h:90) - Flags indicating that programs that access the scene need to be recompiled. This is needed if defines, type conformances, and/or the shader code has changed. The goal is to minimize changes that require recompilation, as it can be costly.
- [`All`](Source/Falcor/Scene/IScene.h:92) - All flags

**RecompileNeeded Flag**:
- Combination of: `SceneDefinesChanged | TypeConformancesChanged | ShaderCodeChanged`
- Indicates that programs accessing the scene need to be recompiled
- Used to minimize recompilation overhead

### TypeConformancesKind Enumeration

**Purpose**: Enumeration for type conformances kinds.

**Key Features**:
- Flags for material, geometry, and other type conformances
- All flag for all type conformances
- Used for filtering type conformances by kind

**Values**:
- [`None`](Source/Falcor/Scene/IScene.h:97) - No type conformances
- [`Material`](Source/Falcor/Scene/IScene.h:98) - Material type conformances (1 << 0)
- [`Geometry`](Source/Falcor/Scene/IScene.h:99) - Geometry type conformances (1 << 1)
- [`Other`](Source/Falcor/Scene/IScene.h:100) - Other type conformances (1 << 2)
- [`All`](Source/Falcor/Scene/IScene.h:101) - All type conformances

### RenderSettings Structure

**Purpose**: Render settings determining how the scene is rendered. This is used primarily by the path tracer renderers.

**Key Features**:
- Flags for enabling environment light, analytic lights, emissive lights, and grid volumes
- Diffuse albedo multiplier for material diffuse albedo
- Comparison operators for equality and inequality
- Trivially copyable for efficient passing

**Fields**:
- [`useEnvLight`](Source/Falcor/Scene/IScene.h:109) - `bool` - Enable lighting from environment map
- [`useAnalyticLights`](Source/Falcor/Scene/IScene.h:110) - `bool` - Enable lighting from analytic lights
- [`useEmissiveLights`](Source/Falcor/Scene/IScene.h:111) - `bool` - Enable lighting from emissive lights
- [`useGridVolumes`](Source/Falcor/Scene/IScene.h:112) - `bool` - Enable rendering of grid volumes
- [`diffuseAlbedoMultiplier`](Source/Falcor/Scene/IScene.h:115) - `float` - Fixed multiplier applied to material diffuse albedo (DEMO21)

**Methods**:
- [`operator==(const RenderSettings& other) const`](Source/Falcor/Scene/IScene.h:117) - Equality comparison operator
- [`operator!=(const RenderSettings& other) const`](Source/Falcor/Scene/IScene.h:125) - Inequality comparison operator

**Constraints**:
- Must be trivially copyable: `static_assert(std::is_trivially_copyable<RenderSettings>())`

### IScene Class

**Purpose**: Abstract base class for scene implementations.

**Key Features**:
- Pure virtual interface for scene operations
- Update flags signal for scene change notifications
- Shader data binding for rasterization and ray tracing
- Light collection management
- Camera access
- Material system access
- Environment map access
- Render settings management
- Ray tracing support

**Inheritance**:
- Inherits from [`Object`](Source/Falcor/Scene/IScene.h:50)

**Virtual Methods**:

**Device Access**:
- [`virtual const ref<Device>& getDevice() const = 0`](Source/Falcor/Scene/IScene.h:132) - Get the graphics device

**Update Flags Signal**:
- [`virtual UpdateFlagsSignal::Interface getUpdateFlagsSignal() = 0`](Source/Falcor/Scene/IScene.h:136) - Get the update flags signal for scene change notifications

**Shader Data**:
- [`virtual void getShaderDefines(DefineList& defines) const = 0`](Source/Falcor/Scene/IScene.h:139) - Get all defines required by the Scene
- [`virtual void getTypeConformances(TypeConformanceList& conformances, TypeConformancesKind kind = TypeConformancesKind::All) const = 0`](Source/Falcor/Scene/IScene.h:141) - Get all type conformances required by the Scene
- [`virtual void getShaderModules(ProgramDesc::ShaderModuleList& shaderModuleList) const = 0`](Source/Falcor/Scene/IScene.h:143) - Get all shader modules required by the Scene
- [`virtual void bindShaderData(const ShaderVar& sceneVar) const = 0`](Source/Falcor/Scene/IScene.h:146) - Assign all required variables into the Scene slang object, except TLAS and RayTypeCount. Pass in the ShaderVar, the Scene will bind to the correct global var name.
- [`virtual void bindShaderDataForRaytracing(RenderContext* renderContext, const ShaderVar& sceneVar, uint32_t rayTypeCount = 0) = 0`](Source/Falcor/Scene/IScene.h:151) - On-demand creates TLAS and binds and the associated rayTypeCount to the scene. If rayTypeCount is not specified (== 0), will bind any available existing TLAS (creating it for rayTypeCount == 1 if no TLAS exists). Otherwise will bind TLAS for the specified rayTypeCount. Also calls bindShaderData().

**Render Settings**:
- [`virtual const RenderSettings& getRenderSettings() const = 0`](Source/Falcor/Scene/IScene.h:154) - Get the render settings

**Ray Tracing**:
- [`virtual RtPipelineFlags getRtPipelineFlags() const = 0`](Source/Falcor/Scene/IScene.h:157) - Return the RtPipelineFlags relevant for the scene

**Scene Bounds**:
- [`virtual const AABB& getSceneBounds() const = 0`](Source/Falcor/Scene/IScene.h:160) - Get AABB for the currently updated scene

**Lighting**:
- [`virtual bool useEmissiveLights() const = 0`](Source/Falcor/Scene/IScene.h:165) - True when emissive lights are both enabled and present in the scene. Emissive lights are meshes with emissive material. TODO: Rename to GeometricLights (all lights are by definition emissive...)
- [`virtual bool useEnvLight() const = 0`](Source/Falcor/Scene/IScene.h:168) - True when environment map is both enabled and present in the scene
- [`virtual bool useAnalyticLights() const = 0`](Source/Falcor/Scene/IScene.h:170) - True when analytic lights are both enabled and present in the scene
- [`virtual const ref<EnvMap>& getEnvMap() const = 0`](Source/Falcor/Scene/IScene.h:172) - Get the environment map
- [`virtual ref<ILightCollection> getILightCollection(RenderContext* renderContext) = 0`](Source/Falcor/Scene/IScene.h:175) - Get the light collection. TODO: Remove the `renderContext` when not needed. TODO: Ideally this wouldn't be non-const and build-on-demand (as Scene1 does)
- [`virtual const std::vector<ref<Light>>& getActiveAnalyticLights() const = 0`](Source/Falcor/Scene/IScene.h:180) - Returns list of active analytic lights. TODO: This implementation requires lights to be tightly repacked every time activity changes, which might be suboptimal for editing, but better for rendering. Needs to be investigated.

**Camera**:
- [`virtual const ref<Camera>& getCamera() const = 0`](Source/Falcor/Scene/IScene.h:182) - Get the camera

**Material System**:
- [`virtual const MaterialSystem& getMaterialSystem() const = 0`](Source/Falcor/Scene/IScene.h:185) - Get the material system. Used to get type conformances for materials.

**Texture Sampling**:
- [`virtual void setDefaultTextureSampler(const ref<Sampler>& pSampler) = 0`](Source/Falcor/Scene/IScene.h:189) - Allows changing the default texture sampler based on the pass requirements. This will be applied to all materials, old and new.

**Compatibility Calls**:
- [`virtual void raytrace(RenderContext* renderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)`](Source/Falcor/Scene/IScene.h:194) - Convenience function when the Scene wants to do something besides just calling raytrace. TODO: Remove when no longer useful

**Implementation**:
```cpp
void IScene::raytrace(RenderContext* renderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)
{
    bindShaderDataForRaytracing(renderContext, pVars->getRootVar()["gScene"], pVars->getRayTypeCount());
    renderContext->raytrace(pProgram, pVars.get(), dispatchDims.x, dispatchDims.y, dispatchDims.z);
}
```

## Technical Details

### Update Flags System

**Purpose**: Track changes in the scene to optimize rendering and shader compilation.

**Update Flags Categories**:
- Geometry changes: GeometryMoved, GeometryChanged, MeshesChanged, CurvesMoved, CustomPrimitivesMoved
- Camera changes: CameraMoved, CameraPropertiesChanged, CameraSwitched
- Light changes: LightsMoved, LightIntensityChanged, LightPropertiesChanged, LightCollectionChanged, LightCountChanged, EmissiveMaterialsChanged
- Material changes: MaterialsChanged, DisplacementChanged
- Environment map changes: EnvMapChanged, EnvMapPropertiesChanged
- Grid volume changes: GridVolumesMoved, GridVolumePropertiesChanged, GridVolumeGridsChanged, GridVolumeBoundsChanged
- SDF grid changes: SDFGridConfigChanged, SDFGeometryChanged
- Scene graph changes: SceneGraphChanged
- Render settings changes: RenderSettingsChanged
- Shader changes: SceneDefinesChanged, TypeConformancesChanged, ShaderCodeChanged

**RecompileNeeded Flag**:
- Combination of: `SceneDefinesChanged | TypeConformancesChanged | ShaderCodeChanged`
- Indicates that programs accessing the scene need to be recompiled
- Used to minimize recompilation overhead by only recompiling when necessary
- Shader recompilation is costly, so this flag helps avoid unnecessary recompilations

**Signal-Based Updates**:
- Uses sigs library for signal-based update notifications
- [`UpdateFlagsSignal`](Source/Falcor/Scene/IScene.h:134) - Signal type for update flags
- [`getUpdateFlagsSignal()`](Source/Falcor/Scene/IScene.h:136) - Get the update flags signal interface
- Allows multiple subscribers to receive update notifications
- Enables efficient scene update propagation

### Type Conformances System

**Purpose**: Provide type conformances for shader type matching.

**Type Conformance Kinds**:
- Material: Type conformances for materials
- Geometry: Type conformances for geometry
- Other: Type conformances for other types

**Type Conformance Methods**:
- [`getTypeConformances(TypeConformanceList& conformances, TypeConformancesKind kind = TypeConformancesKind::All)`](Source/Falcor/Scene/IScene.h:141) - Get all type conformances required by the Scene
- Supports filtering by kind (Material, Geometry, Other, All)
- Used for shader type matching and polymorphism

### Render Settings System

**Purpose**: Determine how the scene is rendered.

**Render Settings Fields**:
- [`useEnvLight`](Source/Falcor/Scene/IScene.h:109) - Enable/disable environment map lighting
- [`useAnalyticLights`](Source/Falcor/Scene/IScene.h:110) - Enable/disable analytic lights (point, directional, spot lights)
- [`useEmissiveLights`](Source/Falcor/Scene/IScene.h:111) - Enable/disable emissive lights (mesh lights)
- [`useGridVolumes`](Source/Falcor/Scene/IScene.h:112) - Enable/disable grid volume rendering
- [`diffuseAlbedoMultiplier`](Source/Falcor/Scene/IScene.h:115) - Multiplier for material diffuse albedo (DEMO21)

**Render Settings Comparison**:
- Equality operator: Compares all fields
- Inequality operator: Negation of equality
- Used for detecting render settings changes

**RenderSettingsChanged Flag**:
- Set when render settings change
- Triggers scene update notifications
- May require shader recompilation if shader code changes

### Shader Data Binding

**Purpose**: Bind scene data to shader variables.

**Shader Data Binding Methods**:
- [`getShaderDefines(DefineList& defines)`](Source/Falcor/Scene/IScene.h:139) - Get all defines required by the Scene
  - Defines are compile-time constants for shaders
  - Used for shader compilation
  - May include feature flags, configuration options, etc.

- [`getTypeConformances(TypeConformanceList& conformances, TypeConformancesKind kind)`](Source/Falcor/Scene/IScene.h:141) - Get all type conformances required by the Scene
  - Type conformances for shader type matching
  - Used for polymorphic shader types
  - Supports filtering by kind (Material, Geometry, Other, All)

- [`getShaderModules(ProgramDesc::ShaderModuleList& shaderModuleList)`](Source/Falcor/Scene/IScene.h:143) - Get all shader modules required by the Scene
  - Shader modules for scene-specific functionality
  - Includes scene shaders, material shaders, etc.
  - Used for shader program compilation

- [`bindShaderData(const ShaderVar& sceneVar)`](Source/Falcor/Scene/IScene.h:146) - Assign all required variables into the Scene slang object, except TLAS and RayTypeCount
  - Binds scene data to shader variables
  - Excludes TLAS (Top-Level Acceleration Structure) and RayTypeCount
  - Pass in the ShaderVar, the Scene will bind to the correct global var name
  - Used for rasterization and compute shaders

- [`bindShaderDataForRaytracing(RenderContext* renderContext, const ShaderVar& sceneVar, uint32_t rayTypeCount = 0)`](Source/Falcor/Scene/IScene.h:151) - On-demand creates TLAS and binds and the associated rayTypeCount to the scene
  - If rayTypeCount is not specified (== 0), will bind any available existing TLAS (creating it for rayTypeCount == 1 if no TLAS exists)
  - Otherwise will bind TLAS for the specified rayTypeCount
  - Also calls bindShaderData()
  - Used for ray tracing shaders

**Ray Type Count**:
- Number of ray types in the ray tracing pipeline
- Used for TLAS creation and binding
- Default value: 0 (bind existing TLAS)
- If no TLAS exists, creates one with rayTypeCount == 1

### Ray Tracing Support

**Purpose**: Provide ray tracing support for scene rendering.

**Ray Tracing Methods**:
- [`getRtPipelineFlags()`](Source/Falcor/Scene/IScene.h:157) - Return the RtPipelineFlags relevant for the scene
  - Flags for ray tracing pipeline configuration
  - May include hit group flags, ray generation flags, etc.
  - Used for ray tracing pipeline creation

- [`bindShaderDataForRaytracing(RenderContext* renderContext, const ShaderVar& sceneVar, uint32_t rayTypeCount = 0)`](Source/Falcor/Scene/IScene.h:151) - Bind shader data for ray tracing
  - On-demand creates TLAS
  - Binds TLAS and rayTypeCount to the scene
  - Calls bindShaderData() to bind other scene data

- [`raytrace(RenderContext* renderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)`](Source/Falcor/Scene/IScene.h:194) - Convenience function for ray tracing
  - Binds shader data for ray tracing
  - Calls renderContext->raytrace() to execute ray tracing
  - Used when the Scene wants to do something besides just calling raytrace
  - TODO: Remove when no longer useful

**TLAS (Top-Level Acceleration Structure)**:
- Created on-demand when ray tracing is needed
- Binds to scene shader variable
- Supports multiple ray types
- Reused if already created with compatible ray type count

### Light Collection Management

**Purpose**: Manage light collection for scene rendering.

**Light Collection Methods**:
- [`useEmissiveLights()`](Source/Falcor/Scene/IScene.h:165) - True when emissive lights are both enabled and present in the scene
  - Emissive lights are meshes with emissive material
  - Used for enabling/disabling emissive light rendering
  - TODO: Rename to GeometricLights (all lights are by definition emissive...)

- [`useEnvLight()`](Source/Falcor/Scene/IScene.h:168) - True when environment map is both enabled and present in the scene
  - Used for enabling/disabling environment map lighting
  - Environment map provides background illumination

- [`useAnalyticLights()`](Source/Falcor/Scene/IScene.h:170) - True when analytic lights are both enabled and present in the scene
  - Analytic lights include point, directional, and spot lights
  - Used for enabling/disabling analytic light rendering

- [`getEnvMap()`](Source/Falcor/Scene/IScene.h:172) - Get the environment map
  - Returns reference to environment map
  - Used for environment map sampling
  - May be null if no environment map is present

- [`getILightCollection(RenderContext* renderContext)`](Source/Falcor/Scene/IScene.h:175) - Get the light collection
  - Returns reference to light collection
  - Light collection contains mesh lights
  - TODO: Remove the `renderContext` when not needed
  - TODO: Ideally this wouldn't be non-const and build-on-demand (as Scene1 does)

- [`getActiveAnalyticLights()`](Source/Falcor/Scene/IScene.h:180) - Returns list of active analytic lights
  - Returns vector of Light references
  - Lights are tightly repacked every time activity changes
  - May be suboptimal for editing, but better for rendering
  - TODO: Needs to be investigated

**Light Update Flags**:
- [`LightsMoved`](Source/Falcor/Scene/IScene.h:61) - Lights were moved
- [`LightIntensityChanged`](Source/Falcor/Scene/IScene.h:62) - Light intensity changed
- [`LightPropertiesChanged`](Source/Falcor/Scene/IScene.h:63) - Other light changes not included in LightIntensityChanged and LightsMoved
- [`LightCollectionChanged`](Source/Falcor/Scene/IScene.h:65) - Light collection changed (mesh lights)
- [`LightCountChanged`](Source/Falcor/Scene/IScene.h:69) - Number of active lights changed
- [`EmissiveMaterialsChanged`](Source/Falcor/Scene/IScene.h:85) - Emissive materials changed

### Camera Access

**Purpose**: Provide access to the scene camera.

**Camera Methods**:
- [`getCamera()`](Source/Falcor/Scene/IScene.h:182) - Get the camera
  - Returns reference to camera
  - Used for camera parameter access
  - Camera provides view and projection matrices

**Camera Update Flags**:
- [`CameraMoved`](Source/Falcor/Scene/IScene.h:58) - The camera moved
- [`CameraPropertiesChanged`](Source/Falcor/Scene/IScene.h:59) - Some camera properties changed, excluding position
- [`CameraSwitched`](Source/Falcor/Scene/IScene.h:60) - Selected a different camera

### Material System Access

**Purpose**: Provide access to the material system.

**Material System Methods**:
- [`getMaterialSystem()`](Source/Falcor/Scene/IScene.h:185) - Get the material system
  - Returns reference to material system
  - Used to get type conformances for materials
  - Material system manages material creation and access

**Material Update Flags**:
- [`MaterialsChanged`](Source/Falcor/Scene/IScene.h:66) - Materials changed
- [`DisplacementChanged`](Source/Falcor/Scene/IScene.h:78) - Displacement mapping parameters changed
- [`EmissiveMaterialsChanged`](Source/Falcor/Scene/IScene.h:85) - Emissive materials changed

### Texture Sampling

**Purpose**: Manage default texture sampler for materials.

**Texture Sampling Methods**:
- [`setDefaultTextureSampler(const ref<Sampler>& pSampler)`](Source/Falcor/Scene/IScene.h:189) - Set the default texture sampler
  - Changes the default texture sampler based on pass requirements
  - Applied to all materials, old and new
  - Used for texture sampling configuration

### Scene Bounds

**Purpose**: Provide scene bounding box for culling and optimization.

**Scene Bounds Methods**:
- [`getSceneBounds()`](Source/Falcor/Scene/IScene.h:160) - Get AABB for the currently updated scene
  - Returns reference to AABB
  - Used for culling and optimization
  - AABB represents scene extent

## Integration Points

### Core Integration

**Object Base Class**:
- Inherits from [`Object`](Source/Falcor/Scene/IScene.h:50)
- Provides reference counting
- Enables shared ownership

**Device Access**:
- [`getDevice()`](Source/Falcor/Scene/IScene.h:132) - Get the graphics device
- Used for resource creation
- Provides access to GPU device

**Program Integration**:
- [`getShaderDefines()`](Source/Falcor/Scene/IScene.h:139) - Get shader defines for program compilation
- [`getTypeConformances()`](Source/Falcor/Scene/IScene.h:141) - Get type conformances for program compilation
- [`getShaderModules()`](Source/Falcor/Scene/IScene.h:143) - Get shader modules for program compilation
- [`bindShaderData()`](Source/Falcor/Scene/IScene.h:146) - Bind scene data to shader variables
- [`raytrace()`](Source/Falcor/Scene/IScene.h:194) - Execute ray tracing with program

**Render Context Integration**:
- [`bindShaderDataForRaytracing()`](Source/Falcor/Scene/IScene.h:151) - Bind shader data for ray tracing
- [`getILightCollection()`](Source/Falcor/Scene/IScene.h:175) - Get light collection with render context
- [`raytrace()`](Source/Falcor/Scene/IScene.h:194) - Execute ray tracing with render context

**Ray Tracing Integration**:
- [`getRtPipelineFlags()`](Source/Falcor/Scene/IScene.h:157) - Get ray tracing pipeline flags
- [`bindShaderDataForRaytracing()`](Source/Falcor/Scene/IScene.h:151) - Bind TLAS for ray tracing
- [`raytrace()`](Source/Falcor/Scene/IScene.h:194) - Execute ray tracing

### Scene Integration

**EnvMap Integration**:
- [`getEnvMap()`](Source/Falcor/Scene/IScene.h:172) - Get environment map
- [`useEnvLight()`](Source/Falcor/Scene/IScene.h:168) - Check if environment map is enabled
- [`EnvMapChanged`](Source/Falcor/Scene/IScene.h:67) - Environment map changed flag
- [`EnvMapPropertiesChanged`](Source/Falcor/Scene/IScene.h:68) - Environment map properties changed flag

**Light Integration**:
- [`getILightCollection()`](Source/Falcor/Scene/IScene.h:175) - Get light collection
- [`getActiveAnalyticLights()`](Source/Falcor/Scene/IScene.h:180) - Get active analytic lights
- [`useEmissiveLights()`](Source/Falcor/Scene/IScene.h:165) - Check if emissive lights are enabled
- [`useEnvLight()`](Source/Falcor/Scene/IScene.h:168) - Check if environment map is enabled
- [`useAnalyticLights()`](Source/Falcor/Scene/IScene.h:170) - Check if analytic lights are enabled

**Camera Integration**:
- [`getCamera()`](Source/Falcor/Scene/IScene.h:182) - Get camera
- [`CameraMoved`](Source/Falcor/Scene/IScene.h:58) - Camera moved flag
- [`CameraPropertiesChanged`](Source/Falcor/Scene/IScene.h:59) - Camera properties changed flag
- [`CameraSwitched`](Source/Falcor/Scene/IScene.h:60) - Camera switched flag

**Material Integration**:
- [`getMaterialSystem()`](Source/Falcor/Scene/IScene.h:185) - Get material system
- [`setDefaultTextureSampler()`](Source/Falcor/Scene/IScene.h:189) - Set default texture sampler
- [`MaterialsChanged`](Source/Falcor/Scene/IScene.h:66) - Materials changed flag
- [`DisplacementChanged`](Source/Falcor/Scene/IScene.h:78) - Displacement changed flag
- [`EmissiveMaterialsChanged`](Source/Falcor/Scene/IScene.h:85) - Emissive materials changed flag

**Geometry Integration**:
- [`getSceneBounds()`](Source/Falcor/Scene/IScene.h:160) - Get scene bounds
- [`GeometryMoved`](Source/Falcor/Scene/IScene.h:57) - Geometry moved flag
- [`GeometryChanged`](Source/Falcor/Scene/IScene.h:77) - Geometry changed flag
- [`MeshesChanged`](Source/Falcor/Scene/IScene.h:81) - Meshes changed flag
- [`CurvesMoved`](Source/Falcor/Scene/IScene.h:75) - Curves moved flag
- [`CustomPrimitivesMoved`](Source/Falcor/Scene/IScene.h:76) - Custom primitives moved flag

**Grid Volume Integration**:
- [`GridVolumesMoved`](Source/Falcor/Scene/IScene.h:71) - Grid volumes moved flag
- [`GridVolumePropertiesChanged`](Source/Falcor/Scene/IScene.h:72) - Grid volume properties changed flag
- [`GridVolumeGridsChanged`](Source/Falcor/Scene/IScene.h:73) - Grid volume grids changed flag
- [`GridVolumeBoundsChanged`](Source/Falcor/Scene/IScene.h:74) - Grid volume bounds changed flag

**SDF Grid Integration**:
- [`SDFGridConfigChanged`](Source/Falcor/Scene/IScene.h:79) - SDF grid config changed flag
- [`SDFGeometryChanged`](Source/Falcor/Scene/IScene.h:80) - SDF geometry changed flag

### Signal Integration

**Signal Library**:
- Uses sigs library for signal-based update notifications
- [`UpdateFlagsSignal`](Source/Falcor/Scene/IScene.h:134) - Signal type for update flags
- [`getUpdateFlagsSignal()`](Source/Falcor/Scene/IScene.h:136) - Get update flags signal interface
- Enables multiple subscribers to receive update notifications

## Architecture Patterns

### Interface Pattern

- Pure virtual interface for scene operations
- All methods are pure virtual (except raytrace compatibility method)
- Enables polymorphic scene implementations
- Allows different scene implementations to be used interchangeably

### Observer Pattern

- Signal-based update notifications
- Multiple subscribers can receive update notifications
- Update flags indicate what changed in the scene
- Enables efficient scene update propagation

### Strategy Pattern

- Different render settings for different rendering strategies
- Enable/disable different light types
- Enable/disable different geometry types
- Configure rendering behavior

### Factory Pattern

- On-demand TLAS creation
- On-demand light collection creation
- Lazy initialization of expensive resources
- Reduces memory usage and initialization time

### Template Method Pattern

- Common interface for shader data binding
- Different implementations for rasterization and ray tracing
- bindShaderData() for rasterization
- bindShaderDataForRaytracing() for ray tracing

## Code Patterns

### Update Flags Pattern

```cpp
enum class UpdateFlags
{
    None = 0x0,
    GeometryMoved = 0x1,
    CameraMoved = 0x2,
    CameraPropertiesChanged = 0x4,
    CameraSwitched = 0x8,
    LightsMoved = 0x10,
    LightIntensityChanged = 0x20,
    LightPropertiesChanged = 0x40,
    SceneGraphChanged = 0x80,
    LightCollectionChanged = 0x100,
    MaterialsChanged = 0x200,
    EnvMapChanged = 0x400,
    EnvMapPropertiesChanged = 0x800,
    LightCountChanged = 0x1000,
    RenderSettingsChanged = 0x2000,
    GridVolumesMoved = 0x4000,
    GridVolumePropertiesChanged = 0x8000,
    GridVolumeGridsChanged = 0x10000,
    GridVolumeBoundsChanged = 0x20000,
    CurvesMoved = 0x40000,
    CustomPrimitivesMoved = 0x80000,
    GeometryChanged = 0x100000,
    DisplacementChanged = 0x200000,
    SDFGridConfigChanged = 0x400000,
    SDFGeometryChanged = 0x800000,
    MeshesChanged = 0x1000000,
    SceneDefinesChanged = 0x2000000,
    TypeConformancesChanged = 0x4000000,
    ShaderCodeChanged = 0x8000000,
    EmissiveMaterialsChanged = 0x10000000,
    RecompileNeeded = SceneDefinesChanged | TypeConformancesChanged | ShaderCodeChanged,
    All = -1
};
```

### Render Settings Pattern

```cpp
struct RenderSettings
{
    bool useEnvLight = true;
    bool useAnalyticLights = true;
    bool useEmissiveLights = true;
    bool useGridVolumes = true;
    float diffuseAlbedoMultiplier = 1.f;

    bool operator==(const RenderSettings& other) const
    {
        return (useEnvLight == other.useEnvLight) &&
            (useAnalyticLights == other.useAnalyticLights) &&
            (useEmissiveLights == other.useEmissiveLights) &&
            (useGridVolumes == other.useGridVolumes);
    }

    bool operator!=(const RenderSettings& other) const { return !(*this == other); }
};
```

### Shader Data Binding Pattern

```cpp
virtual void bindShaderData(const ShaderVar& sceneVar) const = 0;
virtual void bindShaderDataForRaytracing(RenderContext* renderContext, const ShaderVar& sceneVar, uint32_t rayTypeCount = 0) = 0;
```

### Ray Tracing Pattern

```cpp
void IScene::raytrace(RenderContext* renderContext, Program* pProgram, const ref<RtProgramVars>& pVars, uint3 dispatchDims)
{
    bindShaderDataForRaytracing(renderContext, pVars->getRootVar()["gScene"], pVars->getRayTypeCount());
    renderContext->raytrace(pProgram, pVars.get(), dispatchDims.x, dispatchDims.y, dispatchDims.z);
}
```

### Signal Pattern

```cpp
using UpdateFlagsSignal = sigs::Signal<void(IScene::UpdateFlags)>;
virtual UpdateFlagsSignal::Interface getUpdateFlagsSignal() = 0;
```

## Use Cases

### Rendering

- **Path Tracing**:
  - Get render settings for path tracer configuration
  - Bind shader data for path tracing
  - Get light collection for light sampling
  - Get scene bounds for culling

- **Ray Tracing**:
  - Get ray tracing pipeline flags
  - Bind shader data for ray tracing
  - Create TLAS on-demand
  - Execute ray tracing

- **Rasterization**:
  - Bind shader data for rasterization
  - Get camera for view/projection matrices
  - Get scene bounds for culling
  - Get material system for material access

### Scene Updates

- **Scene Change Detection**:
  - Subscribe to update flags signal
  - Detect what changed in the scene
  - Update rendering accordingly
  - Recompile shaders if needed

- **Shader Recompilation**:
  - Check RecompileNeeded flag
  - Recompile programs that access the scene
  - Update shader defines
  - Update type conformances

### Light Management

- **Light Sampling**:
  - Get active analytic lights for light sampling
  - Get light collection for mesh light sampling
  - Check if environment map is enabled
  - Check if emissive lights are enabled

- **Light Updates**:
  - Detect light intensity changes
  - Detect light position changes
  - Detect light property changes
  - Update light collection

### Material Management

- **Material Access**:
  - Get material system for material access
  - Set default texture sampler
  - Detect material changes
  - Detect displacement changes

### Camera Management

- **Camera Access**:
  - Get camera for view/projection matrices
  - Detect camera movement
  - Detect camera property changes
  - Detect camera switching

## Performance Considerations

### Shader Compilation

**RecompileNeeded Flag**:
- Minimizes shader recompilation by tracking changes that require recompilation
- Only recompiles when scene defines, type conformances, or shader code changes
- Avoids unnecessary recompilation for other changes
- Shader recompilation is costly, so this optimization is important

**Shader Data Binding**:
- Efficient binding of scene data to shader variables
- On-demand TLAS creation for ray tracing
- Reuses existing TLAS if compatible
- Minimizes resource creation overhead

### Light Management

**Light Collection**:
- On-demand light collection creation
- Tightly repacked active analytic lights for rendering
- May be suboptimal for editing, but better for rendering
- TODO: Needs to be investigated

**Light Update Detection**:
- Granular update flags for different light changes
- Avoids unnecessary light updates
- Enables efficient light update propagation

### Memory Usage

**TLAS**:
- Created on-demand
- Reused if compatible with ray type count
- Minimizes memory usage

**Light Collection**:
- Built on-demand
- Contains only active lights
- Minimizes memory usage

### Signal-Based Updates

**Update Notifications**:
- Signal-based update notifications
- Multiple subscribers can receive notifications
- Efficient update propagation
- Minimizes unnecessary updates

## Limitations

### Feature Limitations

- **Pure Virtual Interface**:
  - Cannot be instantiated directly
  - Requires concrete implementation
  - All methods must be implemented

- **Light Collection**:
  - Requires render context parameter
  - Non-const method (builds on-demand)
  - May be suboptimal for editing

- **Active Analytic Lights**:
  - Lights are tightly repacked every time activity changes
  - May be suboptimal for editing
  - TODO: Needs to be investigated

### Performance Limitations

- **Light Collection**:
  - Build-on-demand may cause performance spikes
  - Non-const method prevents const usage
  - TODO: Remove renderContext parameter when not needed

- **Active Analytic Lights**:
  - Tight repacking may be expensive
  - May be suboptimal for editing
  - TODO: Needs to be investigated

### Integration Limitations

- **Scene Coupling**:
  - Tightly coupled to Falcor scene system
  - Requires specific scene implementation
  - Not suitable for standalone use

- **Compatibility Methods**:
  - raytrace() method is marked as TODO: Remove when no longer useful
  - May be deprecated in future
  - Should not be relied upon

### Debugging Limitations

- **Limited Error Reporting**:
  - No detailed error messages in interface
  - No logging of failures
  - Difficult to debug scene issues
  - No validation of input parameters

## Best Practices

### Update Flags

- **Granular Updates**:
  - Use specific update flags for different changes
  - Avoid using All flag unnecessarily
  - Minimize update propagation
  - Enable efficient scene updates

- **RecompileNeeded Flag**:
  - Check RecompileNeeded flag before recompiling
  - Only recompile when necessary
  - Minimize shader recompilation overhead
  - Update all programs that access the scene

### Shader Data Binding

- **Bind Once**:
  - Bind shader data once per frame
  - Reuse shader bindings
  - Minimize binding overhead
  - Use appropriate binding method for rendering mode

- **Ray Tracing**:
  - Use bindShaderDataForRaytracing() for ray tracing
  - Specify ray type count for TLAS creation
  - Reuse existing TLAS if compatible
  - Minimize TLAS creation overhead

### Light Management

- **Light Updates**:
  - Use granular light update flags
  - Avoid unnecessary light updates
  - Update light collection when needed
  - Detect light changes efficiently

- **Light Sampling**:
  - Check if light types are enabled before sampling
  - Use light collection for mesh lights
  - Use active analytic lights for analytic lights
  - Use environment map for environment lighting

### Material Management

- **Material Access**:
  - Get material system for material access
  - Set default texture sampler for all materials
  - Detect material changes efficiently
  - Update materials when needed

### Camera Management

- **Camera Access**:
  - Get camera for view/projection matrices
  - Detect camera movement
  - Detect camera property changes
  - Detect camera switching

### Performance Optimization

- **On-Demand Resource Creation**:
  - Create TLAS on-demand
  - Create light collection on-demand
  - Reuse existing resources if compatible
  - Minimize resource creation overhead

- **Shader Recompilation**:
  - Check RecompileNeeded flag before recompiling
  - Only recompile when necessary
  - Minimize shader recompilation overhead
  - Update all programs that access the scene

### Debugging

- **Validate Input Parameters**:
  - Validate shader variables before binding
  - Validate ray type count before creating TLAS
  - Validate render context before using
  - Check for null references

- **Profile Performance**:
  - Measure shader compilation time
  - Profile shader data binding
  - Profile light collection creation
  - Identify bottlenecks

## Progress Log

- **2026-01-07T23:56:00Z**: IScene analysis completed. Analyzed IScene.h (199 lines) and IScene.cpp (45 lines). Documented IScene abstract base class with UpdateFlags enumeration (26 flags), TypeConformancesKind enumeration, RenderSettings structure, and IScene interface with pure virtual methods for device access, update flags signal, shader data binding, render settings, ray tracing, scene bounds, lighting, camera, material system, and texture sampling. Documented update flags system for tracking scene changes, type conformances system for shader type matching, render settings system for rendering configuration, shader data binding for rasterization and ray tracing, ray tracing support with on-demand TLAS creation, light collection management, camera access, material system access, texture sampling, and scene bounds. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The IScene module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
