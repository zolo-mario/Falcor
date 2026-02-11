# SceneCache - Scene Cache

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SceneCache.h** - Scene cache header (132 lines)
- [x] **SceneCache.cpp** - Scene cache implementation

### External Dependencies

- **Scene/Scene.h** - Scene main class
- **Scene/Animation/Animation.h** - Animation system
- **Scene/Camera/Camera.h** - Camera system
- **Scene/Lights/EnvMap.h** - Environment map
- **Scene/Lights/Light.h** - Light
- **Scene/Volume/Grid.h** - Grid
- **Scene/Volume/GridVolume.h** - Grid volume
- **Scene/Material/BasicMaterial.h** - Basic material
- **Scene/Material/MaterialSystem.h** - Material system
- **Scene/Material/MaterialTextureLoader.h** - Material texture loader
- **Core/Macros** - Macros
- **Core/API/fwd.h** - Graphics API forward declarations
- **Utils/CryptoUtils.h** - Cryptographic utilities
- **filesystem** - Filesystem utilities
- **string** - String utilities
- **vector** - Vector utilities

## Module Overview

SceneCache is a comprehensive scene caching system designed to heavily reduce load times of more complex assets. The cache stores a binary representation of [`Scene::SceneData`](Source/Falcor/Scene/SceneCache.h:55) which contains everything to re-create a [`Scene`](Source/Falcor/Scene/SceneCache.h:55). It provides helper class for reading and writing scene cache files using split buffers for efficient memory management. The cache uses SHA1 hashes as keys for cache identification and validation. SceneCache integrates with the material system, texture loader, and device to provide complete scene caching functionality including metadata, cameras, lights, materials, volumes, grids, environment maps, transforms, animations, and split buffer data.

## Component Specifications

### Key Type

**Purpose**: Cache key type using SHA1 hash.

**Definition**:
- [`Key = SHA1::MD`](Source/Falcor/Scene/SceneCache.h:60) - SHA1 MD hash used as cache key

**Usage**:
- Unique identifier for cached scenes
- Based on scene content
- Used for cache validation
- Used for cache lookup

### SceneCache Class

**Purpose**: Helper class for reading and writing scene cache files. The scene cache is used to heavily reduce load times of more complex assets. The cache stores a binary representation of `Scene::SceneData` which contains everything to re-create a `Scene`.

**Public Methods**:

**Cache Validation**:
- [`hasValidCache(const Key& key)`](Source/Falcor/Scene/SceneCache.h:66) - Check if there is a valid scene cache for a given cache key
  - Parameters: key - Cache key
  - Returns: true if a valid cache exists

**Cache Writing**:
- [`writeCache(const Scene::SceneData& sceneData, const Key& key)`](Source/Falcor/Scene/SceneCache.h:72) - Write a scene cache
  - Parameters: sceneData - Scene data, key - Cache key
  - Writes scene data to cache file

**Cache Reading**:
- [`readCache(ref<Device> pDevice, const Key& key)`](Source/Falcor/Scene/SceneCache.h:79) - Read a scene cache
  - Parameters: pDevice - GPU device, key - Cache key
  - Returns: Loaded scene data

### Private Types

**OutputStream Class**:
- Private class for output stream operations
- Handles binary writing
- Manages file output

**InputStream Class**:
- Private class for input stream operations
- Handles binary reading
- Manages file input

### Private Methods

**Cache Path**:
- [`getCachePath(const Key& key)`](Source/Falcor/Scene/SceneCache.h:85) - Get cache file path from cache key
  - Parameters: key - Cache key
  - Returns: Filesystem path to cache file

**Scene Data Writing**:
- [`writeSceneData(OutputStream& stream, const Scene::SceneData& sceneData)`](Source/Falcor/Scene/SceneCache.h:87) - Write scene data to output stream
  - Parameters: stream - Output stream, sceneData - Scene data
  - Writes complete scene data structure

