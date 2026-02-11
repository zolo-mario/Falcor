# SceneCache - Scene Caching System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneCache** - Helper class for reading and writing scene cache files

### External Dependencies

- **Core/API** - Graphics API (Device)
- **Scene/Scene** - Scene data structures
- **Scene/Animation** - Animation system
- **Scene/Camera** - Camera system
- **Scene/Lights** - Light system (Light, EnvMap)
- **Scene/Volume** - Volume system (Grid, GridVolume)
- **Scene/Material** - Material system (BasicMaterial, MaterialSystem, MaterialTextureLoader)
- **Utils/Crypto** - Cryptographic utilities (SHA1)

## Module Overview

The SceneCache system provides comprehensive scene caching for heavily reducing load times of more complex assets. The cache stores a binary representation of Scene::SceneData which contains everything to re-create a Scene, including cameras, lights, volumes, materials, and animations.

## Component Specifications

### SceneCache

**File**: [`SceneCache.h`](Source/Falcor/Scene/SceneCache.h:1)

**Purpose**: Helper class for reading and writing scene cache files.

**Key Features**:
- Binary scene cache format
- SHA1-based cache key generation
- Scene data serialization and deserialization
- Camera caching
- Light caching
- Material system caching
- Volume caching
- Environment map caching
- Transform caching
- Animation caching
- Marker caching
- Split buffer support for large data

**Cache Key** (from [`SceneCache::Key`](Source/Falcor/Scene/SceneCache.h:60)):
- **Type**: SHA1::MD
- **Purpose**: Unique identifier for scene cache
- **Usage**: Cache file naming and lookup

**Core Methods**:

**Cache Validation**:
- [`hasValidCache()`](Source/Falcor/Scene/SceneCache.h:66) - Check if valid cache exists for given cache key
  - Parameters: key (SHA1::MD)
  - Returns: true if valid cache exists

**Cache Writing**:
- [`writeCache()`](Source/Falcor/Scene/SceneCache.h:72) - Write scene cache
  - Parameters: sceneData (Scene::SceneData), key (SHA1::MD)
  - Purpose: Serialize scene data to cache file

**Cache Reading**:
- [`readCache()`](Source/Falcor/Scene/SceneCache.h:79) - Read scene cache
  - Parameters: pDevice (GPU device), key (SHA1::MD)
  - Returns: Loaded scene data (Scene::SceneData)

**Path Management**:
- [`getCachePath()`](Source/Falcor/Scene/SceneCache.h:85) - Get cache path for given cache key
  - Parameters: key (SHA1::MD)
  - Returns: Filesystem path to cache file

