# Material - Scene Material Management System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Material** - Base material class
- [x] **MaterialSystem** - Material system manager
- [x] **MaterialData** - Material data structures
- [x] **MaterialTypeRegistry** - Material type registration
- [x] **BasicMaterial** - Base class for basic non-layered materials
- [x] **StandardMaterial** - Standard material (MetalRough, SpecGloss)
- [x] **ClothMaterial** - Cloth material
- [x] **HairMaterial** - Hair material
- [x] **MERLMaterial** - MERL BRDF material
- [x] **RGLMaterial** - RGL BRDF material
- [x] **MERLMixMaterial** - MERL mix material
- [x] **TextureHandle** - Texture handle for GPU binding
- [x] **MaterialParamLayout** - Material parameter layout
- [x] **SerializedMaterialParams** - Serialized material parameters
- [x] **MaterialTextureLoader** - Material texture loading
- [x] **DiffuseSpecularUtils** - Diffuse-specular utilities

### External Dependencies

- **Core/Object** - Base object class
- **Core/API** - Graphics API (Device, Texture, Sampler, Buffer, Formats)
- **Core/Program** - Shader program management (Program, DefineList, TypeConformanceList)
- **Core/API/ParameterBlock** - Parameter block for shader binding
- **Utils/Math** - Mathematical utilities (Vector, Matrix, Transform)
- **Utils/UI** - GUI widgets
- **Utils/Image** - Image processing (TextureManager, TextureAnalyzer)
- **Scene/Transform** - Transform utilities
- **Scene/SceneCache** - Scene caching

## Module Overview

The Material system provides comprehensive material management for the Falcor rendering framework. It supports multiple material types including standard physically-based materials (MetalRough, SpecGloss), specialized materials (cloth, hair), and measured BRDF materials (MERL, RGL). The system manages material data, textures, samplers, and GPU resources with efficient update tracking and optimization capabilities.

## Component Specifications

### Material

**File**: [`Material.h`](Source/Falcor/Scene/Material/Material.h:1)

**Purpose**: Abstract base class for all materials.

**Key Features**:
- Update flags tracking (CodeChanged, DataChanged, ResourcesChanged, DisplacementChanged, EmissiveChanged)
- Texture slot management (BaseColor, Specular, Emissive, Normal, Transmission, Displacement, Index)
- Material properties (double-sided, thin surface, alpha mode, nested priority, index of refraction)
- Texture optimization support
- Shader data binding
- UI rendering
- Material comparison
- Dynamic material support

**Update Flags** (from [`Material::UpdateFlags`](Source/Falcor/Scene/Material/Material.h:63)):
- **None** - Nothing updated
- **CodeChanged** - Material shader code changed
- **DataChanged** - Material data (parameters) changed
- **ResourcesChanged** - Material resources (textures, buffers, samplers) changed
- **DisplacementChanged** - Displacement mapping parameters changed
- **EmissiveChanged** - Material emissive properties changed

**Texture Slots** (from [`Material::TextureSlot`](Source/Falcor/Scene/Material/Material.h:76)):
- **BaseColor** - Base color texture
- **Specular** - Specular texture
- **Emissive** - Emissive texture
- **Normal** - Normal map
- **Transmission** - Transmission texture
- **Displacement** - Displacement map
- **Index** - Index texture (for MERLMix material)
- **Count** - Total number of texture slots

**Core Methods**:

**Construction**:
- [`Material()`](Source/Falcor/Scene/Material/Material.h:353) - Constructor with device, name, and type
- [`~Material()`](Source/Falcor/Scene/Material/Material.h:118) - Virtual destructor

**State Management**:
- [`update()`](Source/Falcor/Scene/Material/Material.h:129) - Update material for rendering
- [`renderUI()`](Source/Falcor/Scene/Material/Material.h:123) - Render UI elements
- [`setName()`](Source/Falcor/Scene/Material/Material.h:133) - Set material name
- [`getName()`](Source/Falcor/Scene/Material/Material.h:137) - Get material name
- [`getType()`](Source/Falcor/Scene/Material/Material.h:141) - Get material type

**Material Properties**:
- [`isOpaque()`](Source/Falcor/Scene/Material/Material.h:145) - Check if material is opaque
- [`isDisplaced()`](Source/Falcor/Scene/Material/Material.h:149) - Check if material has displacement map
- [`isEmissive()`](Source/Falcor/Scene/Material/Material.h:153) - Check if material is emissive
- [`isDynamic()`](Source/Falcor/Scene/Material/Material.h:158) - Check if material is dynamic
- [`setDoubleSided()`](Source/Falcor/Scene/Material/Material.h:168) - Set double-sided flag
- [`isDoubleSided()`](Source/Falcor/Scene/Material/Material.h:172) - Check if material is double-sided
- [`setThinSurface()`](Source/Falcor/Scene/Material/Material.h:176) - Set thin surface flag
- [`isThinSurface()`](Source/Falcor/Scene/Material/Material.h:180) - Check if material is thin surface
- [`setAlphaMode()`](Source/Falcor/Scene/Material/Material.h:184) - Set alpha mode
- [`getAlphaMode()`](Source/Falcor/Scene/Material/Material.h:188) - Get alpha mode
- [`setAlphaThreshold()`](Source/Falcor/Scene/Material/Material.h:192) - Set alpha threshold
- [`getAlphaThreshold()`](Source/Falcor/Scene/Material/Material.h:196) - Get alpha threshold
- [`setNestedPriority()`](Source/Falcor/Scene/Material/Material.h:204) - Set nested priority
- [`getNestedPriority()`](Source/Falcor/Scene/Material/Material.h:209) - Get nested priority
- [`setIndexOfRefraction()`](Source/Falcor/Scene/Material/Material.h:213) - Set index of refraction
- [`getIndexOfRefraction()`](Source/Falcor/Scene/Material/Material.h:217) - Get index of refraction

