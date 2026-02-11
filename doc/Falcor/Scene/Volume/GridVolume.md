# GridVolume - Grid Volume System

## Module State Machine

**Status**: Complete

## Dependency Graph

### External Dependencies

- Falcor/Scene/Volume/Grid.h (Grid class for voxel data)
- Falcor/Scene/Volume/GridVolumeData.slang (GridVolumeData structure)
- Falcor/Core/Macros.h (FALCOR_OBJECT, FALCOR_API macros)
- Falcor/Utils/Math/AABB.h (AABB for bounds)
- Falcor/Utils/Math/Matrix.h (float4x4 for transforms)
- Falcor/Utils/UI/Gui.h (UI rendering)
- Falcor/Scene/Animation/Animatable.h (Animatable base class)
- Falcor/Core/API/Device.h (Device for GPU resources)
- Falcor/Utils/Logger.h (Logging)
- Falcor/Utils/Scripting/ScriptBindings.h (Python bindings)
- STL (array, filesystem, memory, string, vector, algorithm, set)

## Module Overview

GridVolume is a comprehensive volumetric rendering system that manages heterogeneous participating media with density and emission grids. It supports both static and dynamic grid sequences, animation through Animatable interface, and provides extensive rendering properties including albedo, anisotropy, and emission modes (Direct and Blackbody).

## Component Specifications

### GridVolume Class (C++)

**Signature**: `class GridVolume : public Animatable`

**Purpose**: CPU-side grid volume management with animation support

**Type Aliases**:
- `GridSequence`: `std::vector<ref<Grid>>` (sequence of grids for dynamic volumes)

**Enums**:
- `UpdateFlags`: Flags indicating what was updated (None, PropertiesChanged, GridsChanged, TransformChanged, BoundsChanged)
- `GridSlot`: Grid slots available in volume (Density, Emission, Count)
- `EmissionMode`: Specifies how emission is rendered (Direct, Blackbody)

**Factory Methods**:
- `create(ref<Device> pDevice, const std::string& name)`: Create new GridVolume

**Constructor**:
- `GridVolume(ref<Device> pDevice, const std::string& name)`: Initialize volume

**Rendering Methods**:
- `renderUI(Gui::Widgets& widget)`: Render UI for volume properties
- `getUpdates()`: Get updates since last call to clearUpdates
- `clearUpdates()`: Clear update flags

**Naming Methods**:
- `setName(const std::string& name)`: Set volume name
- `getName()`: Get volume name

**Grid Loading Methods**:
- `loadGrid(GridSlot slot, const std::filesystem::path& path, const std::string& gridname)`: Load single grid from file
- `createGridSequence(ref<Device> pDevice, const std::vector<std::filesystem::path>& paths, const std::string& gridname, bool keepEmpty)`: Create grid sequence from files
- `loadGridSequence(GridSlot slot, const std::vector<std::filesystem::path>& paths, const std::string& gridname, bool keepEmpty)`: Load grid sequence from files
- `loadGridSequence(GridSlot slot, const std::filesystem::path& path, const std::string& gridname, bool keepEmpty)`: Load grid sequence from directory

**Grid Management Methods**:
- `setGridSequence(GridSlot slot, const GridSequence& grids)`: Set grid sequence for slot
- `getGridSequence(GridSlot slot)`: Get grid sequence for slot
- `setGrid(GridSlot slot, const ref<Grid>& grid)`: Set single grid for slot
- `getGrid(GridSlot slot)`: Get current grid from slot
- `getAllGrids()`: Get all grids used by volume