**Data Writing**:
- [`writeSceneData()`](Source/Falcor/Scene/SceneCache.h:87) - Write scene data to stream
- [`writeMetadata()`](Source/Falcor/Scene/SceneCache.h:90) - Write metadata to stream
- [`writeCamera()`](Source/Falcor/Scene/SceneCache.h:93) - Write camera to stream
- [`writeLight()`](Source/Falcor/Scene/SceneCache.h:96) - Write light to stream
- [`writeMaterials()`](Source/Falcor/Scene/SceneCache.h:99) - Write material system to stream
- [`writeMaterial()`](Source/Falcor/Scene/SceneCache.h:101) - Write material to stream
- [`writeBasicMaterial()`](Source/Falcor/Scene/SceneCache.h:101) - Write basic material to stream
- [`writeSampler()`](Source/Falcor/Scene/SceneCache.h:106) - Write sampler to stream
- [`writeGridVolume()`](Source/Falcor/Scene/SceneCache.h:109) - Write grid volume to stream
- [`writeGrid()`](Source/Falcor/Scene/SceneCache.h:112) - Write grid to stream
- [`writeEnvMap()`](Source/Falcor/Scene/SceneCache.h:115) - Write environment map to stream
- [`writeTransform()`](Source/Falcor/Scene/SceneCache.h:118) - Write transform to stream
- [`writeAnimation()`](Source/Falcor/Scene/SceneCache.h:121) - Write animation to stream
- [`writeMarker()`](Source/Falcor/Scene/SceneCache.h:124) - Write marker to stream
- [`writeSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:128) - Write split buffer to stream

**Data Reading**:
- [`readSceneData()`](Source/Falcor/Scene/SceneCache.h:88) - Read scene data from stream
- [`readMetadata()`](Source/Falcor/Scene/SceneCache.h:91) - Read metadata from stream
- [`readCamera()`](Source/Falcor/Scene/SceneCache.h:94) - Read camera from stream
- [`readLight()`](Source/Falcor/Scene/SceneCache.h:97) - Read light from stream
- [`readMaterials()`](Source/Falcor/Scene/SceneCache.h:102) - Read materials from stream
- [`readMaterial()`](Source/Falcor/Scene/SceneCache.h:103) - Read material from stream
- [`readBasicMaterial()`](Source/Falcor/Scene/SceneCache.h:104) - Read basic material from stream
- [`readSampler()`](Source/Falcor/Scene/SceneCache.h:107) - Read sampler from stream
- [`readGridVolume()`](Source/Falcor/Scene/SceneCache.h:110) - Read grid volume from stream
- [`readGrid()`](Source/Falcor/Scene/SceneCache.h:113) - Read grid from stream
- [`readEnvMap()`](Source/Falcor/Scene/SceneCache.h:116) - Read environment map from stream
- [`readTransform()`](Source/Falcor/Scene/SceneCache.h:119) - Read transform from stream
- [`readAnimation()`](Source/Falcor/Scene/SceneCache.h:122) - Read animation from stream
- [`readMarker()`](Source/Falcor/Scene/SceneCache.h:125) - Read marker from stream
- [`readSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:130) - Read split buffer from stream

**Split Buffer Support**:
- Template: `<typename T, bool TUseByteAddressBuffer>`
- [`writeSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:128) - Write split buffer to stream
- [`readSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:130) - Read split buffer from stream

**Internal Classes**:
- **OutputStream** - Output stream for writing cache files
- **InputStream** - Input stream for reading cache files

## Technical Details

### Cache Key Generation

**SHA1 Hashing**:
- SHA1::MD hash algorithm
- Generates unique cache key
- Used for cache file naming
- Used for cache lookup

**Cache File Naming**:
- Based on SHA1 hash
- Unique per scene
- Enables cache validation
- Enables cache lookup

### Binary Cache Format

**Scene Data Structure**:
- Contains everything to re-create a Scene
- Binary serialization for efficiency
- Compact representation
- Fast loading and saving

**Cached Components**:
- **Scene Data**: Complete scene representation
- **Metadata**: Scene metadata
- **Cameras**: Camera configurations
- **Lights**: Light definitions
- **Materials**: Material system and materials
- **Samplers**: Texture samplers
- **Volumes**: Grid volumes and grids
- **Environment Maps**: Environment map data
- **Transforms**: Transform data
- **Animations**: Animation data
- **Markers**: Scene markers

### Serialization Strategy

**Stream-Based Writing**:
- Sequential writing to output stream
- Type-safe serialization
- Efficient binary format
- Supports large data via split buffers

**Stream-Based Reading**:
- Sequential reading from input stream
- Type-safe deserialization
- Efficient binary format parsing
- GPU device integration for resource creation

### Material System Caching

**Material System**:
- [`writeMaterials()`](Source/Falcor/Scene/SceneCache.h:99) - Write entire material system
- [`readMaterials()`](Source/Falcor/Scene/SceneCache.h:102) - Read entire material system
- Parameters: MaterialSystem, MaterialTextureLoader, Device

**Individual Materials**:
- [`writeMaterial()`](Source/Falcor/Scene/SceneCache.h:101) - Write single material
- [`readMaterial()`](Source/Falcor/Scene/SceneCache.h:103) - Read single material
- Parameters: MaterialTextureLoader, Device

