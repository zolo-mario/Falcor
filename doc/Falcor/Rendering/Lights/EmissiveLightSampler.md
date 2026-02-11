# EmissiveLightSampler - Base Class for Emissive Light Samplers

## Module State Machine

**Status**: Complete

## Dependency Graph

### Direct Sub-modules

None (Base component)

### External Dependencies

- **Core/Program** - Shader program management (Program, DefineList)
- **Scene/Lights** - Light collection interfaces (ILightCollection)
- **Utils/Sampling** - Sample generator interface (ISampleGenerator)
- **Utils/Math** - Mathematical utilities (MathConstants, MathHelpers)
- **Utils/Scripting** - Python bindings

## Module Overview

EmissiveLightSampler provides the base class and interface for all emissive light sampler implementations in Falcor. It defines a common interface that makes different light sampling strategies (uniform, power-based, BVH-based) interchangeable at runtime. The component includes CPU-side base class, shader-side interface, utility functions for triangle sampling, and type definitions for sampler selection.

## Component Specifications

### EmissiveLightSampler Class

**Files**:
- [`EmissiveLightSampler.h`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:1) - Base class header (86 lines)
- [`EmissiveLightSampler.cpp`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.cpp:1) - Base class implementation (58 lines)

**Purpose**: Base class for emissive light sampler implementations.

**Key Features**:
- Abstract base class for all emissive light samplers
- Common interface for runtime sampler selection
- Light collection integration with update tracking
- Shader define generation for type selection
- Update flag tracking for efficient recompilation
- Virtual methods for extensibility

### EmissiveLightSamplerType Enum

**File**: [`EmissiveLightSamplerType.slangh`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerType.slangh:1)

**Purpose**: Enumerates available emissive light sampler types.

**Values**:
- `Uniform = 0` - Uniform emissive light sampling
- `LightBVH = 1` - BVH-based emissive light sampling
- `Power = 2` - Power-based emissive light sampling
- `Null = 0xff` - Null/no-op sampler

**Features**:
- Shared between CPU and GPU code
- Enum info for reflection (string names)
- Static asserts for value validation
- Compile-time constants for shader specialization

### EmissiveLightSampler.slang

**File**: [`EmissiveLightSampler.slang`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.slang:1)

**Purpose**: Shader-side emissive light sampler interface and type selection.

**Key Features**:
- Conditional compilation for sampler type selection
- Null sampler implementation for fallback
- Type typedef based on preprocessor define
- Compile-time error for invalid types

**Conditional Compilation**:
```cpp
#if _EMISSIVE_LIGHT_SAMPLER_TYPE == EMISSIVE_LIGHT_SAMPLER_UNIFORM
    import Rendering.Lights.EmissiveUniformSampler;
    typedef EmissiveUniformSampler EmissiveLightSampler;
#elif _EMISSIVE_LIGHT_SAMPLER_TYPE == EMISSIVE_LIGHT_SAMPLER_LIGHT_BVH
    import Rendering.Lights.LightBVHSampler;
    typedef LightBVHSampler EmissiveLightSampler;
#elif _EMISSIVE_LIGHT_SAMPLER_TYPE == EMISSIVE_LIGHT_SAMPLER_POWER
    import Rendering.Lights.EmissivePowerSampler;
    typedef EmissivePowerSampler EmissiveLightSampler;
#elif _EMISSIVE_LIGHT_SAMPLER_TYPE == EMISSIVE_LIGHT_SAMPLER_NULL
    typedef NullEmissiveSampler EmissiveLightSampler;
#else
    #error _EMISSIVE_LIGHT_SAMPLER_TYPE is not set to a supported type.
#endif
```

### NullEmissiveSampler Struct

**File**: [`EmissiveLightSampler.slang`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.slang:64)

**Purpose**: Null sampler implementation for cases where emissive sampling is not needed.

**Methods**:
- `sampleLight()` - Returns false, invalid sample
- `evalTriangleSelectionPdf()` - Returns 0.0
- `evalPdf()` - Returns 0.0

**Use Cases**:
- Scenes without emissive lights
- Testing and debugging
- Fallback when sampler type is not defined

### EmissiveLightSamplerInterface.slang

**File**: [`EmissiveLightSamplerInterface.slang`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerInterface.slang:1)