**Scene Data Reading**:
- [`readSceneData(InputStream& stream, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:88) - Read scene data from input stream
  - Parameters: stream - Input stream, pDevice - GPU device
  - Returns: Scene data structure

**Metadata Writing**:
- [`writeMetadata(OutputStream& stream, const Scene::Metadata& metadata)`](Source/Falcor/Scene/SceneCache.h:90) - Write metadata to output stream
  - Parameters: stream - Output stream, metadata - Scene metadata
  - Writes metadata structure

**Metadata Reading**:
- [`readMetadata(InputStream& stream)`](Source/Falcor/Scene/SceneCache.h:91) - Read metadata from input stream
  - Parameters: stream - Input stream
  - Returns: Metadata structure

**Camera Writing**:
- [`writeCamera(OutputStream& stream, const ref<Camera>& pCamera)`](Source/Falcor/Scene/SceneCache.h:93) - Write camera to output stream
  - Parameters: stream - Output stream, pCamera - Camera reference
  - Writes camera data

**Camera Reading**:
- [`readCamera(InputStream& stream)`](Source/Falcor/Scene/SceneCache.h:94) - Read camera from input stream
  - Parameters: stream - Input stream
  - Returns: Camera reference

**Light Writing**:
- [`writeLight(OutputStream& stream, const ref<Light>& pLight)`](Source/Falcor/Scene/SceneCache.h:96) - Write light to output stream
  - Parameters: stream - Output stream, pLight - Light reference
  - Writes light data

**Light Reading**:
- [`readLight(InputStream& stream)`](Source/Falcor/Scene/SceneCache.h:97) - Read light from input stream
  - Parameters: stream - Input stream
  - Returns: Light reference

**Materials Writing**:
- [`writeMaterials(OutputStream& stream, const MaterialSystem& materialSystem)`](Source/Falcor/Scene/SceneCache.h:99) - Write materials to output stream
  - Parameters: stream - Output stream, materialSystem - Material system
  - Writes complete material system

**Material Writing**:
- [`writeMaterial(OutputStream& stream, const ref<Material>& pMaterial)`](Source/Falcor/Scene/SceneCache.h:100) - Write material to output stream
  - Parameters: stream - Output stream, pMaterial - Material reference
  - Writes single material

**Materials Reading**:
- [`readMaterials(InputStream& stream, MaterialSystem& materialSystem, MaterialTextureLoader& materialTextureLoader, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:102) - Read materials from input stream
  - Parameters: stream - Input stream, materialSystem - Material system, materialTextureLoader - Material texture loader, pDevice - GPU device
  - Returns: Populates material system

**Material Reading**:
- [`readMaterial(InputStream& stream, MaterialTextureLoader& materialTextureLoader, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:103) - Read material from input stream
  - Parameters: stream - Input stream, materialTextureLoader - Material texture loader, pDevice - GPU device
  - Returns: Material reference

**Basic Material Reading**:
- [`readBasicMaterial(InputStream& stream, MaterialTextureLoader& materialTextureLoader, const ref<BasicMaterial>& pMaterial, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:104) - Read basic material from input stream
  - Parameters: stream - Input stream, materialTextureLoader - Material texture loader, pMaterial - Basic material reference, pDevice - GPU device
  - Returns: Populates basic material

**Sampler Writing**:
- [`writeSampler(OutputStream& stream, const ref<Sampler>& pSampler)`](Source/Falcor/Scene/SceneCache.h:106) - Write sampler to output stream
  - Parameters: stream - Output stream, pSampler - Sampler reference
  - Writes sampler data

**Sampler Reading**:
- [`readSampler(InputStream& stream, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:107) - Read sampler from input stream
  - Parameters: stream - Input stream, pDevice - GPU device
  - Returns: Sampler reference

**Grid Volume Writing**:
- [`writeGridVolume(OutputStream& stream, const ref<GridVolume>& pVolume, const std::vector<ref<Grid>>& grids)`](Source/Falcor/Scene/SceneCache.h:109) - Write grid volume to output stream
  - Parameters: stream - Output stream, pVolume - Grid volume reference, grids - Grid references
  - Writes grid volume data

**Grid Volume Reading**:
- [`readGridVolume(InputStream& stream, const std::vector<ref<Grid>>& grids, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:110) - Read grid volume from input stream
  - Parameters: stream - Input stream, grids - Grid references, pDevice - GPU device
  - Returns: Grid volume reference

**Grid Writing**:
- [`writeGrid(OutputStream& stream, const ref<Grid>& pGrid)`](Source/Falcor/Scene/SceneCache.h:112) - Write grid to output stream
  - Parameters: stream - Output stream, pGrid - Grid reference
  - Writes grid data

**Grid Reading**:
- [`readGrid(InputStream& stream, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:113) - Read grid from input stream
  - Parameters: stream - Input stream, pDevice - GPU device
  - Returns: Grid reference

**Environment Map Writing**:
- [`writeEnvMap(OutputStream& stream, const ref<EnvMap>& pEnvMap)`](Source/Falcor/Scene/SceneCache.h:115) - Write environment map to output stream
  - Parameters: stream - Output stream, pEnvMap - Environment map reference
  - Writes environment map data

**Environment Map Reading**:
- [`readEnvMap(InputStream& stream, ref<Device> pDevice)`](Source/Falcor/Scene/SceneCache.h:116) - Read environment map from input stream
  - Parameters: stream - Input stream, pDevice - GPU device
  - Returns: Environment map reference

**Transform Writing**:
- [`writeTransform(OutputStream& stream, const Transform& transform)`](Source/Falcor/Scene/SceneCache.h:118) - Write transform to output stream
  - Parameters: stream - Output stream, transform - Transform
  - Writes transform data

**Transform Reading**:
- [`readTransform(InputStream& stream)`](Source/Falcor/Scene/SceneCache.h:119) - Read transform from input stream
  - Parameters: stream - Input stream
  - Returns: Transform structure

**Animation Writing**:
- [`writeAnimation(OutputStream& stream, const ref<Animation>& pAnimation)`](Source/Falcor/Scene/SceneCache.h:121) - Write animation to output stream
  - Parameters: stream - Output stream, pAnimation - Animation reference
  - Writes animation data

**Animation Reading**:
- [`readAnimation(InputStream& stream)`](Source/Falcor/Scene/SceneCache.h:122) - Read animation from input stream
  - Parameters: stream - Input stream
  - Returns: Animation reference

**Marker Writing**:
- [`writeMarker(OutputStream& stream, const std::string& id)`](Source/Falcor/Scene/SceneCache.h:124) - Write marker to output stream
  - Parameters: stream - Output stream, id - Marker ID
  - Writes marker data

**Marker Reading**:
- [`readMarker(InputStream& stream, const std::string& id)`](Source/Falcor/Scene/SceneCache.h:125) - Read marker from input stream
  - Parameters: stream - Input stream, id - Marker ID
  - Returns: Marker data

**Split Buffer Writing**:
- [`writeSplitBuffer<T, TUseByteAddressBuffer>(OutputStream& stream, const SplitBuffer<T, TUseByteAddressBuffer>& buffer)`](Source/Falcor/Scene/SceneCache.h:128) - Write split buffer to output stream
  - Template parameters: stream - Output stream, buffer - Split buffer
  - Writes split buffer data

**Split Buffer Reading**:
- [`readSplitBuffer<T, TUseByteAddressBuffer>(InputStream& stream, SplitBuffer<T, TUseByteAddressBuffer>& buffer)`](Source/Falcor/Scene/SceneCache.h:130) - Read split buffer from input stream
  - Template parameters: stream - Input stream, buffer - Split buffer
  - Returns: Populates split buffer

## Technical Details

### Cache Key Generation

**SHA1 Hashing**:
- Uses SHA1::MD for cache keys
- Provides unique identifier for scene content
- Enables cache validation
- Supports cache lookup

**Cache Key Components**:
- Scene data hash
- Build flags
- Settings
- File paths
- Import dictionaries

### Cache File Format

**Binary Format**:
- Binary representation of Scene::SceneData
- Efficient serialization
- Supports all scene data types

**Cache File Structure**:
- Scene metadata
- Cameras
- Lights
- Materials
- Volumes
- Grids
- Environment maps
- Transforms
- Animations
- Split buffers

### Stream Operations

**Output Stream**:
- Binary writing
- File-based output
- Efficient serialization

**Input Stream**:
- Binary reading
- File-based input
- Efficient deserialization

### Split Buffer Support

**Template Implementation**:
- `SplitBuffer<T, TUseByteAddressBuffer>` - Generic split buffer
- Supports both byte address buffers and regular buffers
- Used for efficient memory management

**Split Buffer Operations**:
- Write to stream
- Read from stream
- Supports all data types

## Integration Points

### Scene Integration

**Scene Data Structure**:
- [`Scene::SceneData`](Source/Falcor/Scene/SceneCache.h:55) - Complete scene data
- Contains all scene elements
- Used for scene reconstruction

**Scene Reconstruction**:
- Rebuilds Scene from cache
- Preserves all scene data
- Maintains scene structure

### Material System Integration

**Material System**:
- [`MaterialSystem`](Source/Falcor/Scene/SceneCache.h:99) - Material system reference
- Complete material serialization
- Texture loader integration

**Material Texture Loader**:
- [`MaterialTextureLoader`](Source/Falcor/Scene/SceneCache.h:102) - Texture loader reference
- Loads textures during cache read
- Supports async texture loading

### Device Integration

**GPU Device**:
- [`ref<Device>`](Source/Falcor/Scene/SceneCache.h:79) - GPU device reference
- Required for resource creation
- Used for texture loading

### Camera Integration

**Camera System**:
- [`Camera`](Source/Falcor/Scene/SceneCache.h:93) - Camera reference
- Complete camera serialization
- Supports camera properties

### Light Integration

**Light System**:
- [`Light`](Source/Falcor/Scene/SceneCache.h:96) - Light reference
- Complete light serialization
- Supports all light types

### Volume Integration

**Grid Volume System**:
- [`GridVolume`](Source/Falcor/Scene/SceneCache.h:109) - Grid volume reference
- Complete grid volume serialization
- Grid references integration

**Grid System**:
- [`Grid`](Source/Falcor/Scene/SceneCache.h:112) - Grid reference
- Complete grid serialization
- Volume data support

### Environment Map Integration

**Environment Map System**:
- [`EnvMap`](Source/Falcor/Scene/SceneCache.h:115) - Environment map reference
- Complete environment map serialization
- Supports environment map properties

### Transform Integration

**Transform System**:
- [`Transform`](Source/Falcor/Scene/SceneCache.h:118) - Transform structure
- Complete transform serialization
- Supports transform matrices

### Animation Integration

**Animation System**:
- [`Animation`](Source/Falcor/Scene/SceneCache.h:121) - Animation reference
- Complete animation serialization
- Supports animation properties

### Split Buffer Integration

**Split Buffer System**:
- [`SplitBuffer`](Source/Falcor/Scene/SceneCache.h:50) - Split buffer template
- Efficient memory management
- Supports byte address buffers

## Architecture Patterns

### Caching Pattern

- Binary serialization
- Disk-based cache
- SHA1 hash keys
- Cache validation
- Cache lookup

### Serialization Pattern

- Binary format
- Efficient I/O
- Complete scene data
- Type-specific serialization

### Stream Pattern

- Input/output streams
- File-based operations
- Binary reading/writing
- Template-based split buffers

### Template Pattern

- Generic split buffer support
- Type-safe operations
- Byte address buffer support
- Efficient memory management

## Code Patterns

### Cache Validation Pattern

```cpp
static bool hasValidCache(const Key& key)
{
    // Check if cache file exists
    // Validate cache key
    // Return true if valid cache exists
}
```

### Cache Writing Pattern

```cpp
static void writeCache(const Scene::SceneData& sceneData, const Key& key)
{
    // Generate cache path
    // Open output stream
    // Write scene data
    // Write metadata
    // Write cameras, lights, materials, volumes, grids, env maps, transforms, animations
    // Close stream
}
```

### Cache Reading Pattern

```cpp
static Scene::SceneData readCache(ref<Device> pDevice, const Key& key)
{
    // Generate cache path
    // Open input stream
    // Read scene data
    // Read metadata
    // Read cameras, lights, materials, volumes, grids, env maps, transforms, animations
    // Close stream
    // Return scene data
}
```

### Split Buffer Pattern

```cpp
template<typename T, bool TUseByteAddressBuffer>
static void writeSplitBuffer(OutputStream& stream, const SplitBuffer<T, TUseByteAddressBuffer>& buffer)
{
    // Write split buffer data
    // Handle byte address buffers
    // Write buffer metadata
}
```

## Use Cases

### Scene Caching

- **Cache Writing**:
  - Write scene to cache
  - Use SHA1 hash for cache key
  - Reduce load time for complex assets
  - Cache all scene data

- **Cache Reading**:
  - Read scene from cache
  - Validate cache key
  - Reconstruct complete scene
  - Bypass expensive import process

### Cache Validation

- **Cache Key Validation**:
  - Check if cache exists
  - Validate cache key
  - Determine cache validity

- **Cache Integrity**:
  - Validate cache file
  - Check cache version
  - Verify cache compatibility

### Performance Optimization

- **Load Time Reduction**:
  - Cache complex assets
  - Reduce import time
  - Improve startup performance

- **Memory Efficiency**:
  - Binary serialization
  - Efficient I/O operations
  - Split buffer optimization

## Performance Considerations

### Cache Performance

**Load Time Reduction**:
- O(1) for cache lookup
- O(n) for cache reading
- Significantly reduces load time for complex assets

**Cache Size**:
- Binary format minimizes size
- Complete scene data
- All scene elements cached

**Cache Validation**:
- O(1) for cache existence check
- O(1) for cache key validation
- Fast cache validation

### I/O Performance

**Binary Serialization**:
- Efficient binary format
- Minimal overhead
- Fast read/write operations

**Split Buffer Performance**:
- Template-based implementation
- Type-safe operations
- Efficient memory management

## Limitations

### Feature Limitations

- **Cache Key Collision**:
- SHA1 hash collisions possible (unlikely)
- No collision resolution mechanism
- Rare but possible

- **Cache Validation**:
- Limited to existence check
- No content validation
- No version compatibility check

- **Cache Size**:
- No cache size limit
- May consume large disk space
- No automatic cache cleanup

### Performance Limitations

- **I/O Bound**:
- Disk I/O bound
- Limited by disk speed
- May be slow for large scenes

- **Memory Usage**:
- Loads entire scene into memory
- May require significant memory
- No streaming support

### Integration Limitations

- **Scene Coupling**:
- Tightly coupled to Scene::SceneData
- Requires specific data structures
- Not suitable for other scene systems

- **Material System Coupling**:
- Tightly coupled to MaterialSystem
- Requires MaterialTextureLoader
- Not suitable for other material systems

## Best Practices

### Cache Usage

- **Cache Writing**:
  - Use caching for repeated loads
  - Write cache after successful import
  - Use appropriate cache keys
  - Enable cache rebuild when needed

- **Cache Reading**:
  - Check cache validity before reading
  - Validate cache key
  - Handle cache misses gracefully

### Cache Management

- **Cache Cleanup**:
  - Implement cache cleanup
  - Remove old cache files
  - Manage cache size

- **Cache Validation**:
  - Validate cache before use
  - Check cache integrity
  - Handle cache corruption

### Performance Optimization

- **Binary Format**:
  - Use binary serialization for efficiency
  - Minimize cache size
  - Optimize I/O operations

- **Split Buffers**:
  - Use split buffers for large data
  - Enable byte address buffers
  - Optimize memory layout

### Error Handling

- **Cache Misses**:
  - Handle cache misses gracefully
  - Fall back to import
  - Log cache miss events

- **Cache Corruption**:
  - Detect cache corruption
  - Rebuild cache when needed
  - Log corruption events

## Progress Log

- **2026-01-08T00:38:00Z**: SceneCache analysis completed. Analyzed SceneCache.h (132 lines) containing comprehensive scene caching system. Documented SceneCache class with Key type (SHA1::MD), public methods (hasValidCache, writeCache, readCache), private classes (OutputStream, InputStream), private methods (getCachePath, writeSceneData, readSceneData, writeMetadata, readMetadata, writeCamera, readCamera, writeLight, readLight, writeMaterials, writeMaterial, readMaterials, readMaterial, readBasicMaterial, writeSampler, readSampler, writeGridVolume, readGridVolume, writeGrid, readGrid, writeEnvMap, readEnvMap, writeTransform, readTransform, writeAnimation, readAnimation, writeMarker, readMarker, writeSplitBuffer, readSplitBuffer). Documented comprehensive scene caching system with binary serialization of Scene::SceneData containing all scene elements (metadata, cameras, lights, materials, volumes, grids, environment maps, transforms, animations, split buffers), SHA1 hash-based cache keys, cache validation, cache file path generation, and split buffer template support for efficient memory management. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The SceneCache module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