**Basic Materials**:
- [`writeBasicMaterial()`](Source/Falcor/Scene/SceneCache.h:101) - Write basic material
- [`readBasicMaterial()`](Source/Falcor/Scene/SceneCache.h:104) - Read basic material
- Parameters: MaterialTextureLoader, BasicMaterial, Device

### Volume Caching

**Grid Volumes**:
- [`writeGridVolume()`](Source/Falcor/Scene/SceneCache.h:109) - Write grid volume
- [`readGridVolume()`](Source/Falcor/Scene/SceneCache.h:110) - Read grid volume
- Parameters: GridVolume, vector of Grids, Device
- Caches density and emission grids
- Caches volume properties

**Grids**:
- [`writeGrid()`](Source/Falcor/Scene/SceneCache.h:112) - Write grid
- [`readGrid()`](Source/Falcor/Scene/SceneCache.h:113) - Read grid
- Parameters: Device
- Caches NanoVDB grid data
- Caches grid transform

### Split Buffer Support

**Purpose**:
- Handle large data that exceeds buffer limits
- Split data into multiple buffers
- Byte address buffer support option

**Template Parameters**:
- **T**: Data type to store in buffer
- **TUseByteAddressBuffer**: Whether to use byte address buffer

**Methods**:
- [`writeSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:128) - Write split buffer
- [`readSplitBuffer()`](Source/Falcor/Scene/SceneCache.h:130) - Read split buffer

### Cache Validation

**hasValidCache()**:
- Checks if cache file exists
- Uses cache key for lookup
- Returns true if valid cache exists
- Enables cache hit detection

**Cache Path Generation**:
- [`getCachePath()`](Source/Falcor/Scene/SceneCache.h:85) - Generate cache file path
- Based on cache key (SHA1 hash)
- Consistent path naming
- Enables cache file lookup

### Performance Benefits

**Load Time Reduction**:
- Binary format is faster than parsing scene files
- Pre-processed scene data
- Skip expensive import steps
- Direct GPU resource creation

**Memory Efficiency**:
- Compact binary representation
- Efficient serialization
- Split buffer support for large data
- Optimized for cache storage

## Integration Points

### Scene Integration

**Scene Builder**:
- Cache writing after scene construction
- Cache reading before scene loading
- Automatic cache hit detection

**Scene Data**:
- Complete scene data serialization
- All scene components cached
- Consistent with Scene::SceneData structure

### GPU Integration

**Device Parameter**:
- GPU device passed to read methods
- Enables direct GPU resource creation
- Texture and buffer allocation during cache read

### Material System Integration

**MaterialTextureLoader**:
- Passed to material reading methods
- Enables texture loading during cache read
- Consistent with scene builder material loading

### Volume System Integration

**Grid and GridVolume**:
- Volume data serialization
- Grid sequence caching
- Volume properties preservation

### Animation Integration

**Animation Caching**:
- Animation data serialization
- Animation playback state preservation
- Consistent with scene animation system

## Architecture Patterns

### Serialization Pattern
- Binary serialization of scene data
- Stream-based writing and reading
- Type-safe serialization/deserialization

### Cache Pattern
- SHA1-based cache key generation
- Cache file naming and lookup
- Cache validation and hit detection

### Template Pattern
- Split buffer template for large data
- Type-safe buffer handling
- Byte address buffer support option

### Factory Pattern
- Static methods for cache operations
- Consistent interface for cache read/write
- Device integration for GPU resource creation

## Progress Log

- **2026-01-07T20:11:00Z**: Scene SceneCache subsystem analysis completed. Analyzed SceneCache helper class for reading and writing scene cache files. Documented SHA1-based cache key generation, binary cache format, serialization strategy, material system caching, volume caching, split buffer support, and performance benefits. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (Transform, VertexAttrib, SceneTypes, SceneIDs) to complete Scene module analysis.
