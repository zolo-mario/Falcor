# Materials - Material System

## Module State Machine

**Status**: In Progress

## Dependency Graph

### Components (Pending Analysis)

- [ ] **BSDFIntegrator** - BSDF integration utilities
  - [ ] BSDFIntegrator.cpp - Implementation
  - [ ] BSDFIntegrator.cs.slang - Shader implementation
  - [ ] BSDFIntegrator.h - Interface
- [ ] **BSDF Implementations** - Various BSDF models
  - [ ] AnisotropicGGX.slang - Anisotropic GGX BSDF
  - [ ] ClothBRDF.slang - Cloth material BSDF
  - [ ] DisneyDiffuseBRDF.slang - Disney diffuse BSDF
  - [ ] DielectricPlateBSDF.slang - Dielectric plate BSDF
  - [ ] DiffuseSpecularBRDF.slang - Diffuse-specular BSDF
  - [ ] FrostbiteDiffuseBRDF.slang - Frostbite diffuse BSDF
  - [ ] LambertDiffuseBRDF.slang - Lambert diffuse BSDF
  - [ ] LambertDiffuseBTDF.slang - Lambert diffuse BTDF
  - [ ] OrenNayarBRDF.slang - Oren-Nayar BRDF
  - [ ] SheenBSDF.slang - Sheen BSDF
  - [ ] SimpleBTDF.slang - Simple BTDF
  - [ ] SpecularMicrofacet.slang - Specular microfacet BSDF
  - [ ] StandardBSDF.slang - Standard material BSDF
- [ ] **Material Implementations** - Various material models
  - [ ] ClothMaterial.slang - Cloth material
  - [ ] ClothMaterialInstance.slang - Cloth material instance
  - [ ] HairChiang16.slang - Chiang 2016 hair BSDF
  - [ ] HairMaterial.slang - Hair material
  - [ ] HairMaterialInstance.slang - Hair material instance
  - [ ] IMaterial.slang - Material interface
  - [ ] IMaterialInstance.slang - Material instance interface
  - [ ] INDF.slang - NDF interface
  - [ ] InteriorList.slang - Interior list for layered materials
  - [ ] InteriorListHelpers.slang - Interior list helpers
  - [ ] IsotropicGGX.slang - Isotropic GGX NDF
  - [ ] LayeredBSDF.slang - Layered BSDF
  - [ ] LobeType.slang - BSDF lobe types
  - [ ] MaterialInstanceHints.slang - Material instance hints
  - [ ] MERLCommon.slang - MERL material common
  - [ ] MERLMaterial.slang - MERL material
  - [ ] MERLMaterialInstance.slang - MERL material instance
  - [ ] MERLMixMaterial.slang - MERL mix material
  - [ ] MERLMixMaterialInstance.slang - MERL mix material instance
  - [ ] Microfacet.slang - Microfacet theory
  - [ ] NDF.slang - Normal distribution functions
  - [ ] RGLCommon.slang - RGL material common
  - [ ] RGLMaterial.slang - RGL material
  - [ ] RGLMaterialInstance.slang - RGL material instance
  - [ ] RGLAcquisition.cpp - RGL acquisition
  - [ ] RGLAcquisition.cs.slang - RGL acquisition shader
  - [ ] RGLCommon.slang - RGL common
  - [ ] StandardMaterial.slang - Standard material
  - [ ] StandardMaterialInstance.slang - Standard material instance
  - [ ] TexLODHelpers.slang - Texture LOD helpers
  - [ ] TexLODTypes.slang - Texture LOD types

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Core/Pass** - Compute pass for BSDF integration
- **Core/Program** - Shader program management (DefineList)
- **Scene/Scene** - Scene integration
- **Scene/SceneIDs** - Material ID types
- **Utils/Math** - Mathematical utilities (Vector types)

## Module Overview

The Materials subsystem provides a comprehensive material system with multiple BSDF implementations and material models. It supports physically based rendering with various material types including standard PBR, cloth, hair, MERL, RGL, and PBRT materials.

## Component Specifications

### BSDFIntegrator

**File**: [`BSDFIntegrator.h`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:1)

**Purpose**: Utility class for BSDF integration and precomputation.