**Playback Methods**:
- `setGridFrame(uint32_t gridFrame)`: Set current frame of grid sequence
- `getGridFrame()`: Get current frame of grid sequence
- `getGridFrameCount()`: Get number of frames in grid sequence
- `setFrameRate(double frameRate)`: Set frame rate for grid playback
- `getFrameRate()`: Get frame rate for grid playback
- `setStartFrame(uint32_t frame)`: Set grid playback start frame
- `getStartFrame()`: Get grid playback start frame
- `setPlaybackEnabled(bool enabled)`: Enable/disable grid playback
- `isPlaybackEnabled()`: Check if grid playback is enabled
- `updatePlayback(double currentTime)`: Update selected grid frame based on global time

**Property Methods**:
- `setDensityGrid(const ref<Grid>& densityGrid)`: Set density grid
- `getDensityGrid()`: Get density grid
- `setDensityScale(float densityScale)`: Set density scale factor
- `getDensityScale()`: Get density scale factor
- `setEmissionGrid(const ref<Grid>& emissionGrid)`: Set emission grid
- `getEmissionGrid()`: Get emission grid
- `setEmissionScale(float emissionScale)`: Set emission scale factor
- `getEmissionScale()`: Get emission scale factor
- `setAlbedo(const float3& albedo)`: Set scattering albedo
- `getAlbedo()`: Get scattering albedo
- `setAnisotropy(float anisotropy)`: Set phase function anisotropy
- `getAnisotropy()`: Get phase function anisotropy
- `setEmissionMode(EmissionMode emissionMode)`: Set emission mode
- `getEmissionMode()`: Get emission mode
- `setEmissionTemperature(float emissionTemperature)`: Set emission base temperature (K)
- `getEmissionTemperature()`: Get emission base temperature (K)

**Data Access Methods**:
- `getData()`: Get grid volume data struct
- `getBounds()`: Get volume bounds in world space

**Animation Methods**:
- `updateFromAnimation(const float4x4& transform)`: Update transform from animation

**Private Methods**:
- `updateSequence()`: Update sequence metadata
- `updateBounds()`: Update volume bounds
- `markUpdates(UpdateFlags updates)`: Mark updates
- `setFlags(uint32_t flags)`: Set flags

**Member Variables**:
- `ref<Device> mpDevice`: GPU device
- `std::string mName`: Volume name
- `std::array<GridSequence, (size_t)GridSlot::Count> mGrids`: Grid sequences for each slot
- `uint32_t mGridFrame`: Current frame of grid sequence
- `uint32_t mGridFrameCount`: Number of frames in grid sequence
- `double mFrameRate`: Frame rate for grid playback (default 30 fps)
- `uint32_t mStartFrame`: Grid playback start frame
- `bool mPlaybackEnabled`: Whether grid playback is enabled
- `AABB mBounds`: Volume world-space bounds
- `GridVolumeData mData`: Host/device shared data structure
- `mutable UpdateFlags mUpdates`: Update flags

### GridVolume Struct (Slang)

**Signature**: `struct GridVolume`

**Purpose**: GPU-side grid volume representation

**Enum**:
- `EmissionMode`: Specifies how emission is rendered (Direct = 0, Blackbody = 1)

**Constants**:
- `kInvalidGrid = -1`: Invalid grid index

**Data Members**:
- `GridVolumeData data`: Grid volume data structure

**Methods**:
- `getBounds()`: Get world-space bounds
- `getEmissionMode()`: Get emission mode
- `getAnisotropy()`: Get phase function anisotropy
- `hasDensityGrid()`: Check if density grid is present
- `hasEmissionGrid()`: Check if emission grid is present
- `getDensityGrid()`: Get density grid index
- `getEmissionGrid()`: Get emission grid index

### GridVolumeData Struct (Slang)

**Signature**: `struct GridVolumeData`

**Purpose**: Host/device shared data structure for grid volume

