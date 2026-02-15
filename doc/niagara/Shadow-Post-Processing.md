# Shadow Post-Processing

> **Relevant source files**
> * [src/shaders/shadow.comp.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadow.comp.glsl)
> * [src/shaders/shadowblur.comp.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl)
> * [src/shaders/shadowfill.comp.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl)

## Purpose and Scope

This document covers the post-processing steps applied to ray-traced shadow maps after the initial shadow ray tracing completes. The system implements two optimization techniques: checkerboard rendering to reduce ray tracing cost by half, and depth-aware blur for soft shadow generation. For details on shadow ray tracing itself, see [Shadow Ray Tracing](/zeux/niagara/9.2-shadow-ray-tracing). For acceleration structure management, see [Acceleration Structure Management](/zeux/niagara/9.1-acceleration-structure-management).

## Checkerboard Rendering Strategy

The shadow system supports optional checkerboard rendering, where shadow rays are only traced for half the pixels in a checkerboard pattern. This reduces ray tracing workload by 50% while maintaining visual quality through reconstruction.

### Checkerboard Pattern Generation

The shadow ray tracing shader selects pixels based on a checkerboard pattern:

```mermaid
flowchart TD

Invocation["Compute Invocation<br>(gl_GlobalInvocationID.xy)"]
CheckerTest["checkerboard > 0?"]
CalcPos["Calculate pixel position:<br>pos.x *= 2<br>pos.x += (pos.y ^ checkerboard) & 1"]
DirectPos["Use invocation ID directly"]
RayTrace["Trace shadow ray<br>at calculated position"]

Invocation --> CheckerTest
CheckerTest --> CalcPos
CheckerTest --> DirectPos
CalcPos --> RayTrace
DirectPos --> RayTrace
```

The `checkerboard` parameter controls the pattern phase (0 for disabled, 1 or 2 for alternating patterns). The XOR operation with `pos.y` ensures a checkerboard distribution where adjacent horizontal pixels alternate between traced and skipped.