**Purpose**: Slang interface and data structures for emissive light samplers.

**Data Structures**:

**TriangleLightSample** (from [`TriangleLightSample`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerInterface.slang:36)):
```cpp
struct TriangleLightSample {
    uint    triangleIndex;      // Index of sampled triangle
    float3  posW;               // Sampled point on light source in world space
    float3  normalW;            // Normal at sampled point on light source in world space
    float3  dir;                // Normalized direction from shading point to sampled point
    float   distance;           // Distance from shading point to sampled point
    float3  Le;                 // Emitted radiance (zero if back-facing or invalid)
    float   pdf;                // Probability density [0,inf], 0.0 indicates invalid
    float2  uv;                 // Barycentric coords over triangle
};
```

**TriangleLightHit** (from [`TriangleLightHit`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerInterface.slang:51)):
```cpp
struct TriangleLightHit {
    uint    triangleIndex;      // Index of triangle
    float3  posW;               // Sampled point on triangle in world space
    float3  normalW;            // Face normal in world space (flipped for back-facing hits)
};
```

**IEmissiveLightSampler Interface** (from [`IEmissiveLightSampler`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerInterface.slang:60)):

**Methods**:
- `sampleLight<S : ISampleGenerator>(const float3 posW, const float3 normalW, const bool upperHemisphere, inout S sg, out TriangleLightSample ls)` - Draw single light sample
- `evalTriangleSelectionPdf(const float3 posW, const float3 normalW, const bool upperHemisphere, const uint triangleIndex)` - Evaluate PDF for triangle selection
- `evalPdf(const float3 posW, const float3 normalW, const bool upperHemisphere, const TriangleLightHit hit)` - Evaluate PDF at hit point

### EmissiveLightSamplerHelpers.slang

**File**: [`EmissiveLightSamplerHelpers.slang`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:1)

**Purpose**: Shared utility functions for emissive light sampler implementations.

**Key Features**:
- Triangle uniform sampling by area
- PDF evaluation for triangle selection
- PDF evaluation at hit points
- Back-facing rejection
- Light profile integration (DEMO21)
- Material system integration

**Functions**:

**sampleTriangle()** (from [`sampleTriangle()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:54)):
```cpp
bool sampleTriangle(const float3 posW, const uint triangleIndex, const float2 u, out TriangleLightSample ls)
```
- Samples triangle uniformly by area
- Computes barycentric coordinates
- Calculates light vector and distance
- Gets triangle normal from LightCollection
- Rejects back-facing triangles (cosTheta <= 0)
- Evaluates emitted radiance from material system
- Applies light profile if available (DEMO21)
- Computes probability density with respect to solid angle
- Formula: `pdf = distSqr / (cosTheta * tri.area)`
- Returns true if sample valid, false otherwise

**evalTrianglePdf()** (from [`evalTrianglePdf()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:106)):
```cpp
float evalTrianglePdf(const float3 posW, const TriangleLightHit hit)
```
- Evaluates PDF for a given triangle hit
- Computes light vector and squared distance
- Calculates cosine of angle between light normal and light vector
- Rejects back-facing hits (cosTheta <= 0)
- Computes probability density with respect to solid angle
- Formula: `pdf = distSqr / (cosTheta * tri.area)`
- Returns 0.0 for invalid samples

## Technical Details

### CPU-Side Base Class

**EmissiveLightSampler Class Members**:

**Protected Members**:
- [`mType`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:80) - Sampler type (EmissiveLightSamplerType)
- [`mpDevice`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:81) - Device reference
- [`mpLightCollection`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:82) - Light collection reference
- [`mUpdateFlagsConnection`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:83) - Signal connection for update tracking
- [`mLightCollectionUpdateFlags`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:84) - Cached update flags

**Virtual Methods**:
- [`update()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:54) - Update sampler for current frame (default returns false)
- [`getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:59) - Return shader defines
- [`bindShaderData()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:63) - Bind sampler data to shader (default empty)
- [`renderUI()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:68) - Render GUI (default returns false)
- [`getType()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:73) - Return sampler type

### Constructor

**EmissiveLightSampler()** (from [`EmissiveLightSampler::EmissiveLightSampler()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.cpp:34)):
```cpp
EmissiveLightSampler(EmissiveLightSamplerType type, ref<ILightCollection> pLightCollection)
    : mType(type)
    , mpDevice(pLightCollection->getDevice())
{
    setLightCollection(std::move(pLightCollection));
}
```
- Initializes sampler type
- Stores device reference
- Sets light collection with signal connection