**Texture Management**:
- [`getTextureSlotInfo()`](Source/Falcor/Scene/Material/Material.h:223) - Get information about texture slot
- [`hasTextureSlot()`](Source/Falcor/Scene/Material/Material.h:229) - Check if material has texture slot
- [`setTexture()`](Source/Falcor/Scene/Material/Material.h:237) - Set texture slot
- [`loadTexture()`](Source/Falcor/Scene/Material/Material.h:246) - Load texture from file
- [`clearTexture()`](Source/Falcor/Scene/Material/Material.h:252) - Clear texture slot
- [`getTexture()`](Source/Falcor/Scene/Material/Material.h:258) - Get texture from slot
- [`optimizeTexture()`](Source/Falcor/Scene/Material/Material.h:266) - Optimize texture usage
- [`getMaxTextureDimensions()`](Source/Falcor/Scene/Material/Material.h:270) - Get maximum texture dimensions
- [`setDefaultTextureSampler()`](Source/Falcor/Scene/Material/Material.h:274) - Set default texture sampler
- [`getDefaultTextureSampler()`](Source/Falcor/Scene/Material/Material.h:278) - Get default texture sampler
- [`setTextureTransform()`](Source/Falcor/Scene/Material/Material.h:282) - Set texture transform
- [`getTextureTransform()`](Source/Falcor/Scene/Material/Material.h:286) - Get texture transform

**Shader Integration**:
- [`getDataBlob()`](Source/Falcor/Scene/Material/Material.h:298) - Get material data blob for GPU
- [`getShaderModules()`](Source/Falcor/Scene/Material/Material.h:304) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/Material.h:309) - Get type conformances
- [`getDefines()`](Source/Falcor/Scene/Material/Material.h:314) - Get shader defines
- [`getMaxBufferCount()`](Source/Falcor/Scene/Material/Material.h:318) - Get number of buffers
- [`getMaxTextureCount()`](Source/Falcor/Scene/Material/Material.h:325) - Get maximum number of textures
- [`getMaxTexture3DCount()`](Source/Falcor/Scene/Material/Material.h:329) - Get number of 3D textures
- [`getMaterialInstanceByteSize()`](Source/Falcor/Scene/Material/Material.h:340) - Get material instance byte size
- [`getParamLayout()`](Source/Falcor/Scene/Material/Material.h:342) - Get parameter layout
- [`serializeParams()`](Source/Falcor/Scene/Material/Material.h:343) - Serialize parameters
- [`deserializeParams()`](Source/Falcor/Scene/Material/Material.h:344) - Deserialize parameters

**Comparison**:
- [`isEqual()`](Source/Falcor/Scene/Material/Material.h:164) - Compare material to another material

**Protected Members**:
- [`mpDevice`](Source/Falcor/Scene/Material/Material.h:377) - Device reference
- [`mName`](Source/Falcor/Scene/Material/Material.h:379) - Material name
- [`mHeader`](Source/Falcor/Scene/Material/Material.h:380) - Material header data
- [`mTextureTransform`](Source/Falcor/Scene/Material/Material.h:381) - Texture transform
- [`mTextureSlotInfo`](Source/Falcor/Scene/Material/Material.h:383) - Texture slot information
- [`mTextureSlotData`](Source/Falcor/Scene/Material/Material.h:384) - Texture slot data
- [`mUpdates`](Source/Falcor/Scene/Material/Material.h:386) - Update flags
- [`mUpdateCallback`](Source/Falcor/Scene/Material/Material.h:387) - Update callback

### MaterialSystem

**File**: [`MaterialSystem.h`](Source/Falcor/Scene/Material/MaterialSystem.h:1)

**Purpose**: Material system manager for managing materials and their resources.

**Key Features**:
- Material collection management
- Texture manager integration
- Shader module management
- Type conformance management
- GPU resource management (buffers, textures, samplers)
- Material optimization
- Duplicate material removal
- Statistics tracking
- Light profile support

**Material Stats** (from [`MaterialSystem::MaterialStats`](Source/Falcor/Scene/Material/MaterialSystem.h:62)):
- **materialTypeCount** - Number of material types
- **materialCount** - Number of materials
- **materialOpaqueCount** - Number of opaque materials
- **materialMemoryInBytes** - Total memory used by material data
- **textureCount** - Number of unique textures
- **textureCompressedCount** - Number of compressed textures
- **textureTexelCount** - Total number of texels
- **textureTexelChannelCount** - Total number of texel channels
- **textureMemoryInBytes** - Total memory used by textures

**Core Methods**:

**Construction**:
- [`MaterialSystem()`](Source/Falcor/Scene/Material/MaterialSystem.h:77) - Constructor with device

**Update**:
- [`update()`](Source/Falcor/Scene/Material/MaterialSystem.h:85) - Update material system for rendering
- [`renderUI()`](Source/Falcor/Scene/Material/MaterialSystem.h:81) - Render UI elements