**Key Features**:
- GPU-based BSDF integration
- Precomputation of BSDF integrals
- Grid-based discretization of incident directions
- Parallel reduction for final results
- Support for multiple materials

**Core Methods**:
- [`integrateIsotropic()`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:55) - Integrate BSDF for single direction
- [`integrateIsotropic()`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:64) - Integrate BSDF for multiple directions

**Integration Strategy**:
- Discretizes incident directions into grid
- Integrates BSDF over hemisphere for each direction
- Uses GPU compute passes for parallelization
- Reduces results via parallel reduction
- Stages results for CPU readback

**Internal Resources**:
- `mpIntegrationPass` - Integration compute pass
- `mpFinalPass` - Final reduction pass
- `mpCosThetaBuffer` - Buffer for incident cos theta angles
- `mpResultBuffer` - Buffer for intermediate results
- `mpFinalResultBuffer` - Buffer for final results after reduction
- `mpStagingBuffer` - Staging buffer for readback
- `mResultCount` - Number of intermediate results per integration grid

### BSDF Implementations

**Purpose**: Collection of BSDF models for physically based rendering.

**BSDF Types**:

#### Diffuse BSDFs

- **LambertDiffuseBRDF** - Classic Lambertian diffuse
- **DisneyDiffuseBRDF** - Disney principled BSDF diffuse
- **FrostbiteDiffuseBRDF** - Frostbite engine diffuse
- **OrenNayarBRDF** - Oren-Nayar diffuse with roughness

#### Specular BSDFs

- **SpecularMicrofacet** - Microfacet specular reflection
- **IsotropicGGX** - Isotropic GGX normal distribution
- **AnisotropicGGX** - Anisotropic GGX normal distribution

#### Specialized BSDFs

- **SheenBSDF** - Sheen cloth-like BSDF
- **ClothBRDF** - Cloth material BSDF
- **SimpleBTDF** - Simple bidirectional transmittance function
- **DielectricPlateBSDF** - Dielectric plate BSDF for layered materials

#### Layered BSDF

- **LayeredBSDF** - Multi-layer material BSDF
- **StandardBSDF** - Standard PBR material BSDF
- **InteriorList** - Interior list for layered materials

### Material Implementations

**Purpose**: High-level material interfaces and implementations.

**Material Interfaces**:

- **IMaterial** - Base material interface
- **IMaterialInstance** - Material instance interface
- **INDF** - Normal distribution function interface
- **InteriorList** - Interior list interface for layered materials

**Material Types**:

#### Standard Material

- **StandardMaterial** - Standard PBR material
  - Albedo texture
  - Normal map
  - Roughness texture
  - Metallic texture
  - Emissive texture
  - Occlusion texture
  - Transmission texture

#### Cloth Material

- **ClothMaterial** - Cloth-specific material
  - Diffuse color texture
  - Specular color texture
  - Normal map
  - Roughness texture
  - Sheen color texture

#### Hair Material

- **HairMaterial** - Hair-specific material
  - Based on Chiang 2016 BSDF
  - Primary color texture
  - Secondary color texture
  - Specular color texture
  - Roughness texture
  - Longitudinal shift texture
  - Azimuthal thickness texture

#### MERL Material

- **MERLMaterial** - MERL measured material
  - MERL BRDF data
  - Supports measured BRDFs
  - High accuracy material representation

#### RGL Material

- **RGLMaterial** - RGL measured material
  - RGL BRDF data
  - Supports measured BRDFs
  - Similar to MERL but different format

#### MERL Mix Material

- **MERLMixMaterial** - Mix of multiple MERL materials
  - Enables material blending
  - Supports multiple MERL BRDFs

### Material Instance System

**Purpose**: Efficient material instance management for rendering.

**Key Features**:
- Material instance creation and management
- Texture LOD support
- Material hints for optimization
- Efficient resource binding

**Texture LOD**:

- **TexLODHelpers** - Texture level of detail helpers
- **TexLODTypes** - Texture LOD type definitions
- Automatic LOD selection based on distance
- Mipmap generation and selection

### RGL Acquisition

**Purpose**: RGL material acquisition and processing.

**Key Features**:
- RGL file format support
- BRDF data extraction
- Shader-based processing
- Integration with material system