**Data Members**:
- `float4x4 transform`: Local-space to world-space transform
- `float4x4 invTransform`: World-space to local-space transform
- `float3 boundsMin`: World-space bounds (minimum), default (0, 0, 0)
- `float densityScale`: Density scale factor, default 1.0
- `float3 boundsMax`: World-space bounds (maximum), default (0, 0, 0)
- `float emissionScale`: Emission scale factor, default 1.0
- `uint densityGrid`: Index of density grid, default 0
- `uint emissionGrid`: Index of emission grid, default 0
- `uint flags`: Flags (contains only emission mode for now), default 0
- `float anisotropy`: Phase function anisotropy (g), default 0.0
- `float3 albedo`: Medium scattering albedo, default (1, 1, 1)
- `float emissionTemperature`: Emission base temperature (K), default 0.0

**Alignment**: Size is multiple of 16 for GPU alignment

## Technical Details

### Update Flags System

The update flags system tracks what has changed since the last clear:

```cpp
enum class UpdateFlags
{
    None                = 0x0,  ///< Nothing updated.
    PropertiesChanged   = 0x1,  ///< Volume properties changed.
    GridsChanged        = 0x2,  ///< Volume grids changed.
    TransformChanged    = 0x4,  ///< Volume transform changed.
    BoundsChanged       = 0x8,  ///< Volume world-space bounds changed.
};
```

**Usage Pattern**:
1. Call `getUpdates()` to get current flags
2. Process updates based on flags
3. Call `clearUpdates()` to reset flags

**Flag Combination**: Multiple flags can be set simultaneously using bitwise OR

### Grid Slot System

Grids are organized into slots for different volume properties:

```cpp
enum class GridSlot
{
    Density,  ///< Absorbing/scattering medium density
    Emission,  ///< Emission medium density
    Count      ///< Must be last
};
```

**Slot Management**:
- Each slot can hold a grid sequence (for dynamic data) or single grid (for static data)
- Grid sequences are stored as `std::vector<ref<Grid>>`
- Empty slots contain empty sequences

**Grid Access**:
- `getGrid(slot)` returns the current grid based on `mGridFrame`
- If sequence is empty, returns null grid
- Frame index is clamped to sequence size

### Grid Sequence Management

**Static Grids**:
- Single grid per slot
- Created using `setGrid(slot, grid)`
- Replaces any existing sequence

**Dynamic Grids**:
- Multiple grids per slot (sequence)
- Created using `setGridSequence(slot, grids)` or `loadGridSequence(...)`
- Supports frame-based playback

**Frame Management**:
- `mGridFrame`: Current frame index (0 to mGridFrameCount-1)
- `mGridFrameCount`: Total number of frames (minimum 1)
- `mStartFrame`: Playback start frame (clamped to valid range)
- `mFrameRate`: Playback frame rate (1.0 to 1000.0 fps, default 30 fps)

**Playback**:
- Enabled/disabled using `setPlaybackEnabled(bool)`
- Updated using `updatePlayback(double currentTime)`
- Frame index computed as: `(mStartFrame + floor(currentTime * mFrameRate)) % mGridFrameCount`

### Grid Loading

**Single Grid Loading**:
```cpp
bool loadGrid(GridSlot slot, const std::filesystem::path& path, const std::string& gridname)
{
    auto grid = Grid::createFromFile(mpDevice, path, gridname);
    if (grid) setGrid(slot, grid);
    return grid != nullptr;
}
```

**Grid Sequence Loading (Files)**:
```cpp
static GridSequence createGridSequence(ref<Device> pDevice,
                                   const std::vector<std::filesystem::path>& paths,
                                   const std::string& gridname,
                                   bool keepEmpty)
{
    GridSequence grids;
    for (const auto& path : paths)
    {
        auto grid = Grid::createFromFile(pDevice, path, gridname);
        if (keepEmpty || grid) grids.push_back(grid);
    }
    return grids;
}
```