**Shader Integration**:
- [`getDefines()`](Source/Falcor/Scene/Material/MaterialSystem.h:92) - Get shader defines
- [`getTypeConformances()`](Source/Falcor/Scene/Material/MaterialSystem.h:105) - Get type conformances
- [`getShaderModules()`](Source/Falcor/Scene/Material/MaterialSystem.h:123) - Get shader modules
- [`bindShaderData()`](Source/Falcor/Scene/Material/MaterialSystem.h:133) - Bind material system to shader var

**Resource Management**:
- [`setDefaultTextureSampler()`](Source/Falcor/Scene/Material/MaterialSystem.h:137) - Set default texture sampler
- [`addTextureSampler()`](Source/Falcor/Scene/Material/MaterialSystem.h:144) - Add texture sampler
- [`getTextureSamplerCount()`](Source/Falcor/Scene/Material/MaterialSystem.h:148) - Get number of texture samplers
- [`getTextureSampler()`](Source/Falcor/Scene/Material/MaterialSystem.h:152) - Get texture sampler by ID
- [`addBuffer()`](Source/Falcor/Scene/Material/MaterialSystem.h:158) - Add buffer resource
- [`replaceBuffer()`](Source/Falcor/Scene/Material/MaterialSystem.h:164) - Replace buffer
- [`getBufferCount()`](Source/Falcor/Scene/Material/MaterialSystem.h:168) - Get number of buffers
- [`addTexture3D()`](Source/Falcor/Scene/Material/MaterialSystem.h:174) - Add 3D texture
- [`getTexture3DCount()`](Source/Falcor/Scene/Material/MaterialSystem.h:178) - Get number of 3D textures

**Material Management**:
- [`addMaterial()`](Source/Falcor/Scene/Material/MaterialSystem.h:185) - Add material
- [`removeMaterial()`](Source/Falcor/Scene/Material/MaterialSystem.h:190) - Remove material
- [`replaceMaterial()`](Source/Falcor/Scene/Material/MaterialSystem.h:196) - Replace material
- [`getMaterials()`](Source/Falcor/Scene/Material/MaterialSystem.h:201) - Get list of all materials
- [`getMaterialCount()`](Source/Falcor/Scene/Material/MaterialSystem.h:205) - Get total number of materials
- [`getMaterialCountByType()`](Source/Falcor/Scene/Material/MaterialSystem.h:209) - Get number of materials by type
- [`getMaterialTypes()`](Source/Falcor/Scene/Material/MaterialSystem.h:213) - Get set of material types
- [`hasMaterialType()`](Source/Falcor/Scene/Material/MaterialSystem.h:217) - Check if material type is used
- [`hasMaterial()`](Source/Falcor/Scene/Material/MaterialSystem.h:223) - Check if material exists
- [`getMaterial()`](Source/Falcor/Scene/Material/MaterialSystem.h:229) - Get material by ID
- [`getMaterialByName()`](Source/Falcor/Scene/Material/MaterialSystem.h:234) - Get material by name
- [`removeDuplicateMaterials()`](Source/Falcor/Scene/Material/MaterialSystem.h:240) - Remove duplicate materials
- [`optimizeMaterials()`](Source/Falcor/Scene/Material/MaterialSystem.h:245) - Optimize materials

**Statistics**:
- [`getStats()`](Source/Falcor/Scene/Material/MaterialSystem.h:249) - Get material system statistics

**Texture Manager**:
- [`getTextureManager()`](Source/Falcor/Scene/Material/MaterialSystem.h:253) - Get texture manager

**Light Profile**:
- [`loadLightProfile()`](Source/Falcor/Scene/Material/MaterialSystem.h:256) - Load light profile
- [`getLightProfile()`](Source/Falcor/Scene/Material/MaterialSystem.h:258) - Get light profile

**Protected Members**:
- [`mpDevice`](Source/Falcor/Scene/Material/MaterialSystem.h:265) - Device reference
- [`mMaterials`](Source/Falcor/Scene/Material/MaterialSystem.h:267) - List of all materials
- [`mMaterialsUpdateFlags`](Source/Falcor/Scene/Material/MaterialSystem.h:268) - Material update flags
- [`mpTextureManager`](Source/Falcor/Scene/Material/MaterialSystem.h:269) - Texture manager
- [`mShaderModules`](Source/Falcor/Scene/Material/MaterialSystem.h:270) - Shader modules
- [`mTypeConformances`](Source/Falcor/Scene/Material/MaterialSystem.h:271) - Type conformances
- [`mpLightProfile`](Source/Falcor/Scene/Material/MaterialSystem.h:272) - Light profile
- [`mLightProfileBaked`](Source/Falcor/Scene/Material/MaterialSystem.h:273) - Light profile baked flag

### MaterialData

**File**: [`MaterialData.slang`](Source/Falcor/Scene/Material/MaterialData.slang:1)

**Purpose**: Host/device structures for material data.

**Key Structures**:

**MaterialHeader** (from [`MaterialHeader`](Source/Falcor/Scene/Material/MaterialData.slang:43)):
- **packedData** - Packed data (uint4, 16 bytes)
- **Material type** - Material type identifier
- **Alpha mode** - Alpha testing mode
- **Alpha threshold** - Alpha threshold value
- **Nested priority** - Nested dielectric priority
- **Active lobes** - Active BSDF lobes
- **Default sampler ID** - Default texture sampler ID
- **Double-sided flag** - Double-sided shading flag
- **Thin surface flag** - Thin surface flag
- **Emissive flag** - Emissive flag
- **Basic material flag** - Basic material optimization flag
- **Light profile enable flag** - Light profile enable flag
- **Delta specular flag** - Delta specular flag
- **Index of refraction** - IoR value
- **Alpha texture handle** - Alpha mask texture handle

