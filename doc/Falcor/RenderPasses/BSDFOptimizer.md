# BSDFOptimizer - BSDF Parameter Optimization

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **BSDFOptimizer.cs.slang** - Optimizer shader with differentiable rendering
- [x] **BSDFOptimizerHelpers.slang** - Helper functions for BSDF slice geometry
- [x] **BSDFOptimizerParams.slang** - Parameter structures shared between host and device
- [x] **BSDFViewer.cs.slang** - Viewer shader for BSDF visualization

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, Texture, Formats)
- **Core/Object** - Base object class
- **Core/Program** - Shader program management
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection)
- **Scene** - Scene system (Scene, Material, MaterialID)
- **Rendering/Materials** - Material system (BSDFConfig)
- **DiffRendering** - Differentiable rendering (SceneGradients)
- **Utils/Sampling** - Sampling utilities (SampleGenerator)
- **Utils/UI** - UI utilities (Gui)

## Module Overview

BSDFOptimizer implements BSDF (Bidirectional Scattering Distribution Function) parameter optimization using differentiable rendering. The pass optimizes material parameters to match a reference material by computing gradients of a loss function with respect to BSDF parameters. It uses the Adam optimizer for gradient-based optimization and provides real-time visualization of the optimization process with three viewports: initial material, absolute difference, and reference material. The pass supports various material types with configurable learning rates.

## Component Specifications

### BSDFOptimizer Class

**Files**:
- [`BSDFOptimizer.h`](Source/RenderPasses/BSDFOptimizer/BSDFOptimizer.h:1) - BSDF optimizer header
- [`BSDFOptimizer.cpp`](Source/RenderPasses/BSDFOptimizer/BSDFOptimizer.cpp:1) - BSDF optimizer implementation

**Purpose**: Render pass for optimizing BSDF parameters using differentiable rendering.

**Key Features**:
- Differentiable BSDF evaluation
- Adam optimizer for gradient-based optimization
- Real-time BSDF visualization
- Three-viewport display (initial, difference, reference)
- Configurable learning rates per material type
- Automatic stopping when error is small enough
- Material parameter serialization/deserialization
- Integration with SceneGradients for gradient computation

**Public Interface**:

```cpp
class BSDFOptimizer : public RenderPass
{
public:
    static ref<BSDFOptimizer> create(ref<Device> pDevice, const Properties& props);
    
    // RenderPass overrides
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override;
    
    // Scripting functions
    static void registerBindings(pybind11::module& m);
    uint32_t getInitMaterialID() const;
    uint32_t getRefMaterialID() const;
    uint32_t getBSDFSliceResolution() const;
    void setBSDFSliceResolution(uint32_t reso);
    ref<Buffer> computeBSDFGrads();
};
```

**Private Members**:

```cpp
private:
    void parseProperties(const Properties& props);
    void initOptimization();
    void executeOptimizerPass(RenderContext* pRenderContext);
    void step(RenderContext* pRenderContext);
    void executeViewerPass(RenderContext* pRenderContext, const RenderData& renderData);
    
    struct AdamOptimizer;
    
    ref<Scene> mpScene;
    std::unique_ptr<SceneGradients> mpSceneGradients;
    SerializedMaterialParams mInitBSDFParams;
    SerializedMaterialParams mRefBSDFParams;
    SerializedMaterialParams mCurBSDFParams;
    SerializedMaterialParams mBSDFGrads;
    AdamOptimizer mAdam;
    BSDFOptimizerParams mParams;
    ref<SampleGenerator> mpSampleGenerator;
    bool mOptionsChanged = false;
    ref<Fence> mpFence;
    ref<ComputePass> mpOptimizerPass;
    ref<ComputePass> mpViewerPass;
    Gui::DropdownList mMaterialList;
    bool mRunOptimization = false;
};
```

### AdamOptimizer

**Purpose**: Adam optimization algorithm implementation for gradient-based BSDF parameter optimization.

**Key Features**:
- Adaptive learning rates
- Momentum and RMSprop combination
- Bias correction for first and second moments
- Per-parameter learning rates
- Automatic step count tracking

**Algorithm**:

```cpp
struct AdamOptimizer
{
    std::vector<float> lr;  // Learning rates per parameter
    float beta1;             // Exponential decay rate for first moment
    float beta2;             // Exponential decay rate for second moment
    float epsilon;            // Small constant for numerical stability
    int steps;               // Step count
    
    std::vector<float> m;    // First moment (momentum)
    std::vector<float> v;    // Second moment (RMSprop)
    
    void step(fstd::span<float> dx, fstd::span<float> x);
};
```

**Update Rule**:

```cpp
for (size_t i = 0; i < dx.size(); i++)
{
    if (lr[i] == 0.f)
        continue; // Skip parameters with zero learning rate
    m[i] = beta1 * m[i] + (1.f - beta1) * dx[i];
    v[i] = beta2 * v[i] + (1.f - beta2) * dx[i] * dx[i];
    float mHat = m[i] / (1.f - std::pow(beta1, steps));
    float vHat = v[i] / (1.f - std::pow(beta2, steps));
    float delta = lr[i] * mHat / (std::sqrt(vHat) + epsilon);
    x[i] -= delta;
}
steps++;
```

### BSDFOptimizer.cs.slang Shader

**File**:
- [`BSDFOptimizer.cs.slang`](Source/RenderPasses/BSDFOptimizer/BSDFOptimizer.cs.slang:1) - Optimizer shader

**Purpose**: GPU-side differentiable BSDF evaluation and loss computation.

**Key Features**:
- Differentiable BSDF evaluation
- Loss function computation (L2 loss)
- Automatic differentiation
- BSDF slice visualization
- Cosine term removal

**Entry Point**:

```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    execute(dispatchThreadID.xy);
}
```

**Differentiable Functions**:

```hlsl
[Differentiable]
SurfaceData prepareShadingData(const VertexData v, const float3 viewDir, const uint materialID)
{
    SurfaceData data = {};
    data.sd = no_diff gScene.materials.prepareShadingData(v, materialID, viewDir);
    return data;
}

[Differentiable]
IMaterialInstance getDiffMaterialInstance(out DiffMaterialData diffData, const ShadingData sd)
{
    let lod = ExplicitLodTextureSampler(0.f);
    let material = gScene.materials.getMaterial(sd.materialID);
    let mi = material.setupDiffMaterialInstance(diffData, gScene.materials, sd, lod);
    return mi;
}

[Differentiable]
float3 evalBSDFSlice(bool isRef, const uint2 pixel)
{
    float2 uv = no_diff getViewportCoord(pixel, float2(0.f), params.viewPortScale);
    SampleGenerator sg = SampleGenerator(pixel, params.frameCount);
    
    VertexData v;
    float3 viewDir;
    float3 lightDir = no_diff calculateSliceGeometry(uv, v, viewDir);
    
    SurfaceData data = prepareShadingData(v, viewDir, isRef ? params.refMaterialID : params.initMaterialID);
    data.wo = detach(lightDir);
    data.sd.materialGradOffset = 0;
    data.sd.threadID = 0;
    
    DiffMaterialData diffData = DiffMaterialData();
    let mi = getDiffMaterialInstance(diffData, data.sd);
    float3 f = isRef ? no_diff mi.eval(data.sd, data.wo, sg) : mi.evalAD(diffData, data.sd, data.wo, sg);
    
    float NdotL = abs(dot(data.sd.frame.N, data.wo));
    f = NdotL > 0.f ? f / NdotL : float3(0);
    
    return f;
}

[Differentiable]
float3 computeLoss(const uint2 pixel)
{
    float3 fRef = evalBSDFSlice(true, pixel);
    float3 fCur = evalBSDFSlice(false, pixel);
    
    float3 diff = fCur - fRef;
    return 0.5 * diff * diff;  // L2 loss
}
```

### BSDFOptimizerHelpers.slang

**File**:
- [`BSDFOptimizerHelpers.slang`](Source/RenderPasses/BSDFOptimizer/BSDFOptimizerHelpers.slang:1) - Helper functions

**Purpose**: Helper functions for BSDF slice geometry calculation.

**Key Features**:
- Viewport coordinate calculation
- BSDF slice geometry computation
- Direction calculation from spherical coordinates

**Functions**:

```hlsl
float2 getViewportCoord(uint2 pixel, float2 viewportOffset, float2 viewportScale)
{
    float2 p = pixel + float2(0.5f);
    return (p - viewportOffset) * viewportScale;
}

float3 calculateSliceGeometry(float2 uv, out VertexData v, out float3 viewDir)
{
    // Setup local surface frame as T,B,N
    v.posW = float3(0, 0, 0);
    v.normalW = float3(0, 0, 1);
    v.tangentW = float4(1, 0, 0, 1);
    v.texC = float2(0, 0);
    v.faceNormalW = v.normalW;
    
    // Compute dot products based on (theta_h, theta_d) spherical coordinates
    float theta_h = uv.x * (M_PI / 2);
    float theta_d = (1.f - uv.y) * (M_PI / 2);
    
    float NdotH = cos(theta_h);
    float HdotL = cos(theta_d);
    
    // Place H vector at (0,0,1) and compute L, V mirrored about yz-plane
    float3 L = float3(sqrt(1.f - HdotL * HdotL), 0, HdotL);
    float3 V = float3(-L.x, 0.f, L.z);
    
    // Rotate L, V about x-axis by theta_h
    float cos_h = NdotH;
    float sin_h = sqrt(1 - NdotH * NdotH);
    L = float3(L.x, cos_h * L.y - sin_h * L.z, sin_h * L.y + cos_h * L.z);
    V = float3(V.x, cos_h * V.y - sin_h * V.z, sin_h * V.y + cos_h * V.z);
    
    viewDir = V;
    L = normalize(L);
    return L;
}
```