**Grid Sequence Loading (Directory)**:
```cpp
uint32_t loadGridSequence(GridSlot slot,
                       const std::filesystem::path& path,
                       const std::string& gridname,
                       bool keepEmpty)
{
    // Validate directory
    if (!std::filesystem::exists(path)) return 0;
    if (!std::filesystem::is_directory(path)) return 0;

    // Enumerate grid files
    std::vector<std::filesystem::path> paths;
    for (auto it : std::filesystem::directory_iterator(path))
    {
        if (hasExtension(it.path(), "nvdb") || hasExtension(it.path(), "vdb"))
            paths.push_back(it.path());
    }

    // Sort by length first, then alpha-numerically
    auto cmp = [](const std::filesystem::path& a, const std::filesystem::path& b) {
        auto sa = a.string();
        auto sb = b.string();
        return sa.length() != sb.length() ? sa.length() < sb.length() : sa < sb;
    };
    std::sort(paths.begin(), paths.end(), cmp);

    return loadGridSequence(slot, paths, gridname, keepEmpty);
}
```

**File Format Support**:
- NanoVDB (.nvdb): GPU-optimized format
- OpenVDB (.vdb): Industry standard format
- Both formats use `Grid::createFromFile()`

### Volume Properties

**Density Properties**:
- `densityScale`: Scale factor for density values (default 1.0)
- Applied to density grid values during rendering
- Clamped to [0, max_float]

**Emission Properties**:
- `emissionScale`: Scale factor for emission values (default 1.0)
- Applied to emission grid values during rendering
- Clamped to [0, max_float]

**Scattering Properties**:
- `albedo`: Scattering albedo (default (1, 1, 1))
- Represents fraction of scattered light
- Clamped to [0, 1] per component
- Used for phase function computation

**Phase Function Properties**:
- `anisotropy`: Phase function anisotropy g (default 0.0)
- Controls forward/backward scattering
- Range: [-0.99, 0.99]
- Used for Henyey-Greenstein phase function

**Emission Modes**:
- `Direct`: Direct emission from grid values
- `Blackbody`: Blackbody radiation based on temperature
- `emissionTemperature`: Base temperature in Kelvin (default 0.0)

### Transform System

**Transform Storage**:
- `transform`: Local-space to world-space transform
- `invTransform`: World-space to local-space transform
- Both stored in `GridVolumeData` for GPU access

**Transform Updates**:
- Updated through `updateFromAnimation(const float4x4& transform)`
- Inverse transform computed automatically
- Triggers `TransformChanged` flag

**Bounds Computation**:
- Bounds computed from all grids in all slots
- Transformed to world space using `transform`
- Stored in `GridVolumeData` for GPU access
- Triggers `BoundsChanged` flag

### Bounds Management

**Bounds Computation**:
```cpp
void updateBounds()
{
    AABB bounds;
    for (uint32_t slotIndex = 0; slotIndex < (uint32_t)GridSlot::Count; ++slotIndex)
    {
        const auto& grid = getGrid((GridSlot)slotIndex);
        if (grid && grid->getVoxelCount() > 0)
            bounds.include(grid->getWorldBounds());
    }
    bounds = bounds.transform(mData.transform);

    if (mBounds != bounds)
    {
        mBounds = bounds;
        mData.boundsMin = mBounds.valid() ? mBounds.minPoint : float3(0.f);
        mData.boundsMax = mBounds.valid() ? mBounds.maxPoint : float3(0.f);
        markUpdates(UpdateFlags::BoundsChanged);
    }
}
```

**Bounds Storage**:
- CPU-side: `AABB mBounds` for bounds queries
- GPU-side: `boundsMin` and `boundsMax` in `GridVolumeData`
- Both updated simultaneously

**Bounds Validity**:
- If no grids are loaded, bounds are set to (0, 0, 0)
- `mBounds.valid()` checks if bounds are valid
- Invalid bounds default to origin

### UI Rendering

**UI Layout**:
1. Grid playback controls (if sequence has multiple frames)
2. Density grid UI (if density grid is loaded)
3. Emission grid UI (if emission grid is loaded)
4. Volume properties (albedo, anisotropy)
5. Emission mode and temperature (if blackbody mode)