### Light Collection Management

**setLightCollection()** (from [`EmissiveLightSampler::setLightCollection()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.cpp:41)):
```cpp
void EmissiveLightSampler::setLightCollection(ref<ILightCollection> pLightCollection)
{
    if (mpLightCollection == pLightCollection)
        return;
    
    mUpdateFlagsConnection.reset();
    
    mpLightCollection = std::move(pLightCollection);
    
    if (mpLightCollection)
        mUpdateFlagsConnection = mpLightCollection->getUpdateFlagsSignal().connect([&](ILightCollection::UpdateFlags flags) { 
            mLightCollectionUpdateFlags |= flags; 
        });
}
```
- Avoids unnecessary updates (same collection)
- Resets update flags
- Moves new collection reference
- Connects to update flags signal
- Accumulates update flags in member variable

### Shader Define Generation

**getDefines()** (from [`EmissiveLightSampler::getDefines()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.cpp:54)):
```cpp
DefineList EmissiveLightSampler::getDefines() const
{
    return {"_EMISSIVE_LIGHT_SAMPLER_TYPE", std::to_string((uint32_t)mType)};
}
```
- Returns single define for sampler type
- Converts enum to string
- Used for shader specialization

### GPU-Side Interface

**IEmissiveLightSampler Interface**:

**sampleLight()**:
- Template method with sample generator
- Returns TriangleLightSample with all required fields
- Returns boolean indicating sample validity
- Used by path tracing shaders

**evalTriangleSelectionPdf()**:
- Evaluates PDF for selecting a specific triangle
- Used by MIS (Multiple Importance Sampling)
- Returns probability density [0,inf]

**evalPdf()**:
- Evaluates PDF at a hit point on emissive triangle
- Accounts for distance, angle, and triangle area
- Used for PDF-based sampling strategies

### Triangle Sampling Algorithm

**Uniform Area Sampling** (from [`sampleTriangle()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:54)):
1. Sample barycentric coordinates uniformly: `u = sample_triangle(u)`
2. Compute world position: `posW = tri.getPosition(barycentrics)`
3. Compute light vector: `toLight = posW - shadingPoint`
4. Compute squared distance: `distSqr = max(FLT_MIN, dot(toLight, toLight))`
5. Compute distance: `distance = sqrt(distSqr)`
6. Compute direction: `dir = toLight / distance`
7. Get triangle normal: `normalW = tri.normal`
8. Check back-facing: `cosTheta = dot(normalW, -dir)`
9. Reject if back-facing: `if (cosTheta <= 0) return false`
10. Get UV coordinates: `uv = tri.getTexCoord(barycentrics)`
11. Evaluate radiance: `Le = materials.evalEmissive(materialID, uv)`
12. Apply light profile: `Le *= materials.evalLightProfile(dot(normalW, -dir))`
13. Compute PDF: `pdf = distSqr / max(FLT_MIN, cosTheta * tri.area)`
14. Return true

### PDF Evaluation

**Triangle Selection PDF** (from [`evalTrianglePdf()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:106)):
- Computes probability density for selecting a triangle
- Accounts for squared distance and triangle area
- Accounts for angle between normal and light direction
- Rejects back-facing hits (pdf = 0.0)
- Formula: `pdf = distSqr / (cosTheta * tri.area)`

**Hit Point PDF** (from [`evalPdf()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:106)):
- Same formula as triangle selection PDF
- Uses hit data instead of triangle index
- Flips normal for back-facing hits
- Returns 0.0 for invalid samples

### Back-Facing Rejection

**Purpose**: Prevent sampling of back-facing triangles.

**Implementation**:
```cpp
float cosTheta = dot(normalW, -dir);
if (cosTheta <= 0.f) return false;
```
- Computes cosine of angle between normal and light direction
- Rejects if angle is >= 90 degrees
- Ensures only front-facing triangles are sampled
- Critical for correct light sampling

### Light Profile Integration

**DEMO21 Support** (from [`sampleTriangle()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:82)):
```cpp
if (gScene.materials.hasLightProfile(tri.materialID))
{
    ls.Le *= gScene.materials.evalLightProfile(dot(ls.normalW, -ls.dir));
}
```
- Checks if material has light profile
- Evaluates light profile based on angle
- Multiplies radiance by profile factor
- Supports DEMO21 light profiles

