# Lights - Light Sampling System

## Module State Machine

**Status**: In Progress

## Dependency Graph

### Components (Pending Analysis)

- [ ] **EmissiveLightSampler** - Base class for emissive light samplers
  - [ ] EmissiveLightSampler.cpp - Implementation
  - [ ] EmissiveLightSampler.h - Interface
  - [ ] EmissiveLightSampler.slang - Shader interface
  - [ ] EmissiveLightSamplerHelpers.slang - Shader helpers
  - [ ] EmissiveLightSamplerType.slangh - Type definitions
- [ ] **EmissivePowerSampler** - Power-based emissive light sampling
  - [ ] EmissivePowerSampler.cpp - Implementation
  - [ ] EmissivePowerSampler.h - Interface
  - [ ] EmissivePowerSampler.slang - Shader implementation
- [ ] **EmissiveUniformSampler** - Uniform emissive light sampling
  - [ ] EmissiveUniformSampler.cpp - Implementation
  - [ ] EmissiveUniformSampler.h - Interface
  - [ ] EmissiveUniformSampler.slang - Shader implementation
- [ ] **EnvMapSampler** - Environment map light sampling
  - [ ] EnvMapSampler.cpp - Implementation
  - [ ] EnvMapSampler.h - Interface
  - [ ] EnvMapSampler.slang - Shader implementation
  - [ ] EnvMapSamplerSetup.cs.slang - Setup shader
- [ ] **LightBVH** - Light bounding volume hierarchy
  - [ ] LightBVH.cpp - Implementation
  - [ ] LightBVH.h - Interface
  - [ ] LightBVH.slang - Shader implementation
  - [ ] LightBVHRefit.cs.slang - Refitting shader
- [ ] **LightBVHBuilder** - BVH construction
  - [ ] LightBVHBuilder.cpp - Implementation
  - [ ] LightBVHBuilder.h - Interface
- [ ] **LightBVHSampler** - BVH-based light sampling
  - [ ] LightBVHSampler.cpp - Implementation
  - [ ] LightBVHSampler.h - Interface
  - [ ] LightBVHSampler.slang - Shader implementation
  - [ ] LightBVHSamplerSharedDefinitions.slang - Shared definitions
  - [ ] LightBVHTypes.slang - Type definitions
- [ ] **LightHelpers** - Light utility functions
  - [ ] LightHelpers.slang - Shader utilities

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Core/Program** - Shader program management (DefineList)
- **Scene/Lights** - Light collection interfaces (ILightCollection)
- **Utils/Math** - Mathematical utilities

## Module Overview

The Lights subsystem provides multiple light sampling strategies for efficient global illumination in path tracing. It implements various emissive light samplers, environment map sampling, and hierarchical light BVH structures for accelerated light selection.

## Component Specifications

### EmissiveLightSampler Base Class

**Files**: [`EmissiveLightSampler.h`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:1)

**Purpose**: Abstract base class for emissive light sampler implementations.

**Core Virtual Methods**:
- [`update()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:54) - Update sampler for current frame
- [`getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:59) - Get shader defines
- [`bindShaderData()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:63) - Bind sampler data to shader
- [`renderUI()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:68) - Render GUI

**Protected Members**:
- `mType` - EmissiveLightSamplerType enum value
- `mpDevice` - GPU device reference
- `mpLightCollection` - Light collection reference
- `mUpdateFlagsConnection` - Signal connection for update flags
- `mLightCollectionUpdateFlags` - Accumulated update flags

**Design Pattern**: Strategy pattern - all samplers implement the same interface for interchangeability.

### EmissiveLightSamplerType

**File**: [`EmissiveLightSamplerType.slangh`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerType.slangh:1)

**Purpose**: Defines types of emissive light samplers for shader use.

**Types Defined**:
- Various sampler type identifiers (enum-like structure)
- Used for shader conditional compilation
- Enables runtime sampler selection

### EmissivePowerSampler

**Purpose**: Implements power-based emissive light sampling.

**Sampling Strategy**: Samples lights proportionally to their radiant power, ensuring brighter lights contribute more to the final result.

### EmissiveUniformSampler

**Purpose**: Implements uniform emissive light sampling.

**Sampling Strategy**: Samples all emissive lights with equal probability, regardless of their power.

### EnvMapSampler

**Purpose**: Implements environment map light sampling.

**Key Features**:
- Precomputed environment map integration
- Efficient sampling of environment lighting
- Setup shader for environment map preparation
- Supports various environment map formats

**Setup Shader**: [`EnvMapSamplerSetup.cs.slang`](Source/Falcor/Rendering/Lights/EnvMapSamplerSetup.cs.slang:1) prepares environment map data structures.