### BSDFOptimizerParams.slang

**File**:
- [`BSDFOptimizerParams.slang`](Source/RenderPasses/BSDFOptimizer/BSDFOptimizerParams.slang:1) - Parameter structures

**Purpose**: Parameter structures shared between host and device.

**Structure**:

```hlsl
struct BSDFOptimizerParams
{
    uint2 frameDim = { 0, 0 };        ///< Frame dimensions
    uint frameCount = 0;                 ///< Frame counter
    int _pad0;
    
    float2 initViewPortOffset;            ///< Initial material viewport offset
    float2 diffViewPortOffset;            ///< Difference viewport offset
    
    float2 refViewPortOffset;            ///< Reference material viewport offset
    float2 viewPortScale;                ///< Viewport scale factor
    
    uint2 bsdfTableDim = { 0, 0 };    ///< BSDF slice resolution
    uint initMaterialID = 0;                ///< Initial material ID
    uint refMaterialID = 1;                 ///< Reference material ID
};
```

### BSDFViewer.cs.slang Shader

**File**:
- [`BSDFViewer.cs.slang`](Source/RenderPasses/BSDFOptimizer/BSDFViewer.cs.slang:1) - Viewer shader

**Purpose**: GPU-side BSDF visualization for three-viewport display.

**Key Features**:
- Three-viewport rendering (initial, difference, reference)
- BSDF evaluation (non-differentiable)
- Absolute difference computation
- Viewport-based rendering

**Entry Point**:

```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    gBSDFViewer.execute(dispatchThreadID.xy);
}
```

**Execution**:

```hlsl
void execute(const uint2 pixel)
{
    if (any(pixel >= params.frameDim))
        return;
    
    uint viewportID = 0;
    float2 viewportOffset = params.initViewPortOffset;
    
    if (pixel.x >= params.refViewPortOffset.x)
    {
        viewportID = 2;
        viewportOffset = params.refViewPortOffset;
    }
    else if (pixel.x >= params.diffViewPortOffset.x)
    {
        viewportID = 1;
        viewportOffset = params.diffViewPortOffset;
    }
    
    float2 uv = getViewportCoord(pixel, viewportOffset, params.viewPortScale);
    if (any(uv < 0.f || uv >= 1.f))
    {
        output[pixel] = float4(0.f, 0.f, 0.f, 1.f);
        return;
    }
    
    let lod = ExplicitLodTextureSampler(0.f);
    SampleGenerator sg = SampleGenerator(pixel, params.frameCount);
    
    float3 fInitVal = evalBSDFSlice(uv, params.initMaterialID, lod, sg);
    float3 fRefVal = evalBSDFSlice(uv, params.refMaterialID, lod, sg);
    float3 fDiffVal = abs(fInitVal - fRefVal);
    
    if (viewportID == 0)
        output[pixel] = float4(fInitVal, 1.f);
    else if (viewportID == 1)
        output[pixel] = float4(fDiffVal, 1.f);
    else
        output[pixel] = float4(fRefVal, 1.f);
}
```

## Technical Details

### Optimization Pipeline

1. **Initialization** (`initOptimization()`):
   - Reset BSDF parameters to initial material
   - Initialize current BSDF parameters
   - Set learning rates per material type
   - Initialize Adam optimizer

2. **Optimizer Pass** (`executeOptimizerPass()`):
   - Clear gradients
   - Bind scene and gradient data
   - Execute optimizer shader
   - Aggregate gradients

3. **Step** (`step()`):
   - Read gradients from GPU
   - Update BSDF parameters using Adam optimizer
   - Deserialize parameters back to material

4. **Viewer Pass** (`executeViewerPass()`):
   - Render three viewports (initial, difference, reference)
   - Evaluate BSDFs at each pixel
   - Display absolute difference

### Learning Rate Configuration

Per-material-type learning rates:

```cpp
const std::map<MaterialType, std::map<std::string, float>> kLearningRates{
    {MaterialType::PBRTDiffuse,
     {
         {"diffuse", 1e-2f},
     }},
    {MaterialType::PBRTConductor,
     {
         {"eta", 1e-2f},
         {"k", 1e-2f},
         {"roughness", 1e-2f},
     }},
    {MaterialType::Standard,
     {
         {"base_color", 1e-2f},
         {"roughness", 3e-3f},
         {"metallic", 3e-3f},
     }},
};
```

### Viewport Layout

Three viewports arranged horizontally:

```
+----------------+----------------+----------------+
|  Initial      |   Difference   |   Reference    |
|  Material     |               |   Material      |
+----------------+----------------+----------------+
```

Viewport offsets calculated in `compile()`:
- `initViewPortOffset`: Left viewport
- `diffViewPortOffset`: Middle viewport
- `refViewPortOffset`: Right viewport

### BSDF Slice Geometry

BSDF slices use spherical coordinates (theta_h, theta_d):
- `theta_h`: Half-angle direction (x-axis)
- `theta_d`: Difference angle (y-axis)
- Range: [0, π/2] for both axes

Geometry calculation:
1. Place H vector at (0, 0, 1)
2. Compute L and V mirrored about yz-plane
3. Rotate L and V about x-axis by theta_h

### Loss Function

L2 loss function:

```hlsl
float3 computeLoss(const uint2 pixel)
{
    float3 fRef = evalBSDFSlice(true, pixel);
    float3 fCur = evalBSDFSlice(false, pixel);
    
    float3 diff = fCur - fRef;
    return 0.5 * diff * diff;  // L2 loss
}
```

Automatic stopping when relative L1 error < 1e-3:

```cpp
float relL1Error = 0.f;
for (size_t i = 0; i < mCurBSDFParams.size(); i++)
{
    relL1Error += std::abs(mCurBSDFParams[i] - mRefBSDFParams[i]) / std::max(mRefBSDFParams[i], 1e-6f);
}
relL1Error /= mCurBSDFParams.size();
if (relL1Error < 1e-3f)
{
    mRunOptimization = false;
}
```

### Gradient Computation

Gradients computed via automatic differentiation:
- `bwd_diff(computeLoss)(pixel, float3(1.f))` triggers backpropagation
- Gradients accumulated in SceneGradients buffer
- Aggregated across all threads

### Material Parameter Handling

Serialization/Deserialization:
- `serializeParams()`: Convert material parameters to flat array
- `deserializeParams()`: Convert flat array back to material
- Used for gradient updates and optimizer state

## Integration Points

### Render Graph Integration

**Input/Output**:
- Output: "output" - Three-viewport BSDF visualization

**Bind Flags**:
- Output: UnorderedAccess

**Format**:
- Output: RGBA32Float

**Size**:
- Output: Matches frame dimensions

### Scene Integration

- Monitors scene updates
- Requires scene with materials
- Supports multiple material types
- Material selection via dropdown

### DiffRendering Integration

- Uses SceneGradients for gradient storage
- GradientType::Material for BSDF gradients
- 64 gradient slots per parameter
- Automatic gradient aggregation

### Scripting Integration

```python
# Python bindings
pass.init_material_id  # Read-only
pass.ref_material_id    # Read-only
pass.bsdf_slice_resolution  # Read/write
pass.compute_bsdf_grads()  # Method
```

### UI Integration

Three-viewport display with controls:
- Start/Stop/Reset optimization buttons
- Initial material selection dropdown
- Reference material selection dropdown
- Material parameter editing

## Architecture Patterns

### Optimizer Pattern
- Gradient-based optimization
- Adam optimizer algorithm
- Per-parameter learning rates
- Automatic stopping

### Differentiable Rendering Pattern
- Automatic differentiation
- Differentiable BSDF evaluation
- Loss function computation
- Gradient accumulation

### Visualization Pattern
- Three-viewport rendering
- Real-time BSDF display
- Absolute difference visualization
- Interactive parameter editing

## Code Patterns

### Optimization Step

```cpp
void BSDFOptimizer::step(RenderContext* pRenderContext)
{
    auto pBuffer = mpSceneGradients->getGradsBuffer(GradientType::Material);
    pBuffer->getBlob(mBSDFGrads.data(), 0, sizeof(float) * mBSDFGrads.size());
    
    // Update BSDF parameters
    mAdam.step(mBSDFGrads, mCurBSDFParams);
    mpScene->getMaterial(MaterialID(mParams.initMaterialID))->deserializeParams(mCurBSDFParams);
}
```

### Optimizer Pass Execution