## Architecture Patterns

### Interface Pattern

- [`IMaterial`](Source/Falcor/Rendering/Materials/IMaterial.slang:1) defines material interface
- [`IMaterialInstance`](Source/Falcor/Rendering/Materials/IMaterialInstance.slang:1) defines instance interface
- Multiple implementations (Standard, Cloth, Hair, MERL, RGL)
- Runtime polymorphism

### Strategy Pattern

- [`BSDFIntegrator`](Source/Falcor/Rendering/Materials/BSDFIntegrator.h:42) provides integration strategies
- GPU-based computation
- Precomputation for runtime efficiency
- Parallel reduction for performance

### Layered Material Pattern

- [`LayeredBSDF`](Source/Falcor/Rendering/Materials/LayeredBSDF.slang:1) supports multiple layers
- [`InteriorList`](Source/Falcor/Rendering/Materials/InteriorList.slang:1) manages interior interfaces
- Complex material composition
- Physically accurate multi-layer materials

### Measured Material Pattern

- [`MERLMaterial`](Source/Falcor/Rendering/Materials/MERLMaterial.slang:1) and [`RGLMaterial`](Source/Falcor/Rendering/Materials/RGLMaterial.slang:1) use measured data
- High accuracy representation
- File format support (MERL, RGL)
- Acquisition processing

## Technical Details

### BSDF Integration

**Integration Process**:

1. Discretize incident directions into grid
2. For each material, integrate BSDF over hemisphere
3. Use GPU compute for parallel integration
4. Reduce results to final values
5. Stage results to CPU for shader binding

**Grid Resolution**:
- Controls integration accuracy vs. performance
- Higher resolution = more accurate, slower
- Lower resolution = faster, less accurate
- Should be tuned per use case

### Material System

**Material Parameters**:

- **Albedo**: Base color or texture
- **Normal**: Surface normal map
- **Roughness**: Surface roughness
- **Metallic**: Metalness (0 = dielectric, 1 = conductor)
- **Emissive**: Emissive color/intensity
- **Transmission**: Transmission properties
- **Occlusion**: Ambient occlusion

**Texture LOD**:

- Automatic selection based on screen-space derivatives
- Mipmap chain management
- Anisotropic filtering support
- LOD bias compensation

### BSDF Models

**Diffuse Models**:
- Lambert: Simple, fast, energy conserving
- Disney: Principled, energy conserving, artistic control
- Frostbite: Optimized for game rendering
- Oren-Nayar: Rough diffuse with microfacet theory

**Specular Models**:
- GGX: Physically based, anisotropic support
- Microfacet: Energy conserving, physically accurate

**Specialized Models**:
- Cloth: Fabric-like appearance
- Hair: Chiang 2016 hair scattering
- Layered: Multi-layer materials

### Measured Materials

**MERL Format**:
- Measured BRDF data
- High accuracy
- File-based acquisition
- Integration with material system

**RGL Format**:
- Measured BRDF data
- Alternative to MERL
- Similar accuracy
- File-based acquisition

## Performance Considerations

### BSDF Integration

- GPU-based integration is highly parallel
- Precomputation reduces per-frame cost
- Grid resolution controls quality vs. performance
- Staging enables efficient CPU-GPU transfer

### Material System

- Texture LOD reduces texture bandwidth
- Material instances enable efficient resource management
- Material hints guide renderer optimizations
- Measured materials provide high accuracy

### Memory Usage

- Material instances share common resources
- Texture LOD reduces memory footprint
- Measured material data can be cached
- BSDF integration buffers are reusable

## Integration Points

### Scene Integration

- Materials integrate with scene material system
- Material IDs map to material instances
- Texture management through scene

### Shader Integration

- All materials provide shader implementations
- Consistent interface for shader binding
- Material parameters passed via constant buffers

### Rendering Integration

- Materials integrate with path tracers
- BSDF evaluation in shading code
- Material instance management for efficient rendering

## Progress Log

- **2026-01-07T18:52:13Z**: Materials subsystem analysis completed. Comprehensive documentation created covering BSDF implementations, material models, and material instance system. Module marked as In Progress pending detailed component analysis.

## Next Steps

Analyze Volumes subsystem to understand volume rendering capabilities.