### LightBVH

**Purpose**: Bounding volume hierarchy for efficient light sampling and traversal.

**Key Features**:
- Hierarchical organization of emissive lights
- O(log N) light selection complexity
- GPU-friendly data structures
- Support for dynamic scene updates via refitting

**Shader Implementation**: [`LightBVH.slang`](Source/Falcor/Rendering/Lights/LightBVH.slang:1) provides GPU traversal and sampling.

**Refitting**: [`LightBVHRefit.cs.slang`](Source/Falcor/Rendering/Lights/LightBVHRefit.cs.slang:1) enables BVH updates without full reconstruction.

### LightBVHBuilder

**Purpose**: Constructs light BVH from scene emissive lights.

**Key Features**:
- Efficient BVH construction algorithms
- Optimized for various light distributions
- Supports dynamic scene updates
- GPU-friendly construction

### LightBVHSampler

**Purpose**: BVH-based light sampling for path tracing.

**Key Features**:
- Hierarchical light selection
- Spatial coherence exploitation
- Efficient candidate generation
- Reduced variance through stratified sampling

**Shared Definitions**: [`LightBVHSamplerSharedDefinitions.slang`](Source/Falcor/Rendering/Lights/LightBVHSamplerSharedDefinitions.slang:1) provides common structures and functions.

**Type Definitions**: [`LightBVHTypes.slang`](Source/Falcor/Rendering/Lights/LightBVHTypes.slang:1) defines BVH node and traversal structures.

### LightHelpers

**Purpose**: Utility functions for light-related computations.

**Features**:
- Common light calculations
- Sampling utilities
- Coordinate transformations
- PDF computations

## Architecture Patterns

### Strategy Pattern
- [`EmissiveLightSampler`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:44) defines common interface
- Multiple implementations (Power, Uniform, BVH)
- Runtime selection based on configuration

### Builder Pattern
- [`LightBVHBuilder`](Source/Falcor/Rendering/Lights/LightBVHBuilder.h:1) constructs BVH
- Separates construction from traversal
- Optimized for specific use cases

### Hierarchical Pattern
- [`LightBVH`](Source/Falcor/Rendering/Lights/LightBVH.h:1) organizes lights hierarchically
- Enables O(log N) queries
- Spatial coherence exploitation

## Technical Details

### Sampling Strategies

**Power Sampling**:
- Samples proportional to light power
- Reduces variance for scenes with varying light intensities
- Requires light power information

**Uniform Sampling**:
- Equal probability for all lights
- Simpler implementation
- Higher variance for scenes with varying light intensities

**BVH Sampling**:
- Hierarchical traversal
- Spatial subdivision
- Efficient candidate selection
- Support for dynamic scenes

### BVH Structure

**Node Types** (from [`LightBVHTypes.slang`](Source/Falcor/Rendering/Lights/LightBVHTypes.slang:1)):
- Internal nodes with child pointers
- Leaf nodes containing light indices
- Bounding boxes for spatial queries

**Construction**:
- Surface area heuristic (SAH) for optimal splits
- Binned construction for efficiency
- Support for various light distributions

### Environment Map Sampling

**Precomputation**:
- Environment map integration
- PDF texture generation
- Luminance texture creation
- CDF tables for efficient sampling

**Sampling**:
- 2D sampling from precomputed CDF
- Importance sampling based on luminance
- Support for various environment map formats

## Performance Considerations

### Light BVH
- Construction cost: O(N log N)
- Query cost: O(log N)
- Memory overhead: O(N) for BVH structure
- GPU-friendly traversal patterns

### Power Sampling
- Reduced variance for heterogeneous lights
- Additional cost for power computation
- Better convergence for scenes with high dynamic range

### Uniform Sampling
- Lowest computational cost
- Higher variance for heterogeneous scenes
- Simpler implementation

### Environment Map Sampling
- Precomputation cost: one-time
- Per-frame cost: O(1) per sample
- Memory cost: environment map textures
- Excellent temporal coherence

## Integration Points

### Scene Integration
- Integrates with [`ILightCollection`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:32)
- Updates on scene changes
- Tracks light collection update flags

### Shader Integration
- All samplers provide [`getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:59) for compilation
- [`bindShaderData()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:63) for runtime binding
- Consistent shader variable naming

### RTXDI Integration
- Light BVH can be used with RTXDI for candidate generation
- Provides efficient light selection for resampled importance sampling
- Supports various light types

## Progress Log

- **2026-01-07T18:50:27Z**: Lights subsystem analysis initiated. High-level overview created. Component structure established. Ready for detailed analysis of each component.

## Next Steps

Analyze Materials subsystem to understand material system and BSDF implementations.