**Update Tracking**:
- UI changes trigger property setters
- Setters mark appropriate update flags
- `renderUI()` returns true if any changes occurred

**UI Variables**:
```cpp
const Gui::DropdownList kEmissionModeList = {
    { (uint32_t)GridVolume::EmissionMode::Direct, "Direct" },
    { (uint32_t)GridVolume::EmissionMode::Blackbody, "Blackbody" },
};
```

### Python Scripting Bindings

**Properties**:
- `name`: Volume name (read/write)
- `gridFrame`: Current grid frame (read/write)
- `gridFrameCount`: Number of grid frames (read-only)
- `frameRate`: Frame rate (read/write)
- `startFrame`: Start frame (read/write)
- `playbackEnabled`: Playback enabled (read/write)
- `densityGrid`: Density grid (read/write)
- `densityScale`: Density scale (read/write)
- `emissionGrid`: Emission grid (read/write)
- `emissionScale`: Emission scale (read/write)
- `albedo`: Scattering albedo (read/write)
- `anisotropy`: Phase function anisotropy (read/write)
- `emissionMode`: Emission mode (read/write)
- `emissionTemperature`: Emission temperature (read/write)

**Methods**:
- `loadGrid(slot, path, gridname)`: Load single grid from file
- `loadGridSequence(slot, paths, gridname, keepEmpty)`: Load grid sequence from files
- `loadGridSequence(slot, path, gridname, keepEmpty)`: Load grid sequence from directory

**Dependencies**:
- `Animatable`: Inherits from Animatable
- `Grid`: Uses Grid class for voxel data

### Animation Integration

**Animatable Interface**:
- Inherits from `Animatable` class
- Implements `updateFromAnimation(const float4x4& transform)`
- Transform updates trigger `TransformChanged` flag

**Animation Flow**:
1. Animation system computes transform
2. `updateFromAnimation()` called with new transform
3. Transform and inverse transform updated
4. Bounds recomputed with new transform
5. Update flags marked for scene update

## Integration Points

### Usage in Volume Module

The GridVolume class is used throughout the Volume module:

1. **Volume Rendering**: Shaders use GridVolume struct for volume sampling
2. **Scene Integration**: Scene manages GridVolume instances
3. **Animation System**: Animatable interface for transform animations
4. **Path Tracing**: GridVolume data used for volume path tracing

### Integration Pattern

```cpp
// Create volume
ref<GridVolume> volume = GridVolume::create(device, "MyVolume");

// Load density grid
volume->loadGrid(GridVolume::GridSlot::Density, "density.nvdb", "density");

// Set properties
volume->setAlbedo(float3(0.8f, 0.9f, 1.0f));
volume->setAnisotropy(0.3f);

// Bind to shader
shaderData.volume = volume->getData();
```

### Shader Integration

**Slang Shader Example**:
```slang
GridVolume volume;

// Check if grids are present
if (volume.hasDensityGrid() && volume.hasEmissionGrid())
{
    // Get grid indices
    uint densityGridIndex = volume.getDensityGrid();
    uint emissionGridIndex = volume.getEmissionGrid();

    // Access grids (assuming grid array is bound)
    Grid densityGrid = grids[densityGridIndex];
    Grid emissionGrid = grids[emissionGridIndex];

    // Sample volume
    float density = densityGrid.lookupLinearWorld(worldPos, accessor);
    float emission = emissionGrid.lookupLinearWorld(worldPos, accessor);

    // Apply scales
    density *= volume.data.densityScale;
    emission *= volume.data.emissionScale;

    // Get properties
    float3 albedo = volume.data.albedo;
    float anisotropy = volume.getAnisotropy();
    EmissionMode mode = volume.getEmissionMode();
}
```

## Architecture Patterns

### Composite Pattern

GridVolume composes multiple Grid instances:
- Density grid for absorbing/scattering medium
- Emission grid for emission medium
- Both can be static or dynamic