**MaterialPayload** (from [`MaterialPayload`](Source/Falcor/Scene/Material/MaterialData.slang:203)):
- **data** - Payload data (uint[28], 112 bytes)

**MaterialDataBlob** (from [`MaterialDataBlob`](Source/Falcor/Scene/Material/MaterialData.slang:214)):
- **header** - Material header (16 bytes)
- **payload** - Material payload (112 bytes)
- **Total size**: 128 bytes (one cacheline for efficient access)

**MaterialHeader Bit Layout**:

**packedData.x**:
- **kMaterialTypeBits** (16 bits) - Material type
- **kNestedPriorityBits** (4 bits) - Nested priority
- **kLobeTypeBits** (8 bits) - Active BSDF lobes
- **kDoubleSidedFlagOffset** (1 bit) - Double-sided flag
- **kThinSurfaceFlagOffset** (1 bit) - Thin surface flag
- **kEmissiveFlagOffset** (1 bit) - Emissive flag
- **kIsBasicMaterialFlagOffset** (1 bit) - Basic material flag

**packedData.y**:
- **kAlphaThresholdBits** (16 bits) - Alpha threshold (float16_t)
- **kAlphaModeBits** (1 bit) - Alpha mode
- **kSamplerIDBits** (8 bits) - Default sampler ID
- **kEnableLightProfileOffset** (1 bit) - Light profile enable flag
- **kDeltaSpecularFlagOffset** (1 bit) - Delta specular flag

**packedData.z**:
- **kIoRBits** (16 bits) - Index of refraction (float16_t)

**packedData.w**:
- Alpha texture handle

### MaterialTypeRegistry

**File**: [`MaterialTypeRegistry.h`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:1)

**Purpose**: Material type registration system.

**Key Functions**:
- [`registerMaterialType()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:43) - Register new material type
- [`to_string()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:50) - Get material type name
- [`getMaterialTypeCount()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:56) - Get total number of material types
- [`getMaterialParamLayout()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:60) - Get material parameter layout

### BasicMaterial

**File**: [`BasicMaterial.h`](Source/Falcor/Scene/Material/BasicMaterial.h:1)

**Purpose**: Base class for basic non-layered materials.

**Key Features**:
- Base color, specular, emissive, transmission, normal, displacement textures
- Displacement mapping support
- Volume properties (absorption, scattering, anisotropy)
- Alpha testing support
- Texture optimization
- Double-sided shading
- Thin surface support

**Texture Channel Layout**:

**Displacement**:
- RGB - Displacement data
- A - Unused

**Core Methods**:

**Update**:
- [`update()`](Source/Falcor/Scene/Material/BasicMaterial.h:57) - Update material for rendering
- [`renderUI()`](Source/Falcor/Scene/Material/BasicMaterial.h:51) - Render UI elements
- [`isEqual()`](Source/Falcor/Scene/Material/BasicMaterial.h:67) - Compare material

**Texture Management**:
- [`setTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:83) - Set texture slot
- [`optimizeTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:91) - Optimize texture usage
- [`setDefaultTextureSampler()`](Source/Falcor/Scene/Material/BasicMaterial.h:95) - Set default texture sampler
- [`getDefaultTextureSampler()`](Source/Falcor/Scene/Material/BasicMaterial.h:99) - Get default texture sampler

