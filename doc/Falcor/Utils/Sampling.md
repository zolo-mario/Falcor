# Sampling - Sampling Utilities

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **SampleGenerator** - Sample generator interface
  - [x] **SampleGeneratorInterface** - Sample generator shader interface
  - [x] **SampleGeneratorType** - Sample generator type definitions
- [x] **AliasTable** - Alias method for sampling
  - [x] **AliasTable.slang** - Alias table shader
- [x] **LowDiscrepancy** - Low discrepancy sequences
  - [x] **HammersleySequence** - Hammersley sequence
- [x] **Pseudorandom** - Pseudorandom number generators
  - [x] **Xorshift32** - XOR shift PRNG
  - [x] **Xoshiro** - Xoshiro PRNG
- [x] **TinyUniformSampleGenerator** - Tiny uniform sample generator
- [x] **UniformSampleGenerator** - Uniform sample generator

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management (DefineList, ShaderVar)
- **Utils/UI** - User interface (Gui)

## Module Overview

The Sampling module provides comprehensive sampling utilities for the Falcor rendering framework. It includes a sample generator interface with multiple implementations (uniform, low-discrepancy, pseudorandom), alias method for discrete probability distributions, and various sampling patterns. These utilities are essential for Monte Carlo integration, importance sampling, and stochastic rendering techniques.

## Component Specifications

### SampleGenerator

**Files**:
- [`SampleGenerator.h`](Source/Falcor/Utils/Sampling/SampleGenerator.h:1) - Sample generator header
- [`SampleGenerator.cpp`](Source/Falcor/Utils/Sampling/SampleGenerator.cpp:1) - Sample generator implementation
- [`SampleGeneratorInterface.slang`](Source/Falcor/Utils/Sampling/SampleGeneratorInterface.slang:1) - Sample generator shader interface
- [`SampleGeneratorType.slangh`](Source/Falcor/Utils/Sampling/SampleGeneratorType.slangh:1) - Sample generator type definitions

**Purpose**: Sample generator interface for GPU-based sampling.

**Key Features**:
- Abstract base class for sample generators
- Factory function for creating specific types
- Shader macro definitions
- Shader data binding
- Frame management (begin/end)
- UI rendering support
- Type registration system

**Core Methods**:

**Factory**:
- [`create(ref<Device> pDevice, uint32_t type)`](Source/Falcor/Utils/Sampling/SampleGenerator.h:61) - Create sample generator of specified type
  - Parameters:
    - `pDevice` - GPU device
    - `type` - Sample generator type (from SampleGeneratorType.slangh)
  - Returns: New SampleGenerator object
  - Throws exception on error

**Configuration**:
- [`getDefines() const`](Source/Falcor/Utils/Sampling/SampleGenerator.h:67) - Get macro definitions for shader
  - Returns: DefineList with required macros
  - Must be set on shader program using this sampler

**Shader Binding**:
- [`bindShaderData(const ShaderVar& var) const`](Source/Falcor/Utils/Sampling/SampleGenerator.h:73) - Bind data to program vars
  - Parameters:
    - `var` - Shader variable to set data into
  - Binds sampler data to shader
  - Doesn't change resource declarations

**Frame Management**:
- [`beginFrame(RenderContext* pRenderContext, const uint2& frameDim)`](Source/Falcor/Utils/Sampling/SampleGenerator.h:87) - Begin frame
  - Parameters:
    - `pRenderContext` - Render context
    - `frameDim` - Current frame dimensions
  - Returns: True if internal state changed and bindShaderData() should be called
  - Should be called at beginning of each frame for samplers with per-frame setup

- [`endFrame(RenderContext* pRenderContext, const ref<Texture>& pRenderOutput)`](Source/Falcor/Utils/Sampling/SampleGenerator.h:95) - End frame
  - Parameters:
    - `pRenderContext` - Render context
    - `pRenderOutput` - Rendered output
  - Should be called at end of each frame for samplers with per-frame setup

**UI**:
- [`renderUI(Gui::Widgets& widget)`](Source/Falcor/Utils/Sampling/SampleGenerator.h:78) - Render sampler UI
  - Parameters:
    - `widget` - GUI widgets
  - Renders sampler-specific UI controls

**Type Management**:
- [`getGuiDropdownList()`](Source/Falcor/Utils/Sampling/SampleGenerator.h:100) - Get GUI dropdown list of available sample generators
  - Returns: Dropdown list for UI

- [`registerType(uint32_t type, const std::string& name, std::function<ref<SampleGenerator>(ref<Device>)> createFunc)`](Source/Falcor/Utils/Sampling/SampleGenerator.h:108) - Register sample generator type
  - Parameters:
    - `type` - Sample generator type
    - `name` - Descriptive name for UI
    - `createFunc` - Function to create instance
  - Registers type for factory creation

**Data Structures**:

**SampleGenerator Class**:
- [`mpDevice`](Source/Falcor/Utils/Sampling/SampleGenerator.h:113) - Device reference
- [`mType`](Source/Falcor/Utils/Sampling/SampleGenerator.h:114) - Sample generator type