**Sources:** [src/shaders/shadow.comp.glsl L120-L125](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadow.comp.glsl#L120-L125)

### Checkerboard Configuration

| Configuration | Description |
| --- | --- |
| `checkerboard = 0` | Disabled - trace all pixels |
| `checkerboard = 1` | Phase 1 - trace even columns on even rows, odd columns on odd rows |
| `checkerboard = 2` | Phase 2 - trace odd columns on even rows, even columns on odd rows |

The `ShadowData` push constant structure provides the checkerboard mode to the shader:

```mermaid
classDiagram
    class ShadowData {
        +vec3 sunDirection
        +float sunJitter
        +mat4 inverseViewProjection
        +vec2 imageSize
        +int checkerboard
    }
    class shadow_comp {
        +ShadowData shadowData
        +void main()
    }
    class shadowfill_comp {
        +int checkerboard
        +void main()
    }
    shadow_comp --> ShadowData : uses
    shadowfill_comp --> ShadowData : uses checkerboard field
```

**Sources:** [src/shaders/shadow.comp.glsl L17-L26](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadow.comp.glsl#L17-L26)

 [src/shaders/shadowfill.comp.glsl L7-L11](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L7-L11)

## Shadow Fill Pass

When checkerboard rendering is enabled, the `shadowfill.comp.glsl` shader reconstructs shadow values for the untraceable pixels using depth-weighted interpolation from neighboring traced pixels.

### Fill Algorithm

```mermaid
flowchart TD

Start["Compute Invocation<br>shadowfill.comp"]
CalcPos["Calculate opposite<br>checkerboard pixel:<br>pos.x *= 2<br>pos.x += ~(pos.y ^ checkerboard) & 1"]
SampleDepth["Sample center depth<br>and 4 neighbor depths"]
SampleShadows["Sample 4 neighbor<br>shadow values"]
CalcWeights["Calculate depth weights:<br>weights = exp2(-abs(depths/depth - 1) * 20)"]
Interpolate["Weighted average:<br>shadow = dot(weights, shadows) /<br>(dot(weights, vec4(1)) + 1e-2)"]
Store["Store reconstructed<br>shadow value"]

Start --> CalcPos
CalcPos --> SampleDepth
SampleDepth --> SampleShadows
SampleShadows --> CalcWeights
CalcWeights --> Interpolate
Interpolate --> Store
```

The fill shader operates on the complementary checkerboard pattern - pixels that were NOT traced during the shadow ray pass.

**Sources:** [src/shaders/shadowfill.comp.glsl L16-L45](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L16-L45)

### Depth-Weighted Interpolation

The reconstruction uses an exponential depth weighting function to preserve shadow discontinuities at geometry boundaries:

| Component | Formula | Purpose |
| --- | --- | --- |
| Neighbor depths | `depths = {left, right, up, down}` | Sample 4-neighbor depth values |
| Depth ratio | `depths / depth - 1` | Normalized depth difference |
| Weight function | `exp2(-abs(ratio) * 20)` | Exponential falloff with sharpness factor 20 |
| Final shadow | `dot(weights, shadows) / (sum(weights) + 1e-2)` | Normalized weighted average |

The exponential weight function [src/shaders/shadowfill.comp.glsl L40](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L40-L40)

 gives high weight to neighbors with similar depth (preserving smooth shadows) and near-zero weight to neighbors at significantly different depths (preventing shadow bleeding across geometry edges).

**Sources:** [src/shaders/shadowfill.comp.glsl L24-L44](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L24-L44)

## Shadow Blur Pass

The `shadowblur.comp.glsl` shader applies a separable depth-aware Gaussian blur to generate soft shadows while preserving sharp shadow boundaries at geometry discontinuities.

### Blur Pipeline Architecture

```mermaid
flowchart TD

Input["Shadow Image<br>(potentially checkerboard-filled)"]
HBlur["Horizontal Blur Pass<br>direction = 0"]
HTemp["Temporary Image"]
VBlur["Vertical Blur Pass<br>direction = 1"]
Output["Final Blurred Shadow"]

Input --> HBlur
HBlur --> HTemp
HTemp --> VBlur
VBlur --> Output
```

The blur is implemented as two passes with a separable kernel. The `direction` push constant controls the blur axis: 0 for horizontal (X-axis), 1 for vertical (Y-axis).

**Sources:** [src/shaders/shadowblur.comp.glsl L9-L16](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L9-L16)

### Depth-Aware Bilateral Filter

The blur kernel combines Gaussian spatial weighting with depth-based bilateral weighting to prevent bleeding across depth discontinuities:

```mermaid
flowchart TD

Center["Center pixel:<br>depth, shadow value"]
Gradient["Estimate depth gradient:<br>dgrad = neighbor_depth - depth<br>(if abs difference < 0.1)"]
Kernel["For each offset i in [-KERNEL, +KERNEL]"]
SpatialW["Gaussian weight:<br>gw = exp2(-i*i / 50)"]
PredictD["Predict depth:<br>depth + dgrad * i"]
SampleD["Sample actual depth at offset"]
DepthW["Depth weight:<br>dw = exp2(-abs(actual - predicted) * 100)"]
CombineW["Final weight:<br>fw = gw * dw"]
Accumulate["Accumulate weighted shadow:<br>shadow += sample * fw<br>accumw += fw"]
Normalize["Normalize:<br>shadow / accumw"]

Center --> Gradient
Gradient --> Kernel
Kernel --> SpatialW
Kernel --> PredictD
Kernel --> SampleD
SpatialW --> CombineW
PredictD --> DepthW
SampleD --> DepthW
DepthW --> CombineW
CombineW --> Accumulate
Accumulate --> Normalize
```

**Sources:** [src/shaders/shadowblur.comp.glsl L27-L57](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L27-L57)

### Blur Parameters

| Parameter | Value | Description |
| --- | --- | --- |
| `KERNEL` | 10 | Blur radius in pixels [src/shaders/shadowblur.comp.glsl L35](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L35-L35) |
| Gaussian sharpness | 50 | Controls spatial falloff: `exp2(-i*i / 50)` |
| Depth gradient threshold | 0.1 | Maximum depth difference for gradient estimation [src/shaders/shadowblur.comp.glsl L41](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L41-L41) |
| Depth weight sharpness | 100 | Controls depth-based falloff: `exp2(-abs(delta) * 100)` [src/shaders/shadowblur.comp.glsl L49](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L49-L49) |

### Depth Gradient Estimation

The blur shader estimates the local depth gradient to handle sloped surfaces correctly [src/shaders/shadowblur.comp.glsl L39-L41](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L39-L41)

:

```mermaid
flowchart TD

CenterDepth["Center pixel depth"]
NextDepth["Adjacent neighbor depth<br>(in blur direction)"]
Compare["abs(center - next)<br>< 0.1?"]
CalcGrad["dgrad = next - center"]
ZeroGrad["dgrad = 0<br>(discontinuity)"]
UseGrad["Predict depths along blur:<br>depth + dgrad * i"]

CenterDepth --> Compare
NextDepth --> Compare
Compare --> CalcGrad
Compare --> ZeroGrad
CalcGrad --> UseGrad
ZeroGrad --> UseGrad
```

When the depth difference to the immediate neighbor exceeds 0.1 (normalized depth space), the gradient is zeroed to prevent blurring across geometry edges. Otherwise, the gradient is used to predict expected depths along the blur direction, allowing smooth blurring along sloped surfaces.

**Sources:** [src/shaders/shadowblur.comp.glsl L39-L41](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L39-L41)

## Post-Processing Pipeline Integration

The complete shadow post-processing pipeline consists of up to three compute passes:

```mermaid
flowchart TD

RayTrace["shadow.comp<br>Ray trace shadows<br>(potentially checkerboard)"]
CheckCB["Checkerboard<br>enabled?"]
Fill["shadowfill.comp<br>Reconstruct missing pixels<br>Depth-weighted interpolation"]
BlurH["shadowblur.comp<br>Horizontal blur pass<br>direction = 0"]
BlurV["shadowblur.comp<br>Vertical blur pass<br>direction = 1"]
Lighting["final.comp<br>Apply shadows to lighting"]

RayTrace --> CheckCB
BlurV --> Lighting

subgraph subGraph2 ["Final Composition (8.3)"]
    Lighting
end

subgraph subGraph1 ["Shadow Post-Processing (9.3)"]
    CheckCB
    Fill
    BlurH
    BlurV
    CheckCB --> Fill
    CheckCB --> BlurH
    Fill --> BlurH
    BlurH --> BlurV
end

subgraph subGraph0 ["Shadow Ray Tracing (9.2)"]
    RayTrace
end
```

### Pass Execution Details

| Pass | Shader | Input Images | Output Image | Description |
| --- | --- | --- | --- | --- |
| 1. Shadow Fill | `shadowfill.comp` | `shadowImage` (R8), `depthImage` | `shadowImage` (R8) | Reconstructs checkerboard-skipped pixels using depth-weighted 4-neighbor interpolation |
| 2. Horizontal Blur | `shadowblur.comp` | `shadowImage`, `depthImage` | Temporary image | Applies depth-aware 1D Gaussian blur horizontally with kernel size 10 |
| 3. Vertical Blur | `shadowblur.comp` | Temporary image, `depthImage` | `shadowImage` | Applies depth-aware 1D Gaussian blur vertically with kernel size 10 |

The shadow fill pass is conditionally executed only when checkerboard rendering is active. Both blur passes always execute to produce soft shadows.

**Sources:** [src/shaders/shadowfill.comp.glsl L1-L46](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L1-L46)

 [src/shaders/shadowblur.comp.glsl L1-L64](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L1-L64)

### Image Formats and Bindings

```mermaid
classDiagram
    class shadowfill_comp {
        push_constant: imageSize, checkerboard
        local_size: 8x8x1
        binding0: image2D shadowImage(R8, read/write)
        binding1: sampler2D depthImage(read-only)
    }
    class shadowblur_comp {
        push_constant: imageSize, direction, znear
        local_size: 8x8x1
        binding0: image2D outImage(write-only)
        binding1: sampler2D shadowImage(read-only)
        binding2: sampler2D depthImage(read-only)
    }
    shadowfill_comp --> shadowblur_comp : shadowImage output becomes input
```

All shadow post-processing shaders use 8×8 workgroup sizes [src/shaders/shadowfill.comp.glsl L5](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L5-L5)

 [src/shaders/shadowblur.comp.glsl L9](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L9-L9)

 for efficient compute dispatch. The shadow image uses R8 format (single-channel 8-bit) as only a scalar shadow factor is needed.

**Sources:** [src/shaders/shadowfill.comp.glsl L5-L14](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowfill.comp.glsl#L5-L14)

 [src/shaders/shadowblur.comp.glsl L9-L21](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L9-L21)

### Depth Buffer Format

Both fill and blur shaders sample from the same depth buffer used during G-buffer rendering. The depth is converted from normalized device coordinates to linear view-space depth for accurate geometric comparisons:

```
linear_depth = znear / depth_buffer_value
```

The `znear` parameter [src/shaders/shadowblur.comp.glsl L15](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L15-L15)

 is passed as a push constant to enable this conversion.

**Sources:** [src/shaders/shadowblur.comp.glsl L31](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L31-L31)

 [src/shaders/shadowblur.comp.glsl L40](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L40-L40)

 [src/shaders/shadowblur.comp.glsl L48](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/shadowblur.comp.glsl#L48-L48)