```cpp
void BSDFOptimizer::executeOptimizerPass(RenderContext* pRenderContext)
{
    if (!mpScene)
        return;
    
    mpSceneGradients->clearGrads(pRenderContext, GradientType::Material);
    
    auto var = mpOptimizerPass->getRootVar();
    var["CB"]["params"].setBlob(mParams);
    mpSceneGradients->bindShaderData(var["gSceneGradients"]);
    mpScene->bindShaderData(var["gScene"]);
    
    mpOptimizerPass->execute(pRenderContext, uint3(mParams.bsdfTableDim, 1));
    
    mpSceneGradients->aggregateGrads(pRenderContext, GradientType::Material);
}
```

### Initialization

```cpp
void BSDFOptimizer::initOptimization()
{
    // Reset BSDF parameters
    mpScene->getMaterial(MaterialID(mParams.initMaterialID))->deserializeParams(mInitBSDFParams);
    mParams.frameCount = 0;
    
    // Initialize current BSDF parameters
    mCurBSDFParams = mInitBSDFParams;
    
    // Set learning rates and adam optimizer
    std::vector<float> lr(mCurBSDFParams.size(), 0.f);
    const auto& pMaterial = mpScene->getMaterial(MaterialID{mParams.initMaterialID});
    
    auto learningRateMap = kLearningRates.find(pMaterial->getType());
    if (learningRateMap != kLearningRates.end())
    {
        for (const auto& param : pMaterial->getParamLayout())
        {
            auto learningRate = learningRateMap->second.find(param.pythonName);
            if (learningRate != learningRateMap->second.end())
            {
                for (uint32_t i = 0; i < param.size; ++i)
                    lr[param.offset + i] = learningRate->second;
            }
        }
    }
    
    mAdam = AdamOptimizer(lr);
}
```

## Use Cases

1. **Material Fitting**: Fit material parameters to match reference material
2. **BSDF Analysis**: Visualize BSDF behavior across different directions
3. **Inverse Rendering**: Recover material parameters from reference images
4. **Material Design**: Explore material parameter space interactively
5. **Educational**: Understand BSDF behavior and optimization
6. **Research**: Test differentiable rendering algorithms

## Performance Considerations

- **Gradient Computation**: Expensive due to automatic differentiation
- **Adam Optimizer**: Per-parameter operations, scales with parameter count
- **BSDF Resolution**: Higher resolution = more work per frame
- **Material Complexity**: More complex BSDFs = slower evaluation
- **GPU Utilization**: 16x16 thread groups, good occupancy
- **Memory**: Gradient storage for all parameters
- **Synchronization**: Fence for GPU-CPU synchronization

## Limitations

- **Material Types**: Only supports material types with defined learning rates
- **Scene Changes**: Does not support scene changes requiring recompilation
- **BSDF Resolution**: Limited by GPU memory
- **Learning Rates**: Manual tuning required per material type
- **Local Optima**: May converge to local optima
- **Gradient Noise**: Monte Carlo sampling introduces noise
- **Single BSDF**: Only optimizes one material at a time

## Best Practices

1. **Start with Low Resolution**: Use lower BSDF slice resolution for faster iteration
2. **Tune Learning Rates**: Adjust learning rates per material type
3. **Monitor Convergence**: Watch relative L1 error for convergence
4. **Use Good Initial Guess**: Start with parameters close to target
5. **Visualize Difference**: Use difference viewport to guide optimization
6. **Reset When Needed**: Reset optimization if it diverges
7. **Check Material Support**: Ensure material type has defined learning rates

## Progress Log

- **2026-01-07T21:08:08Z**: BSDFOptimizer analysis completed. Analyzed BSDFOptimizer class (BSDF parameter optimization with differentiable rendering), AdamOptimizer (gradient-based optimization with adaptive learning rates), BSDFOptimizer.cs.slang (differentiable BSDF evaluation and loss computation), BSDFOptimizerHelpers.slang (BSDF slice geometry calculation), BSDFOptimizerParams.slang (host-device parameter structures), BSDFViewer.cs.slang (three-viewport BSDF visualization), optimization pipeline (initialization, optimizer pass, step, viewer pass), learning rate configuration (per-material-type), viewport layout (initial/difference/reference), BSDF slice geometry (spherical coordinates), loss function (L2 loss), gradient computation (automatic differentiation), material parameter handling (serialization/deserialization), integration points (render graph, scene, diff rendering, scripting, UI), architecture patterns (optimizer, differentiable rendering, visualization), code patterns (optimization step, optimizer pass execution, initialization), use cases (material fitting, BSDF analysis, inverse rendering), performance considerations (gradient computation, Adam optimizer, BSDF resolution), limitations (material types, scene changes, local optima), and best practices. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