**Protected Members**:
- Device reference and type for derived classes

**Technical Details**:

**Abstract Base Class**:
- Provides common interface for all sample generators
- Derived classes implement specific sampling patterns
- Factory pattern for type creation

**Shader Integration**:
- Import SampleGeneratorInterface.slang in shader
- Use getDefines() to get required macros
- Bind shader data with bindShaderData()
- Frame management for per-frame setup

**Type Registration**:
- Static registration system
- Factory function uses registered types
- UI integration through dropdown list

**Use Cases**:
- Monte Carlo integration
- Importance sampling
- Path tracing
- Light sampling
- Material sampling

### AliasTable

**Files**:
- [`AliasTable.h`](Source/Falcor/Utils/Sampling/AliasTable.h:1) - Alias table header
- [`AliasTable.cpp`](Source/Falcor/Utils/Sampling/AliasTable.cpp:1) - Alias table implementation
- [`AliasTable.slang`](Source/Falcor/Utils/Sampling/AliasTable.slang:1) - Alias table shader

**Purpose**: Alias method for sampling from discrete probability distributions.

**Key Features**:
- Efficient O(1) sampling from discrete distributions
- Weighted sampling support
- GPU-based implementation
- Non-normalized weights supported
- Random number generator integration

**Core Methods**:

**Constructor**:
- [`AliasTable(ref<Device> pDevice, std::vector<float> weights, std::mt19937& rng)`](Source/Falcor/Utils/Sampling/AliasTable.h:50) - Create alias table
  - Parameters:
    - `pDevice` - GPU device
    - `weights` - Weights for each entry (proportional to desired probability)
    - `rng` - Random number generator for table creation
  - Weights don't need to be normalized (sum to 1)

**Shader Binding**:
- [`bindShaderData(const ShaderVar& var) const`](Source/Falcor/Utils/Sampling/AliasTable.h:56) - Bind alias table data to shader var
  - Parameters:
    - `var` - Shader variable to set data into
  - Binds alias table buffers to shader

**Queries**:
- [`getCount() const`](Source/Falcor/Utils/Sampling/AliasTable.h:61) - Get number of weights in table
  - Returns: Number of items

- [`getWeightSum() const`](Source/Falcor/Utils/Sampling/AliasTable.h:66) - Get total sum of all weights
  - Returns: Total weight sum

**Data Structures**:

**AliasTable Class**:
- [`mCount`](Source/Falcor/Utils/Sampling/AliasTable.h:78) - Number of items in alias table
- [`mWeightSum`](Source/Falcor/Utils/Sampling/AliasTable.h:79) - Total weight of all elements
- [`mpItems`](Source/Falcor/Utils/Sampling/AliasTable.h:80) - Buffer containing table items
- [`mpWeights`](Source/Falcor/Utils/Sampling/AliasTable.h:81) - Buffer containing item weights

**Item Structure** (internal):
- [`threshold`](Source/Falcor/Utils/Sampling/AliasTable.h:72) - If rand() < threshold, pick indexB (else pick indexA)
- [`indexA`](Source/Falcor/Utils/Sampling/AliasTable.h:73) - Redirect index for uniform sampling
- [`indexB`](Source/Falcor/Utils/Sampling/AliasTable.h:74) - Original/permutation index, sampled uniformly in [0...mCount-1]
- [`_pad`](Source/Falcor/Utils/Sampling/AliasTable.h:75) - Padding for alignment

**Technical Details**:

**Alias Method**:
- Preprocessing: Build alias table from weights
- Sampling: O(1) per sample
- Two-stage lookup: threshold comparison + uniform sampling
- Supports non-normalized weights

**Table Construction**:
- For each item: compute cumulative weight
- Split items into two groups based on weight
- Create threshold and index mappings
- Use random number generator for initialization

**Sampling Algorithm**:
1. Generate random number in [0, 1)
2. Compare with threshold
3. If below threshold: sample from group A (indexA)
4. If above threshold: sample from group B (indexB)
5. Return sampled index

**GPU Implementation**:
- Buffers for items and weights
- Shader-side sampling
- Efficient memory access
- Parallel sampling support

**Use Cases**:
- Light source sampling
- Material sampling
- Discrete probability distributions
- Importance sampling
- Weighted random selection

### LowDiscrepancy Sequences

**Files**:
- [`HammersleySequence.slang`](Source/Falcor/Utils/Sampling/LowDiscrepancy/HammersleySequence.slang:1) - Hammersley sequence shader

**Purpose**: Low-discrepancy sequences for quasi-random sampling.

**Key Features**:
- Hammersley sequence implementation
- Low discrepancy for uniform coverage
- GPU-based generation
- Efficient sampling patterns

**Technical Details**:

**Hammersley Sequence**:
- Quasi-random sequence
- Low discrepancy property
- Better uniform coverage than random sampling
- Deterministic based on sequence index