### State Pattern

Update flags track state changes:
- Multiple flags can be set simultaneously
- Flags are queried and cleared separately
- Enables efficient scene updates

### Strategy Pattern

Multiple emission strategies:
- Direct emission: Direct use of grid values
- Blackbody emission: Blackbody radiation based on temperature

### Template Method Pattern

Grid loading follows a template method:
1. Validate input (file/directory exists)
2. Enumerate files (if directory)
3. Sort files (if directory)
4. Load each grid
5. Create sequence
6. Set to slot

### Observer Pattern

Animatable interface for transform updates:
- Animation system notifies of transform changes
- GridVolume updates internal state
- Scene is notified of changes through update flags

## Code Patterns

### Property Setter Pattern

```cpp
void setDensityScale(float densityScale)
{
    if (mData.densityScale != densityScale)
    {
        mData.densityScale = densityScale;
        markUpdates(UpdateFlags::PropertiesChanged);
    }
}
```

### Grid Sequence Update Pattern

```cpp
void updateSequence()
{
    mGridFrameCount = 1;
    for (const auto& grids : mGrids)
        mGridFrameCount = std::max(mGridFrameCount, (uint32_t)grids.size());
    setGridFrame(std::min(mGridFrame, mGridFrameCount - 1));
}
```

### Playback Update Pattern

```cpp
void updatePlayback(double currentTime)
{
    if (mPlaybackEnabled && mGridFrameCount > 0)
    {
        uint32_t frameIndex = (mStartFrame +
                             (uint32_t)std::floor(std::max(0.0, currentTime) * mFrameRate)) %
                             mGridFrameCount;
        setGridFrame(frameIndex);
    }
}
```

### Transform Update Pattern

```cpp
void updateFromAnimation(const float4x4& transform)
{
    if (mData.transform != transform)
    {
        mData.transform = transform;
        mData.invTransform = inverse(transform);
        markUpdates(UpdateFlags::TransformChanged);
        updateBounds();
    }
}
```

## Use Cases

### Volumetric Rendering

Primary use case is GPU-accelerated volumetric rendering:
- Medical imaging (CT, MRI)
- Scientific visualization
- Atmospheric effects (clouds, fog)
- Volumetric lighting (global illumination)

### Participating Media

Supports heterogeneous participating media:
- Absorption and scattering (density grid)
- Emission (emission grid)
- Phase function control (anisotropy)
- Scattering albedo control

### Dynamic Volumes

Supports time-varying volume data:
- Animated clouds
- Time-varying scientific data
- Dynamic volumetric lighting
- Frame-based playback

### Path Tracing

Supports Monte Carlo path tracing through volumes:
- Density sampling for absorption/scattering
- Emission sampling for light sources
- Phase function sampling for scattering direction
- Majorant estimation for delta tracking

## Performance Considerations

### Memory Efficiency

- **Shared Data Structure**: `GridVolumeData` is shared between host and device
- **Sparse Grids**: Only non-empty grids are stored
- **Grid Sequences**: Multiple grids share same GPU resources
- **Alignment**: 16-byte alignment for optimal GPU access

### Update Efficiency

- **Flags-Based Tracking**: Only update what has changed
- **Lazy Bounds Computation**: Bounds updated only when needed
- **Incremental Updates**: Grid sequences updated incrementally
- **Minimal Copying**: Reference counting for grid sharing

### Rendering Performance

- **GPU-Ready Data**: Direct GPU access to volume properties
- **Transform Caching**: Both transform and inverse transform stored
- **Bounds Caching**: Bounds pre-computed and stored
- **Grid Indexing**: Direct indexing for grid access

### Playback Performance

- **Frame Rate Control**: Adjustable frame rate (1-1000 fps)
- **Modulo Arithmetic**: Efficient frame index computation
- **Conditional Updates**: Only update when playback is enabled
- **Start Frame Support**: Flexible playback range