### Material System Integration

**Radiance Evaluation** (from [`sampleTriangle()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:79)):
```cpp
float2 uv = tri.getTexCoord(barycentrics);
ls.Le = gScene.materials.evalEmissive(tri.materialID, uv);
```
- Gets triangle UV coordinates
- Evaluates emissive radiance from material system
- Supports texture-based emissive materials
- Integrates with Falcor's material system

### Light Collection Access

**Triangle Access** (from [`sampleTriangle()`](Source/Falcor/Rendering/Lights/EmissiveLightSamplerHelpers.slang:58)):
```cpp
const EmissiveTriangle tri = gScene.lightCollection.getTriangle(triangleIndex);
```
- Accesses triangle data from global LightCollection
- Provides position, normal, area, material ID
- Efficient GPU-side access to light data

### Type Selection Strategy

**Preprocessor-Based Selection**:
- Define `_EMISSIVE_LIGHT_SAMPLER_TYPE` before including EmissiveLightSampler.slang
- Value determines which implementation is used
- Compile-time selection (zero runtime overhead)
- Error if invalid type is specified

**Null Sampler Fallback**:
- Defined if `_EMISSIVE_LIGHT_SAMPLER_TYPE` is not defined
- Prevents compiler warnings
- Provides valid but no-op sampler
- Useful for testing and debugging

### Update Flag Tracking

**Signal Connection** (from [`setLightCollection()`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.cpp:50)):
```cpp
mUpdateFlagsConnection = mpLightCollection->getUpdateFlagsSignal().connect([&](ILightCollection::UpdateFlags flags) { 
    mLightCollectionUpdateFlags |= flags; 
});
```
- Connects to LightCollection's update signal
- Accumulates update flags in member variable
- Enables efficient recompilation detection

## Architecture Patterns

### Abstract Base Class Pattern
- [`EmissiveLightSampler`](Source/Falcor/Rendering/Lights/EmissiveLightSampler.h:44) defines common interface
- Virtual methods for extensibility
- Runtime polymorphism for sampler selection
- Template methods for shader integration

### Strategy Pattern
- Multiple implementations (Uniform, Power, LightBVH)
- Common interface enables interchangeability
- Runtime selection via type enum
- Compile-time specialization via defines

### Template Pattern
- `sampleLight<S : ISampleGenerator>` with sample generator
- Generic sample generation
- Type-safe sample generation
- Integrates with Falcor's sampling system

### Signal/Slot Pattern
- Connection to LightCollection update signal
- Automatic flag accumulation
- Efficient change detection
- Decouples update tracking from main logic

### Conditional Compilation Pattern
- Preprocessor-based type selection
- Zero runtime overhead for type selection
- Compile-time error checking
- Null sampler fallback for safety

## Code Patterns

### Virtual Interface
```cpp
virtual bool update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection) { return false; }
virtual DefineList getDefines() const;
virtual void bindShaderData(const ShaderVar& var) const {}
virtual bool renderUI(Gui::Widgets& widget) { return false; }
```
- Pure virtual interface
- Default implementations provided
- Derived classes override as needed

### Signal Connection
```cpp
mUpdateFlagsConnection = mpLightCollection->getUpdateFlagsSignal().connect([&](ILightCollection::UpdateFlags flags) { 
    mLightCollectionUpdateFlags |= flags; 
});
```
- Lambda-based callback
- Accumulates flags
- Automatic disconnection on reset

### Define Generation
```cpp
return {"_EMISSIVE_LIGHT_SAMPLER_TYPE", std::to_string((uint32_t)mType)};
```
- Single define for type selection
- Enum to string conversion
- Used for shader compilation

### Conditional Compilation
```cpp
#if _EMISSIVE_LIGHT_SAMPLER_TYPE == EMISSIVE_LIGHT_SAMPLER_UNIFORM
    import Rendering.Lights.EmissiveUniformSampler;
    typedef EmissiveUniformSampler EmissiveLightSampler;
#endif
```
- Preprocessor-based selection
- Type typedef for uniform access
- Compile-time error for invalid types

### Back-Facing Rejection
```cpp
float cosTheta = dot(normalW, -dir);
if (cosTheta <= 0.f) return false;
```
- Cosine angle check
- Early rejection
- Prevents back-facing sampling

### PDF Computation
```cpp
float denom = max(FLT_MIN, cosTheta * tri.area);
ls.pdf = distSqr / denom;
```
- Solid angle PDF formula
- Division by zero protection
- Distance squared in numerator

## Use Cases

### Base Class Usage
```cpp
// Derived class:
class EmissivePowerSampler : public EmissiveLightSampler
{
public:
    EmissivePowerSampler(ref<ILightCollection> pLightCollection)
        : EmissiveLightSampler(EmissiveLightSamplerType::Power, pLightCollection)
    {
        // Initialize power-specific data
    }
    
    bool update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection) override
    {
        // Update power-based sampling data
        return true;
    }
};
```

### Shader Integration
```cpp
// In path tracing shader:
#include "Rendering/Lights/EmissiveLightSampler.slang"