**Use Cases**:
- Monte Carlo integration
- Quasi-Monte Carlo methods
- Importance sampling
- Path tracing
- Light transport

### Pseudorandom Number Generators

**Files**:
- [`Xorshift32.slang`](Source/Falcor/Utils/Sampling/Pseudorandom/Xorshift32.slang:1) - XOR shift PRNG shader
- [`Xoshiro.slang`](Source/Falcor/Utils/Sampling/Pseudorandom/Xoshiro.slang:1) - Xoshiro PRNG shader

**Purpose**: Pseudorandom number generators for GPU-based sampling.

**Key Features**:
- GPU-based PRNG implementations
- Fast random number generation
- State management
- Multiple generator types

**Xorshift32**:
- XOR shift algorithm
- 32-bit state
- Fast and simple
- Good statistical properties

**Xoshiro**:
- Xoshiro family PRNG
- Better statistical properties
- Multiple variants available
- State-based generation

**Technical Details**:

**Xorshift32 Algorithm**:
- State: 32-bit integer
- XOR with shifted state
- Fast and simple
- Good for basic random sampling

**Xoshiro Algorithm**:
- State: Multiple integers
- XOR and shift operations
- Better statistical quality
- Longer period than XOR shift

**Use Cases**:
- Monte Carlo sampling
- Path tracing
- Light sampling
- Material sampling
- Stochastic rendering

### Uniform Sample Generators

**Files**:
- [`UniformSampleGenerator.slang`](Source/Falcor/Utils/Sampling/UniformSampleGenerator.slang:1) - Uniform sample generator shader
- [`TinyUniformSampleGenerator.slang`](Source/Falcor/Utils/Sampling/TinyUniformSampleGenerator.slang:1) - Tiny uniform sample generator shader

**Purpose**: Uniform sample generators for GPU-based sampling.

**Key Features**:
- GPU-based uniform sampling
- Multiple implementations
- Efficient random number generation
- State management

**UniformSampleGenerator**:
- Standard uniform sampling
- Good statistical properties
- Full-precision state

**TinyUniformSampleGenerator**:
- Compact implementation
- Reduced memory usage
- Slightly different statistical properties

**Technical Details**:

**Uniform Sampling**:
- Pseudorandom number generation
- Uniform distribution in [0, 1]
- GPU-based implementation
- State management for reproducibility

**Implementation Variants**:
- Full-precision: Better statistical properties
- Tiny: Reduced memory, different properties

**Use Cases**:
- Monte Carlo integration
- Path tracing
- Light sampling
- Material sampling
- Stochastic rendering

## Architecture Patterns

### Interface Pattern
- Abstract base class
- Factory function for type creation
- Virtual methods for polymorphism
- Type registration system

### GPU-Based Sampling
- Shader-side sampling
- Buffer-based data storage
- Efficient memory access
- Parallel sampling support

### Alias Method Pattern
- Preprocessing for O(1) sampling
- Two-stage lookup
- Weighted distribution support
- Non-normalized weights

## Technical Details

### Sample Generator Interface

**Shader Integration**:
- Import SampleGeneratorInterface.slang
- Use getDefines() for macros
- Bind shader data with bindShaderData()
- Frame management for per-frame setup

**Type System**:
- Type enumeration in SampleGeneratorType.slangh
- Factory function for type creation
- UI dropdown list integration

### Alias Method Implementation

**Table Construction**:
- O(n) preprocessing
- Cumulative weight computation
- Two-group partitioning
- Threshold and index mapping

**Sampling Algorithm**:
- Single random number generation
- Threshold comparison
- Uniform sampling from selected group
- O(1) per sample

**GPU Implementation**:
- Buffers for items and weights
- Shader-side sampling
- Efficient memory layout
- Parallel sampling support

### Low-Discrepancy Sequences

**Hammersley Sequence**:
- Quasi-random property
- Low discrepancy
- Deterministic generation
- Better coverage than random

**Use Cases**:
- Quasi-Monte Carlo integration
- Importance sampling
- Light transport simulation
- Path tracing

### Pseudorandom Generators

**Xorshift32**:
- 32-bit state
- XOR and shift operations
- Fast and simple
- Good basic properties

**Xoshiro**:
- Multiple integer state
- XOR and shift operations
- Better statistical quality
- Longer period

**Use Cases**:
- Monte Carlo sampling
- Path tracing
- Light sampling
- Material sampling
- Stochastic rendering

## Progress Log

- **2026-01-07T19:15:25Z**: Sampling sub-module analysis completed. Analyzed SampleGenerator, AliasTable, and related sampling utilities. Documented sample generator interface with factory pattern and shader integration, alias method for O(1) discrete distribution sampling, low-discrepancy sequences (Hammersley), pseudorandom number generators (Xorshift32, Xoshiro), and uniform sample generators. Created comprehensive technical specification with detailed code patterns, data structures, and use cases.

## Next Steps

Proceed to analyze UI sub-module to understand user interface utilities (Font, Gui).