**Texture Accessors**:
- [`setBaseColorTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:110) - Set base color texture
- [`getBaseColorTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:114) - Get base color texture
- [`setSpecularTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:118) - Set specular texture
- [`getSpecularTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:122) - Get specular texture
- [`setEmissiveTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:126) - Set emissive texture
- [`getEmissiveTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:130) - Get emissive texture
- [`setTransmissionTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:134) - Set transmission texture
- [`getTransmissionTexture()`](Source/Falcor/Scene/Material/BasicMaterial.h:138) - Get transmission texture
- [`setNormalMap()`](Source/Falcor/Scene/Material/BasicMaterial.h:142) - Set normal map
- [`getNormalMap()`](Source/Falcor/Scene/Material/BasicMaterial.h:146) - Get normal map
- [`setDisplacementMap()`](Source/Falcor/Scene/Material/BasicMaterial.h:150) - Set displacement map
- [`getDisplacementMap()`](Source/Falcor/Scene/Material/BasicMaterial.h:154) - Get displacement map

**Displacement**:
- [`isDisplaced()`](Source/Falcor/Scene/Material/BasicMaterial.h:61) - Check if material has displacement map
- [`setDisplacementScale()`](Source/Falcor/Scene/Material/BasicMaterial.h:158) - Set displacement scale
- [`getDisplacementScale()`](Source/Falcor/Scene/Material/BasicMaterial.h:162) - Get displacement scale
- [`setDisplacementOffset()`](Source/Falcor/Scene/Material/BasicMaterial.h:166) - Set displacement offset
- [`getDisplacementOffset()`](Source/Falcor/Scene/Material/BasicMaterial.h:170) - Get displacement offset

**Material Parameters**:
- [`setBaseColor()`](Source/Falcor/Scene/Material/BasicMaterial.h:174) - Set base color
- [`getBaseColor()`](Source/Falcor/Scene/Material/BasicMaterial.h:179) - Get base color
- [`getBaseColor3()`](Source/Falcor/Scene/Material/BasicMaterial.h:180) - Get base color (RGB)
- [`setSpecularParams()`](Source/Falcor/Scene/Material/BasicMaterial.h:184) - Set specular parameters
- [`getSpecularParams()`](Source/Falcor/Scene/Material/BasicMaterial.h:188) - Get specular parameters
- [`setTransmissionColor()`](Source/Falcor/Scene/Material/BasicMaterial.h:192) - Set transmission color
- [`getTransmissionColor()`](Source/Falcor/Scene/Material/BasicMaterial.h:196) - Get transmission color
- [`setDiffuseTransmission()`](Source/Falcor/Scene/Material/BasicMaterial.h:200) - Set diffuse transmission
- [`getDiffuseTransmission()`](Source/Falcor/Scene/Material/BasicMaterial.h:204) - Get diffuse transmission
- [`setSpecularTransmission()`](Source/Falcor/Scene/Material/BasicMaterial.h:208) - Set specular transmission
- [`getSpecularTransmission()`](Source/Falcor/Scene/Material/BasicMaterial.h:212) - Get specular transmission

**Volume Properties**:
- [`setVolumeAbsorption()`](Source/Falcor/Scene/Material/BasicMaterial.h:216) - Set volume absorption
- [`getVolumeAbsorption()`](Source/Falcor/Scene/Material/BasicMaterial.h:220) - Get volume absorption
- [`setVolumeScattering()`](Source/Falcor/Scene/Material/BasicMaterial.h:224) - Set volume scattering
- [`getVolumeScattering()`](Source/Falcor/Scene/Material/BasicMaterial.h:228) - Get volume scattering
- [`setVolumeAnisotropy()`](Source/Falcor/Scene/Material/BasicMaterial.h:232) - Set volume anisotropy
- [`getVolumeAnisotropy()`](Source/Falcor/Scene/Material/BasicMaterial.h:236) - Get volume anisotropy

**Normal Map**:
- [`getNormalMapType()`](Source/Falcor/Scene/Material/BasicMaterial.h:240) - Get normal map type

**Data Access**:
- [`getData()`](Source/Falcor/Scene/Material/BasicMaterial.h:244) - Get material data struct

**Protected Members**:
- [`mData`](Source/Falcor/Scene/Material/BasicMaterial.h:265) - Material parameters
- [`mpDefaultSampler`](Source/Falcor/Scene/Material/BasicMaterial.h:267) - Default sampler
- [`mpDisplacementMinSampler`](Source/Falcor/Scene/Material/BasicMaterial.h:268) - Displacement min sampler
- [`mpDisplacementMaxSampler`](Source/Falcor/Scene/Material/BasicMaterial.h:269) - Displacement max sampler
- [`mAlphaRange`](Source/Falcor/Scene/Material/BasicMaterial.h:272) - Alpha range
- [`mIsTexturedBaseColorConstant`](Source/Falcor/Scene/Material/BasicMaterial.h:273) - Textured base color constant flag
- [`mIsTexturedAlphaConstant`](Source/Falcor/Scene/Material/BasicMaterial.h:274) - Textured alpha constant flag
- [`mDisplacementMapChanged`](Source/Falcor/Scene/Material/BasicMaterial.h:275) - Displacement map changed flag

### StandardMaterial

**File**: [`StandardMaterial.h`](Source/Falcor/Scene/Material/StandardMaterial.h:1)

**Purpose**: Standard material with MetalRough and SpecGloss shading models.

**Key Features**:
- MetalRough shading model (base color, roughness, metallic)
- SpecGloss shading model (diffuse color, specular color, gloss)
- Emissive color and factor
- Light profile support
- Differentiable rendering support

**Texture Channel Layout**:

**ShadingModel::MetalRough**:
- **BaseColor**: RGB - Base Color, A - Opacity
- **Specular**: R - Unused, G - Roughness, B - Metallic, A - Unused

**ShadingModel::SpecGloss**:
- **BaseColor**: RGB - Diffuse Color, A - Opacity
- **Specular**: RGB - Specular Color, A - Gloss

**All Shading Models**:
- **Normal**: 3-Channel standard normal map, or 2-Channel BC5 format
- **Emissive**: RGB - Emissive Color, A - Unused
- **Transmission**: RGB - Transmission color, A - Unused

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Material/StandardMaterial.h:72) - Static factory method
- [`StandardMaterial()`](Source/Falcor/Scene/Material/StandardMaterial.h:77) - Constructor with device, name, shading model

**Update**:
- [`renderUI()`](Source/Falcor/Scene/Material/StandardMaterial.h:82) - Render UI elements
- [`getShaderModules()`](Source/Falcor/Scene/Material/StandardMaterial.h:84) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/StandardMaterial.h:85) - Get type conformances

**Shading Model**:
- [`getShadingModel()`](Source/Falcor/Scene/Material/StandardMaterial.h:89) - Get shading model

**MetalRough Parameters**:
- [`setRoughness()`](Source/Falcor/Scene/Material/StandardMaterial.h:94) - Set roughness
- [`getRoughness()`](Source/Falcor/Scene/Material/StandardMaterial.h:99) - Get roughness
- [`setMetallic()`](Source/Falcor/Scene/Material/StandardMaterial.h:104) - Set metallic
- [`getMetallic()`](Source/Falcor/Scene/Material/StandardMaterial.h:109) - Get metallic

**Emissive**:
- [`setEmissiveColor()`](Source/Falcor/Scene/Material/StandardMaterial.h:113) - Set emissive color
- [`setEmissiveFactor()`](Source/Falcor/Scene/Material/StandardMaterial.h:117) - Set emissive factor
- [`getEmissiveColor()`](Source/Falcor/Scene/Material/StandardMaterial.h:121) - Get emissive color
- [`getEmissiveFactor()`](Source/Falcor/Scene/Material/StandardMaterial.h:125) - Get emissive factor

**Light Profile**:
- [`setLightProfileEnabled()`](Source/Falcor/Scene/Material/StandardMaterial.h:129) - Set light profile enabled

**Volume Properties**:
- [`setHasEntryPointVolumeProperties()`](Source/Falcor/Scene/Material/StandardMaterial.h:133) - Set has entry point volume properties
- [`getHasEntryPointVolumeProperties()`](Source/Falcor/Scene/Material/StandardMaterial.h:135) - Get has entry point volume properties

**Differentiable Rendering**:
- [`getDefines()`](Source/Falcor/Scene/Material/StandardMaterial.h:137) - Get shader defines
- [`getParamLayout()`](Source/Falcor/Scene/Material/StandardMaterial.h:139) - Get parameter layout
- [`serializeParams()`](Source/Falcor/Scene/Material/StandardMaterial.h:140) - Serialize parameters
- [`deserializeParams()`](Source/Falcor/Scene/Material/StandardMaterial.h:141) - Deserialize parameters

**Protected Methods**:
- [`updateDeltaSpecularFlag()`](Source/Falcor/Scene/Material/StandardMaterial.h:144) - Update delta specular flag
- [`renderSpecularUI()`](Source/Falcor/Scene/Material/StandardMaterial.h:146) - Render specular UI
- [`setShadingModel()`](Source/Falcor/Scene/Material/StandardMaterial.h:147) - Set shading model

### ClothMaterial

**File**: [`ClothMaterial.h`](Source/Falcor/Scene/Material/ClothMaterial.h:1)

**Purpose**: Cloth material for fabric rendering.

**Key Features**:
- Base color and roughness textures
- Normal map support
- Specialized cloth shading model

**Texture Channel Layout**:
- **BaseColor**: RGB - Base Color, A - Opacity
- **Specular**: R - Unused, G - Roughness, B - Unused, A - Unused
- **Normal**: 3-Channel standard normal map, or 2-Channel BC5 format

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Material/ClothMaterial.h:54) - Static factory method
- [`ClothMaterial()`](Source/Falcor/Scene/Material/ClothMaterial.h:56) - Constructor with device and name

**Shader Integration**:
- [`getShaderModules()`](Source/Falcor/Scene/Material/ClothMaterial.h:58) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/ClothMaterial.h:59) - Get type conformances

**Roughness**:
- [`setRoughness()`](Source/Falcor/Scene/Material/ClothMaterial.h:63) - Set roughness
- [`getRoughness()`](Source/Falcor/Scene/Material/ClothMaterial.h:67) - Get roughness

**Protected Methods**:
- [`renderSpecularUI()`](Source/Falcor/Scene/Material/ClothMaterial.h:70) - Render specular UI

### HairMaterial

**File**: [`HairMaterial.h`](Source/Falcor/Scene/Material/HairMaterial.h:1)

**Purpose**: Hair material for hair rendering.

**Key Features**:
- Absorption coefficient (sigmaA)
- Longitudinal roughness (betaM)
- Azimuthal roughness (betaN)
- Scale offset angle
- Eumelanin and pheomelanin concentration support
- Color conversion utilities

**Texture Channel Layout**:
- **BaseColor**: RGB - Absorption coefficient, sigmaA, A - Opacity
- **Specular**: R - Longitudinal roughness, betaM, G - Azimuthal roughness, betaN, B - Scale offset angle (degrees), A - Unused

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Material/HairMaterial.h:52) - Static factory method
- [`HairMaterial()`](Source/Falcor/Scene/Material/HairMaterial.h:54) - Constructor with device and name

**Shader Integration**:
- [`getShaderModules()`](Source/Falcor/Scene/Material/HairMaterial.h:56) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/HairMaterial.h:57) - Get type conformances

**Color Conversion**:
- [`sigmaAFromConcentration()`](Source/Falcor/Scene/Material/HairMaterial.h:61) - Compute sigmaA from eumelanin and pheomelanin concentration
- [`sigmaAFromColor()`](Source/Falcor/Scene/Material/HairMaterial.h:65) - Compute sigmaA from RGB color
- [`colorFromSigmaA()`](Source/Falcor/Scene/Material/HairMaterial.h:69) - Compute RGB color from sigmaA

### MERLMaterial

**File**: [`MERLMaterial.h`](Source/Falcor/Scene/Material/MERLMaterial.h:1)

**Purpose**: Measured material from MERL BRDF database.

**Key Features**:
- MERL BRDF data loading
- Precomputed albedo lookup table
- GPU buffer management
- Measured BRDF rendering

**Reference**:
- Wojciech Matusik, Hanspeter Pfister, Matt Brand and Leonard McMillan. "A Data-Driven Reflectance Model". ACM Transactions on Graphics, vol. 22(3), 2003, pages 759-769.

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Material/MERLMaterial.h:47) - Static factory method with path
- [`MERLMaterial()`](Source/Falcor/Scene/Material/MERLMaterial.h:49) - Constructor with device, name, path
- [`MERLMaterial()`](Source/Falcor/Scene/Material/MERLMaterial.h:50) - Constructor with MERLFile

**Update**:
- [`renderUI()`](Source/Falcor/Scene/Material/MERLMaterial.h:52) - Render UI elements
- [`update()`](Source/Falcor/Scene/Material/MERLMaterial.h:53) - Update material for rendering
- [`isEqual()`](Source/Falcor/Scene/Material/MERLMaterial.h:54) - Compare material

**Shader Integration**:
- [`getDataBlob()`](Source/Falcor/Scene/Material/MERLMaterial.h:55) - Get material data blob
- [`getShaderModules()`](Source/Falcor/Scene/Material/MERLMaterial.h:56) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/MERLMaterial.h:57) - Get type conformances

**Buffer Management**:
- [`getMaxBufferCount()`](Source/Falcor/Scene/Material/MERLMaterial.h:59) - Get number of buffers (1)

**Protected Methods**:
- [`init()`](Source/Falcor/Scene/Material/MERLMaterial.h:62) - Initialize from MERLFile

**Protected Members**:
- [`mPath`](Source/Falcor/Scene/Material/MERLMaterial.h:64) - Full path to BRDF loaded
- [`mBRDFName`](Source/Falcor/Scene/Material/MERLMaterial.h:65) - BRDF name (file basename without extension)
- [`mData`](Source/Falcor/Scene/Material/MERLMaterial.h:67) - Material parameters
- [`mpBRDFData`](Source/Falcor/Scene/Material/MERLMaterial.h:68) - GPU buffer holding BRDF data
- [`mpAlbedoLUT`](Source/Falcor/Scene/Material/MERLMaterial.h:69) - Precomputed albedo lookup table
- [`mpLUTSampler`](Source/Falcor/Scene/Material/MERLMaterial.h:70) - Sampler for LUT texture

### RGLMaterial

**File**: [`RGLMaterial.h`](Source/Falcor/Scene/Material/RGLMaterial.h:1)

**Purpose**: Measured material from RGL BRDF database.

**Key Features**:
- RGL BRDF data loading
- Precomputed albedo lookup table
- Multiple GPU buffers (theta, phi, sigma, NDF, vNDF, lumi, RGB, etc.)
- Adaptive parameterization support

**Reference**:
- Jonathan Dupuy, Wenzel Jakob "An Adaptive Parameterization for Efficient Material Acquisition and Rendering". Transactions on Graphics (Proc. SIGGRAPH Asia 2018)

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Material/RGLMaterial.h:46) - Static factory method with path
- [`RGLMaterial()`](Source/Falcor/Scene/Material/RGLMaterial.h:48) - Constructor with device, name, path

**Update**:
- [`renderUI()`](Source/Falcor/Scene/Material/RGLMaterial.h:50) - Render UI elements
- [`update()`](Source/Falcor/Scene/Material/RGLMaterial.h:51) - Update material for rendering
- [`isEqual()`](Source/Falcor/Scene/Material/RGLMaterial.h:52) - Compare material

**Shader Integration**:
- [`getDataBlob()`](Source/Falcor/Scene/Material/RGLMaterial.h:53) - Get material data blob
- [`getShaderModules()`](Source/Falcor/Scene/Material/RGLMaterial.h:54) - Get shader modules
- [`getTypeConformances()`](Source/Falcor/Scene/Material/RGLMaterial.h:55) - Get type conformances

**Buffer Management**:
- [`getMaxBufferCount()`](Source/Falcor/Scene/Material/RGLMaterial.h:57) - Get number of buffers (12)

**BRDF Loading**:
- [`loadBRDF()`](Source/Falcor/Scene/Material/RGLMaterial.h:59) - Load BRDF from path

**Protected Methods**:
- [`prepareData()`](Source/Falcor/Scene/Material/RGLMaterial.h:62) - Prepare data
- [`prepareAlbedoLUT()`](Source/Falcor/Scene/Material/RGLMaterial.h:63) - Prepare albedo LUT
- [`computeAlbedoLUT()`](Source/Falcor/Scene/Material/RGLMaterial.h:64) - Compute albedo LUT

**Protected Members**:
- [`mPath`](Source/Falcor/Scene/Material/RGLMaterial.h:66) - Full path to BRDF loaded
- [`mBRDFName`](Source/Falcor/Scene/Material/RGLMaterial.h:67) - BRDF name (file basename without extension)
- [`mBRDFDescription`](Source/Falcor/Scene/Material/RGLMaterial.h:68) - BRDF description
- [`mBRDFUploaded`](Source/Falcor/Scene/Material/RGLMaterial.h:70) - BRDF data uploaded flag
- [`mData`](Source/Falcor/Scene/Material/RGLMaterial.h:71) - Material parameters
- [`mpThetaBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:72) - Theta buffer
- [`mpPhiBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:73) - Phi buffer
- [`mpSigmaBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:74) - Sigma buffer
- [`mpNDFBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:75) - NDF buffer
- [`mpVNDFBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:76) - vNDF buffer
- [`mpLumiBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:77) - Lumi buffer
- [`mpRGBBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:78) - RGB buffer
- [`mpVNDFMarginalBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:79) - vNDF marginal buffer
- [`mpLumiMarginalBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:80) - Lumi marginal buffer
- [`mpVNDFConditionalBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:81) - vNDF conditional buffer
- [`mpLumiConditionalBuf`](Source/Falcor/Scene/Material/RGLMaterial.h:82) - Lumi conditional buffer
- [`mpAlbedoLUT`](Source/Falcor/Scene/Material/RGLMaterial.h:83) - Precomputed albedo lookup table
- [`mpSampler`](Source/Falcor/Scene/Material/RGLMaterial.h:84) - Sampler for BRDF textures
- [`mBRDFTesting`](Source/Falcor/Scene/Material/RGLMaterial.h:86) - BRDF testing compute pass

## Technical Details

### Material Data Packing

**MaterialHeader** (16 bytes):
- Uses uint4 for efficient GPU access
- Bit-packed flags and values
- Float16_t for threshold and IoR to save space
- Alpha texture handle in packedData.w

**MaterialDataBlob** (128 bytes):
- Sized to fit in one cacheline
- Header (16 bytes) + Payload (112 bytes)
- Efficient GPU access pattern

### Update Flags

**Update Detection**:
- Per-material update flags
- Material system tracks global updates
- Callback-based update notification
- Bit flag operations for efficiency

**Update Types**:
- **CodeChanged**: Material shader code changed (requires recompilation)
- **DataChanged**: Material parameters changed (requires GPU upload)
- **ResourcesChanged**: Textures/buffers/samplers changed (requires descriptor updates)
- **DisplacementChanged**: Displacement parameters changed
- **EmissiveChanged**: Emissive properties changed

### Texture Optimization

**Optimization Strategies**:
- Constant texture detection
- Replace constant textures with uniform parameters
- Alpha range analysis
- Normal map type detection
- Disabled alpha optimization
- Constant base color optimization
- Constant normal map optimization

**Texture Analyzer Integration**:
- Per-slot optimization
- Statistics tracking
- Conservative alpha range computation
- Texture channel analysis

### Material Type Registration

**Thread-Safe Registration**:
- [`registerMaterialType()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:43) - Thread-safe type registration
- [`to_string()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:50) - Thread-safe type name lookup
- [`getMaterialTypeCount()`](Source/Falcor/Scene/Material/MaterialTypeRegistry.h:56) - Thread-safe count lookup

**Type Management**:
- Built-in types (MaterialType::BuiltinCount)
- Custom type registration
- Type name mapping
- Parameter layout lookup

### Shader Integration

**Shader Modules**:
- Per-material shader modules
- Material system aggregates all modules
- Type conformances for polymorphism
- Define list for conditional compilation

**Parameter Block Binding**:
- Material system parameter block
- Material data buffer
- Texture descriptor arrays
- Buffer descriptor arrays
- Sampler descriptor arrays

**GPU Resources**:
- Material data buffer (all materials)
- Texture descriptor arrays
- Buffer descriptor arrays
- 3D texture descriptor arrays
- Sampler states

### Material System Architecture

**Resource Management**:
- Texture manager integration
- Centralized buffer management
- Sampler state management
- Descriptor array management

**Update Strategy**:
- Reactive updates (default)
- Dynamic materials (updated every frame)
- Per-material update flags
- Global update tracking

**Optimization**:
- Duplicate material removal
- Material optimization
- Texture optimization
- Efficient descriptor usage

**Statistics**:
- Material type count
- Material count
- Opaque material count
- Material memory usage
- Texture count and memory
- Compressed texture count
- Texel counts

## Integration Points

### Scene Integration

**Scene Builder**:
- Material loading from scene files
- Material system initialization
- Material assignment to geometry

**Scene Cache**:
- Material serialization
- Material deserialization
- Material optimization

### Rendering Integration

**Render Passes**:
- Material system binding
- Material data access
- Shader integration
- Type conformance management

**Path Tracing**:
- Material evaluation at shading points
- BSDF sampling
- Material-specific shading

### Animation Integration

**Material Updates**:
- Dynamic material support
- Per-frame updates
- Change tracking

### UI Integration

**Material Editor**:
- Per-material UI rendering
- Material property editing
- Texture preview
- Material comparison

## Architecture Patterns

### Factory Pattern
- Static [`create()`](Source/Falcor/Scene/Material/StandardMaterial.h:72) methods for material construction
- Type-safe factory methods
- Consistent initialization

### Strategy Pattern
- Different material types with common interface
- Polymorphic material evaluation
- Type-specific optimizations

### Observer Pattern
- Update callbacks for material changes
- Material system tracks material updates
- Reactive update system

### Flyweight Pattern
- Texture manager for shared textures
- Sampler state sharing
- Efficient resource usage

### Registry Pattern
- Material type registration
- Thread-safe type management
- Extensible material system

## Progress Log

- **2026-01-07T20:03:00Z**: Scene Material subsystem analysis completed. Analyzed Material base class, MaterialSystem manager, MaterialData structures, MaterialTypeRegistry, BasicMaterial, StandardMaterial, ClothMaterial, HairMaterial, MERLMaterial, and RGLMaterial. Documented material data packing, update flags, texture optimization, material type registration, shader integration, and material system architecture. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (SDFs, Volume, TriangleMesh, SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs) to complete Scene module analysis.