// Use sampler:
TriangleLightSample ls;
if (emissiveSampler.sampleLight(posW, normalW, upperHemisphere, sg, ls))
{
    // Use sample for shading
    float3 radiance = ls.Le;
    float pdf = ls.pdf;
}
```

### Runtime Sampler Selection
```cpp
// Create sampler based on configuration:
EmissiveLightSamplerType type = getConfiguredType();
ref<EmissiveLightSampler> sampler;

switch (type)
{
    case EmissiveLightSamplerType::Uniform:
        sampler = EmissiveUniformSampler::create(pLightCollection);
        break;
    case EmissiveLightSamplerType::Power:
        sampler = EmissivePowerSampler::create(pLightCollection);
        break;
    case EmissiveLightSamplerType::LightBVH:
        sampler = LightBVHSampler::create(pLightCollection);
        break;
}

// Use sampler:
sampler->update(renderContext, lightCollection);
sampler->prepareProgram(program, var);
```

### PDF Evaluation for MIS
```cpp
// In path tracing shader:
TriangleLightHit hit;
hit.triangleIndex = triangleIndex;
hit.posW = intersectionPoint;
hit.normalW = triangleNormal;

float pdf = emissiveSampler.evalPdf(posW, normalW, upperHemisphere, hit);
```

### Null Sampler Usage
```cpp
// For scenes without emissive lights:
#define EMISSIVE_LIGHT_SAMPLER_TYPE EMISSIVE_LIGHT_SAMPLER_NULL
#include "Rendering/Lights/EmissiveLightSampler.slang"

