# Volumes - Volume Rendering

## Module State Machine

**Status**: In Progress

## Dependency Graph

### Components (Pending Analysis)

- [ ] **GridVolumeSampler** - Grid-based volume sampling
  - [ ] GridVolumeSampler.cpp - Implementation
  - [ ] GridVolumeSampler.h - Interface
  - [ ] GridVolumeSampler.slang - Shader implementation
  - [ ] GridVolumeSamplerParams.slang - Parameter definitions
- [ ] **HomogeneousVolumeSampler** - Homogeneous volume sampling
  - [ ] HomogeneousVolumeSampler.slang - Shader implementation
- [ ] **PhaseFunction** - Volume phase functions
  - [ ] PhaseFunction.slang - Shader interface
  - [ ] HenyeyGreenstein.slang - Henyey-Greenstein phase function
  - [ ] IsotropicPhase.slang - Isotropic phase function

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Utils/Math** - Mathematical utilities

## Module Overview

The Volumes subsystem provides volume rendering capabilities for participating media in path tracing. It supports both grid-based and homogeneous volume sampling with various phase function implementations.

## Component Specifications

### GridVolumeSampler

**Purpose**: Grid-based volume sampling for participating media.

**Key Features**:
- Volumetric data structures
- Grid-based sampling
- Efficient traversal
- Support for various volume types

**Parameters** (from [`GridVolumeSamplerParams.slang`](Source/Falcor/Rendering/Volumes/GridVolumeSamplerParams.slang:1)):
- Volume dimensions and resolution
- Sampling parameters
- Phase function selection
- Density and absorption coefficients

### HomogeneousVolumeSampler

**Purpose**: Homogeneous volume sampling for uniform participating media.

**Key Features**:
- Analytical sampling
- Closed-form solutions
- Efficient evaluation
- No grid required

### PhaseFunction

**Purpose**: Volume phase function implementations for scattering.

**Phase Functions**:

- **Henyey-Greenstein** - Anisotropic phase function
  - Henyey-Greenstein asymmetry parameter
  - Directional scattering control
  - Cloud and atmospheric modeling

- **Isotropic** - Isotropic phase function
  - Uniform scattering
  - Simple implementation
  - Fog and uniform media modeling

## Architecture Patterns

### Sampling Pattern

- Grid-based sampling for heterogeneous volumes
- Analytical sampling for homogeneous volumes
- Interchangeable phase functions

### Volume Rendering Pattern

- Participating media integration
- Ray marching through volumes
- Scattering and absorption
- Phase function evaluation

## Technical Details

### Grid-Based Sampling

**Volume Representation**:
- 3D grid of density values
- Trilinear interpolation
- Bounding volume for acceleration
- Efficient GPU traversal

**Sampling Strategy**:
- Woodcock tracking for heterogeneous volumes
- Delta tracking for absorption
- Phase function evaluation at scattering events
- Multiple scattering support

### Homogeneous Sampling

**Analytical Solutions**:
- Closed-form sampling for uniform media
- No grid required
- Very efficient evaluation
- Limited to homogeneous volumes

### Phase Functions

**Henyey-Greenstein**:
- Anisotropic scattering
- Asymmetry parameter g
- Elevation-dependent scattering
- Forward and backward scattering control

**Isotropic**:
- Uniform scattering
- No directional preference
- Simple implementation
- Fast evaluation

## Performance Considerations

### Grid-Based Sampling

- Memory cost: O(N³) for volume grid
- Traversal cost: O(log N) with acceleration
- Interpolation cost: Constant time
- Flexible for heterogeneous volumes

### Homogeneous Sampling

- Memory cost: O(1) per volume
- Evaluation cost: O(1) per sample
- Very efficient
- Limited to homogeneous volumes

### Phase Functions

- Henyey-Greenstein: Higher cost, more flexibility
- Isotropic: Lower cost, simpler
- Choice depends on material properties

## Integration Points

### Scene Integration

- Volumes integrate with scene volume data
- Participating media support
- Volume bounds from scene

### Shader Integration

- Phase functions provide shader implementations
- Consistent interface for shader binding
- Volume parameters passed via constant buffers

### Rendering Integration

- Volumes integrate with path tracers
- Ray marching through volumes
- Scattering and absorption evaluation
- Multiple scattering support

## Progress Log

- **2026-01-07T18:52:51Z**: Volumes subsystem analysis completed. High-level overview created covering grid-based and homogeneous volume sampling, plus phase function implementations. Module marked as In Progress pending detailed component analysis.

## Next Steps

Rendering module analysis nearly complete. Ready to mark as complete and proceed to Scene module analysis.