## Limitations

### Format Limitations

- **Float Only**: Grids only support float data
- **NanoVDB/OpenVDB Only**: Only supports these two formats
- **Fixed Slots**: Only two grid slots (density, emission)
- **No Custom Slots**: Cannot add custom grid slots

### Quality Limitations

- **Scale Factors**: Limited to single scale factor per grid
- **Uniform Albedo**: Single albedo for entire volume
- **Uniform Anisotropy**: Single anisotropy for entire volume
- **Blackbody Approximation**: Simple blackbody model

### Performance Limitations

- **Bounds Computation**: O(N) where N is number of grids
- **Sequence Updates**: All grids checked when sequence changes
- **Transform Updates**: Bounds recomputed on transform change
- **No LOD**: No automatic level-of-detail support

### Dynamic Limitations

- **No Real-Time Updates**: Grids cannot be modified at runtime
- **No Streaming**: Entire grid sequence must be in memory
- **No Interpolation**: No interpolation between grid frames
- **No Compression**: No runtime compression of grid data

## Best Practices

### When to Use GridVolume

1. **Volumetric Rendering**: Ideal for GPU-accelerated volume rendering
2. **Heterogeneous Media**: Support for density and emission grids
3. **Dynamic Data**: Support for time-varying volume data
4. **Path Tracing**: Designed for Monte Carlo path tracing

### When to Avoid GridVolume

1. **Surface Rendering**: Overkill for surface-only rendering
2. **Homogeneous Media**: Use simpler homogeneous volume models
3. **CPU-Only Rendering**: Overhead of GPU data not justified
4. **Real-Time Updates**: Grids are immutable after loading

### Usage Guidelines

1. **Grid Loading**: Load grids from files or directories
2. **Property Tuning**: Adjust scale factors and material properties
3. **Animation**: Use Animatable interface for transform animations
4. **Playback**: Use playback for dynamic grid sequences
5. **Update Tracking**: Use update flags for efficient scene updates

### Optimization Tips

1. **Grid Sequences**: Use sequences for dynamic data instead of reloading
2. **Bounds Caching**: Leverage pre-computed bounds for culling
3. **Update Flags**: Use flags to minimize unnecessary updates
4. **Transform Caching**: Use cached transforms for coordinate conversions
5. **Playback Control**: Adjust frame rate for smooth playback

## Notes

- Inherits from Animatable for transform animations
- Supports both static and dynamic grid sequences
- Dual grid system (density and emission)
- Multiple emission modes (Direct and Blackbody)
- Flags-based update tracking for efficient scene updates
- Complete UI rendering for debugging and editing
- Full Python scripting support
- Host/device shared data structure (GridVolumeData)
- 16-byte alignment for optimal GPU access
- Supports NanoVDB (.nvdb) and OpenVDB (.vdb) formats
- Frame-based playback with adjustable frame rate (1-1000 fps)
- Modulo arithmetic for frame index computation
- Automatic bounds computation and tracking
- Transform and inverse transform stored for GPU access
- Albedo clamped to [0, 1] per component
- Anisotropy clamped to [-0.99, 0.99]
- Emission temperature in Kelvin
- GridVolumeData size is multiple of 16 (static_assert)
- Update flags: None, PropertiesChanged, GridsChanged, TransformChanged, BoundsChanged
- Grid slots: Density, Emission
- Emission modes: Direct, Blackbody
- Constants: kMaxAnisotropy = 0.99, kMinFrameRate = 1.0, kMaxFrameRate = 1000.0
- Default values: densityScale = 1.0, emissionScale = 1.0, albedo = (1, 1, 1), anisotropy = 0.0, emissionTemperature = 0.0
- UI variables: kEmissionModeList for dropdown
- Python bindings with Animatable and Grid dependencies
- Deprecated: Volume alias for GridVolume in Python bindings