// Sampler will return false for all sampleLight() calls
```

## Integration Points

### Light Collection Integration
- Stores reference to ILightCollection
- Connects to update flags signal
- Accesses triangle data via global gScene.lightCollection
- Tracks light collection changes

### Material System Integration
- Evaluates emissive radiance via gScene.materials
- Supports texture-based materials
- Integrates light profiles (DEMO21)
- Uses UV coordinates for texture lookup

### Sampling System Integration
- Implements IEmissiveLightSampler interface
- Uses ISampleGenerator for random numbers
- Returns TriangleLightSample structure
- Integrates with Falcor's sampling framework

### Shader Compilation Integration
- Provides _EMISSIVE_LIGHT_SAMPLER_TYPE define
- Enables compile-time type selection
- Imports appropriate implementation
- Type typedef for uniform access

## Performance Considerations

### CPU-Side Overhead
- Virtual function calls for update/renderUI
- Signal connection overhead
- Minimal per-frame overhead
- Efficient flag accumulation

### GPU-Side Overhead
- Triangle data access from global LightCollection
- Material system calls for radiance evaluation
- Light profile evaluation if available
- Cosine computation for back-facing check
- PDF computation with division

### Memory Usage
- Base class: minimal (type, device ref, collection ref, signal connection)
- Derived classes: additional data structures
- GPU: LightCollection holds all triangle data
- No per-sampler GPU buffers in base class

### Optimization Opportunities
- Compile-time type selection (zero runtime overhead)
- Back-facing rejection before expensive operations
- Efficient PDF computation
- Material system caching (handled by material system)
- Light profile evaluation only when available

## Limitations

### Feature Limitations
- Base class provides no sampling implementation
- Derived classes must implement all virtual methods
- No built-in caching in base class
- Limited to triangle-based emissive lights

### API Limitations
- Must call setLightCollection in constructor
- Update flags must be checked by derived classes
- Shader define must be set before including EmissiveLightSampler.slang
- Null sampler returns false for all samples

### Performance Limitations
- Virtual function call overhead
- Signal connection overhead
- Material system call overhead for each sample
- Light profile evaluation overhead (if enabled)

### Platform Limitations
- Requires shader support for conditional compilation
- Requires LightCollection global access
- Requires material system integration
- Requires sample generator support

## Best Practices

### When to Use
- Implementing new emissive light samplers
- Creating interchangeable sampling strategies
- Supporting runtime sampler selection
- Integrating with LightCollection
- Supporting material-based emissive lights

### Usage Patterns
- Always call base class constructor
- Override all virtual methods
- Use getType() for type identification
- Use getDefines() for shader compilation
- Track update flags for efficient recompilation

### Performance Tips
- Minimize virtual function calls in hot paths
- Cache material data if possible
- Use back-facing rejection early
- Optimize PDF computation
- Avoid unnecessary material system calls

### Error Handling
- Check return value of update()
- Handle invalid samples (pdf == 0.0)
- Validate triangle indices
- Handle back-facing triangles gracefully
- Check for LightCollection availability

### Shader Integration
- Set _EMISSIVE_LIGHT_SAMPLER_TYPE before including
- Use typedef for uniform access
- Check sample validity before using
- Use pdf for MIS calculations
- Handle null sampler case

## Implementation Notes

### Enum Value Mapping
- CPU enum: EmissiveLightSamplerType (class enum)
- GPU defines: EMISSIVE_LIGHT_SAMPLER_* constants
- Static asserts ensure consistency
- Compile-time type selection

### Signal Connection Management
- Connection stored in mUpdateFlagsConnection
- Automatically disconnected on reset
- Accumulates flags in mLightCollectionUpdateFlags
- Derived classes check this for changes

### Null Sampler Workaround
- Prevents compiler warnings for undefined types
- Provides valid but no-op implementation
- Dummy variable prevents Slang optimization
- Safe fallback for testing

### PDF Formula Derivation
- Solid angle PDF: `pdf = dist^2 / (cos(theta) * area)`
- Distance squared in numerator
- Cosine times area in denominator
- Accounts for projected area at shading point
- Goes to infinity as distance increases

### Back-Facing Check
- Computes: `cosTheta = dot(normalW, -dir)`
- Rejects if: `cosTheta <= 0`
- Flips normal for hit point evaluation
- Ensures front-facing sampling

### Light Profile Integration
- Checks: `hasLightProfile(materialID)`
- Evaluates: `evalLightProfile(dot(normalW, -dir))`
- Multiplies: `Le *= profileFactor`
- Supports DEMO21 light profiles
- Only evaluated if profile available

### Material System Integration
- Accesses: `gScene.materials.evalEmissive(materialID, uv)`
- Uses triangle UV coordinates
- Supports texture-based emissive materials
- Integrated with Falcor's material system

## Future Enhancements

### Additional Sampler Types
- Stratified sampling
- Hierarchical sampling
- Adaptive sampling
- Temporal sampling
- Spatial sampling

### Caching Strategies
- Per-frame triangle caching
- Material data caching
- PDF caching
- Light profile caching

### Performance Improvements
- Reduced virtual function overhead
- Optimized PDF computation
- Faster material system access
- SIMD-friendly triangle sampling

### API Extensions
- Per-sampler configuration
- Sampler blending
- Dynamic sampler switching
- Sampler statistics
- Debug visualization

### Debug Features
- Per-triangle statistics
- PDF visualization
- Sampling distribution visualization
- Back-facing rejection statistics
- Performance profiling

### Integration Enhancements
- Automatic sampler selection
- Performance-based selection
- Scene-based selection
- Hybrid sampling strategies
- Multi-resolution sampling
