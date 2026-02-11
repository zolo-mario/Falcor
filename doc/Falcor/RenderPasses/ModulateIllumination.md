# ModulateIllumination - Illumination Modulation Pass

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **ModulateIllumination** - Illumination modulation pass
  - [x] **ModulateIllumination.h** - Modulate illumination header (71 lines)
  - [x] **ModulateIllumination.cpp** - Modulate illumination implementation (235 lines)
  - [x] **ModulateIllumination.cs.slang** - Modulate illumination compute shader (129 lines)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Texture, ResourceBindFlags)
- **Core/Object** - Base object class
- **RenderGraph** - Render graph system (RenderPass, RenderPassReflection, RenderPassHelpers)
- **Core/Pass** - Compute pass base class (ComputePass)

## Module Overview

ModulateIllumination implements an illumination modulation pass that allows users to selectively enable/disable various illumination components (emission, diffuse, specular reflection, specular radiance, delta reflection emission, delta reflection reflectance, delta reflection radiance, delta transmission emission, delta transmission reflectance, delta transmission radiance, and residual radiance) and combine them with the base illumination. This pass is useful for debugging, material analysis, and testing different lighting contributions in isolation.

## Component Specifications

### ModulateIllumination Class

**File**: [`ModulateIllumination.h`](Source/RenderPasses/ModulateIllumination/ModulateIllumination.h:35)

**Purpose**: Illumination modulation pass for selectively enabling/disabling illumination components.

**Key Features**:
- Emission modulation
- Diffuse reflectance modulation
- Specular reflectance modulation
- Specular radiance modulation
- Delta reflection emission modulation
- Delta reflection reflectance modulation
- Delta reflection radiance modulation
- Delta transmission emission modulation
- Delta transmission reflectance modulation
- Delta transmission radiance modulation
- Residual radiance modulation
- Output size configuration
- Optional input handling
- Frame dimension tracking

**Public Interface**:

```cpp
class ModulateIllumination : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ModulateIllumination, "ModulateIllumination", "Modulate illumination pass.");

    static ref<ModulateIllumination> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<ModulateIllumination>(pDevice, props);
    }

    ModulateIllumination(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override;
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;

private:
    uint2 mFrameDim = {0, 0};
    RenderPassHelpers::IOSize mOutputSizeSelection = RenderPassHelpers::IOSize::Default;

    ref<ComputePass> mpModulateIlluminationPass;

    bool mUseEmission = true;
    bool mUseDiffuseReflectance = true;
    bool mUseDiffuseRadiance = true;
    bool mUseSpecularReflectance = true;
    bool mUseSpecularRadiance = true;
    bool mUseDeltaReflectionEmission = true;
    bool mUseDeltaReflectionReflectance = true;
    bool mUseDeltaReflectionRadiance = true;
    bool mUseDeltaTransmissionEmission = true;
    bool mUseDeltaTransmissionReflectance = true;
    bool mUseDeltaTransmissionRadiance = true;
    bool mUseResidualRadiance = true;
};
```

**Public Members**: None (all members are private)

**Private Members**:
- `uint2 mFrameDim` - Frame dimensions (width, height)
- `RenderPassHelpers::IOSize mOutputSizeSelection` - Output size selection (default Default)
- `ref<ComputePass> mpModulateIlluminationPass` - Compute pass
- `bool mUseEmission` - Use emission flag (default true)
- `bool mUseDiffuseReflectance` - Use diffuse reflectance flag (default true)
- `bool mUseDiffuseRadiance` - Use diffuse radiance flag (default true)
- `bool mUseSpecularReflectance` - Use specular reflectance flag (default true)
- `bool mUseSpecularRadiance` - Use specular radiance flag (default true)
- `bool mUseDeltaReflectionEmission` - Use delta reflection emission flag (default true)
- `bool mUseDeltaReflectionReflectance` - Use delta reflection reflectance flag (default true)
- `bool mUseDeltaReflectionRadiance` - Use delta reflection radiance flag (default true)
- `bool mUseDeltaTransmissionEmission` - Use delta transmission emission flag (default true)
- `bool mUseDeltaTransmissionReflectance` - Use delta transmission reflectance flag (default true)
- `bool mUseDeltaTransmissionRadiance` - Use delta transmission radiance flag (default true)
- `bool mUseResidualRadiance` - Use residual radiance flag (default true)

**Private Methods**: None (all methods are public)

**Public Methods**:
- `Properties getProperties() const override` - Get properties
- `RenderPassReflection reflect(const CompileData& compileData) override` - Reflect input/output
- `void compile(RenderContext* pRenderContext, const CompileData& compileData) override` - Compile pass
- `void execute(RenderContext* pRenderContext, const RenderData& renderData) override` - Execute pass
- `void renderUI(Gui::Widgets& widget) override` - Render UI

### ModulateIllumination.cs.slang

**File**: [`ModulateIllumination.cs.slang`](Source/RenderPasses/ModulateIllumination/ModulateIllumination.cs.slang:129)

**Purpose**: Compute shader for illumination modulation.

**Features**:
- Emission modulation
- Diffuse reflectance modulation
- Specular reflectance modulation
- Specular radiance modulation
- Delta reflection emission modulation
- Delta reflection reflectance modulation
- Delta reflection radiance modulation
- Delta transmission emission modulation
- Delta transmission reflectance modulation
- Delta transmission radiance modulation
- Residual radiance modulation
- Optional input handling via defines
- Frame dimension tracking
- 16x16 thread groups

**Constant Buffer**:
```cpp
cbuffer CB
{
    uint2 frameDim;
}
```

**Textures**:
```cpp
Texture2D gEmission;
Texture2D gDiffuseReflectance;
Texture2D gDiffuseRadiance;
Texture2D gSpecularReflectance;
Texture2D gSpecularRadiance;
Texture2D gDeltaReflectionEmission;
Texture2D gDeltaReflectionReflectance;
Texture2D gDeltaReflectionRadiance;
Texture2D gDeltaTransmissionEmission;
Texture2D gDeltaTransmissionReflectance;
Texture2D gDeltaTransmissionRadiance;
Texture2D gResidualRadiance;

RWTexture2D gOutput;
```

**Defines**:
```cpp
#define is_valid(name) (is_valid_##name != 0)
```

**Main Function**:
```cpp
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    const uint2 pixel = dispatchThreadId.xy;
    if (any(pixel >= frameDim))
        return;

    float4 outputColor = float4(0.f);

    if (is_valid(gEmission))
    {
        outputColor.rgb += gEmission[pixel].rgb;
    }

    if (is_valid(gDiffuseReflectance))
    {
        float3 diffuseColor = gDiffuseReflectance[pixel].rgb;

        if (is_valid(gDiffuseRadiance))
        {
            diffuseColor *= gDiffuseRadiance[pixel].rgb;
        }

        outputColor.rgb += diffuseColor;
    }

    if (is_valid(gSpecularReflectance))
    {
        float3 specularColor = gSpecularReflectance[pixel].rgb;

        if (is_valid(gSpecularRadiance))
        {
            specularColor *= gSpecularRadiance[pixel].rgb;
        }

        outputColor.rgb += specularColor;
    }

    if (is_valid(gDeltaReflectionEmission))
    {
        outputColor.rgb += gDeltaReflectionEmission[pixel].rgb;
    }

    if (is_valid(gDeltaReflectionReflectance))
    {
        float3 deltaReflectionColor = gDeltaReflectionEmission[pixel].rgb;

        if (is_valid(gDeltaReflectionRadiance))
        {
            deltaReflectionColor *= gDeltaReflectionRadiance[pixel].rgb;
        }

        outputColor.rgb += deltaReflectionColor;
    }

    if (is_valid(gDeltaTransmissionEmission))
    {
        outputColor.rgb += gDeltaTransmissionEmission[pixel].rgb;
    }

    if (is_valid(gDeltaTransmissionReflectance))
    {
        float3 deltaTransmissionColor = gDeltaTransmissionEmission[pixel].rgb;

        if (is_valid(gDeltaTransmissionRadiance))
        {
            deltaTransmissionColor *= gDeltaTransmissionRadiance[pixel].rgb;
        }

        outputColor.rgb += deltaTransmissionColor;
    }

    if (is_valid(gResidualRadiance))
    {
        outputColor.rgb += gResidualRadiance[pixel].rgb;
    }

    gOutput[pixel] = outputColor;
}
```

## Technical Details

### Input Channels

**Emission**:
- Name: `emission`
- Texture: `gEmission`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Emission color

**Diffuse Reflectance**:
- Name: `diffuseReflectance`
- Texture: `gDiffuseReflectance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Diffuse reflectance color

**Diffuse Radiance**:
- Name: `diffuseRadiance`
- Texture: `gDiffuseRadiance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Diffuse radiance modulation color

**Specular Reflectance**:
- Name: `specularReflectance`
- Texture: `gSpecularReflectance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Specular reflectance color

**Specular Radiance**:
- Name: `specularRadiance`
- Texture: `gSpecularRadiance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Specular radiance modulation color

**Delta Reflection Emission**:
- Name: `deltaReflectionEmission`
- Texture: `gDeltaReflectionEmission`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Delta reflection emission color

**Delta Reflection Reflectance**:
- Name: `deltaReflectionReflectance`
- Texture: `gDeltaReflectionReflectance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Delta reflection reflectance modulation color

**Delta Reflection Radiance**:
- Name: `deltaReflectionRadiance`
- Texture: `gDeltaReflectionRadiance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Delta reflection radiance modulation color

**Delta Transmission Emission**:
- Name: `deltaTransmissionEmission`
- Texture: `gDeltaTransmissionEmission`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Delta transmission emission color

**Delta Transmission Reflectance**:
- Name: `deltaTransmissionReflectance`
- Texture: `gDeltaTransmissionReflectance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Delta transmission reflectance modulation color

**Residual Radiance**:
- Name: `residualRadiance`
- Texture: `gResidualRadiance`
- Format: `Texture2D`
- Bind Flags: `ShaderResource`
- Optional: true
- Description: Residual radiance color

### Output Channel

**Output**:
- Name: `output`
- Texture: `gOutput`
- Format: `RWTexture2D`
- Bind Flags: `UnorderedAccess`
- Resource Format: `RGBA32Float`
- Description: Modulated output color

### Illumination Modulation

**Emission**:
- Adds emission color to output
- Modulated by `is_valid(gEmission)` define
- Simple addition: `outputColor.rgb += gEmission[pixel].rgb`

**Diffuse Reflection**:
- Adds diffuse reflectance color to output
- Modulated by `is_valid(gDiffuseReflectance)` define
- Radiance modulation: `diffuseColor *= gDiffuseRadiance[pixel].rgb`
- Addition: `outputColor.rgb += diffuseColor`

**Specular Reflection**:
- Adds specular reflectance color to output
- Modulated by `is_valid(gSpecularReflectance)` define
- Radiance modulation: `specularColor *= gSpecularRadiance[pixel].rgb`
- Addition: `outputColor.rgb += specularColor`

**Delta Reflection**:
- Adds delta reflection emission color to output
- Modulated by `is_valid(gDeltaReflectionEmission)` define
- Reflectance modulation: `deltaReflectionColor *= gDeltaReflectionReflectance[pixel].rgb`
- Addition: `outputColor.rgb += deltaReflectionColor`

**Delta Reflection Radiance**:
- Modulates delta reflection emission by reflectance
- Modulated by `is_valid(gDeltaReflectionRadiance)` define
- Modulation: `deltaReflectionColor *= gDeltaReflectionRadiance[pixel].rgb`
- Addition: `outputColor.rgb += deltaReflectionColor`

**Delta Transmission**:
- Adds delta transmission emission color to output
- Modulated by `is_valid(gDeltaTransmissionEmission)` define
- Addition: `outputColor.rgb += gDeltaTransmissionEmission[pixel].rgb`

**Delta Transmission Radiance**:
- Modulates delta transmission emission by reflectance
- Modulated by `is_valid(gDeltaTransmissionReflectance)` define
- Modulation: `deltaTransmissionColor *= gDeltaTransmissionRadiance[pixel].rgb`
- Addition: `outputColor.rgb += deltaTransmissionColor`

**Residual Radiance**:
- Adds residual radiance color to output
- Modulated by `is_valid(gResidualRadiance)` define
- Addition: `outputColor.rgb += gResidualRadiance[pixel].rgb`

### Compile Implementation

**Purpose**: Compile the compute pass with appropriate defines.

**Features**:
- Define management for optional inputs
- Frame dimension tracking
- Compute pass creation

**Compile Process**:
```cpp
void ModulateIllumination::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mpModulateIlluminationPass = ComputePass::create(mpDevice, kShaderFile, "main", DefineList(), false);

    // Override defines.
    if (!mUseEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gEmission", "0");
    if (!mUseDiffuseReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDiffuseReflectance", "0");
    if (!mUseDiffuseRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gDiffuseRadiance", "0");
    if (!mUseSpecularReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gSpecularReflectance", "0");
    if (!mUseSpecularRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gSpecularRadiance", "0");
    if (!mUseDeltaReflectionEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaReflectionEmission", "0");
    if (!mUseDeltaReflectionReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaReflectionReflectance", "0");
    if (!mUseDeltaTransmissionEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaTransmissionEmission", "0");
    if (!mUseDeltaTransmissionReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaTransmissionReflectance", "0");
    if (!mUseResidualRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gResidualRadiance", "0");
}
```

### Execute Implementation

**Purpose**: Execute the illumination modulation pass.

**Features**:
- Frame dimension tracking
- Output size calculation
- Resource binding
- Compute pass execution

**Execute Process**:
```cpp
void ModulateIllumination::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pOutput = renderData.getTexture(kOutput);
    mFrameDim = {pOutput->getWidth(), pOutput->getHeight()};

    // For optional I/O resources, set 'is_valid_<name>' defines to inform the program of which ones it can access.
    // TODO: This should be moved to a more general mechanism using Slang.
    DefineList defineList = getValidResourceDefines(kInputChannels, renderData);

    if (mpModulateIlluminationPass->getProgram()->addDefines(defineList))
    {
        mpModulateIlluminationPass->setVars(nullptr);
    }

    auto var = mpModulateIlluminationPass->getRootVar();
    var["CB"]["frameDim"] = mFrameDim;

    auto bind = [&](const ChannelDesc& desc)
    {
        if (!desc.texname.empty())
        {
            ref<Texture> pTexture = renderData.getTexture(desc.name);
            if (pTexture && (mFrameDim.x != pTexture->getWidth() || mFrameDim.y != pTexture->getHeight()))
            {
                logError(
                    "Texture {} has dim {}x{}, not compatible with FrameDim {}x{}.",
                    pTexture->getName(),
                    pTexture->getWidth(),
                    pTexture->getHeight(),
                    mFrameDim.x,
                    mFrameDim.y
                );
            }
            var[desc.texname] = pTexture;
        }
    };
    for (const auto& channel : kInputChannels)
        bind(channel);

    var["gOutput"] = pOutput;

    mpModulateIlluminationPass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

### Property Parsing

**Purpose**: Parse configuration properties from Python or serialization.

**Properties**:
- `useEmission` - Use emission flag
- `useDiffuseReflectance` - Use diffuse reflectance flag
- `useDiffuseRadiance` - Use diffuse radiance flag
- `useSpecularReflectance` - Use specular reflectance flag
- `useSpecularRadiance` - Use specular radiance flag
- `useDeltaReflectionEmission` - Use delta reflection emission flag
- `useDeltaReflectionReflectance` - Use delta reflection reflectance flag
- `useDeltaTransmissionEmission` - Use delta transmission emission flag
- `useDeltaTransmissionReflectance` - Use delta transmission reflectance flag
- `useResidualRadiance` - Use residual radiance flag
- `outputSize` - Output size selection

**Parsing Process**:
```cpp
for (const auto& [key, value] : props)
{
    if (key == kUseEmission)
        mUseEmission = value;
    else if (key == kUseDiffuseReflectance)
        mUseDiffuseReflectance = value;
    else if (key == kUseDiffuseRadiance)
        mUseDiffuseRadiance = value;
    else if (key == kUseSpecularReflectance)
        mUseSpecularReflectance = value;
    else if (key == kUseSpecularRadiance)
        mUseSpecularRadiance = value;
    else if (key == kUseDeltaReflectionEmission)
        mUseDeltaReflectionEmission = value;
    else if (key == kUseDeltaReflectionReflectance)
        mUseDeltaReflectionReflectance = value;
    else if (key == kUseDeltaTransmissionEmission)
        mUseDeltaTransmissionEmission = value;
    else if (key == kUseDeltaTransmissionReflectance)
        mUseDeltaTransmissionReflectance = value;
    else if (key == kUseDeltaTransmissionRadiance)
        mUseDeltaTransmissionRadiance = value;
    else if (key == kUseResidualRadiance)
        mUseResidualRadiance = value;
    else if (key == kOutputSize)
        mOutputSizeSelection = value;
    else
        logWarning("Unknown property '{}' in ModulateIllumination properties.", key);
}
```

### Property Serialization

**Purpose**: Serialize configuration properties.

**Serialization Process**:
```cpp
Properties ModulateIllumination::getProperties() const
{
    Properties props;
    props[kUseEmission] = mUseEmission;
    props[kUseDiffuseReflectance] = mUseDiffuseReflectance;
    props[kUseDiffuseRadiance] = mUseDiffuseRadiance;
    props[kUseSpecularReflectance] = mUseSpecularReflectance;
    props[kUseSpecularRadiance] = mUseSpecularRadiance;
    props[kUseDeltaReflectionEmission] = mUseDeltaReflectionEmission;
    props[kUseDeltaReflectionReflectance] = mUseDeltaReflectionReflectance;
    props[kUseDeltaTransmissionEmission] = mUseDeltaTransmissionEmission;
    props[kUseDeltaTransmissionReflectance] = mUseDeltaTransmissionReflectance;
    props[kUseDeltaTransmissionRadiance] = mUseDeltaTransmissionRadiance;
    props[kUseResidualRadiance] = mUseResidualRadiance;
    props[kOutputSize] = mOutputSizeSelection;
    return props;
}
```

### UI Rendering

**Purpose**: Provide UI controls for illumination modulation.

**Controls**:
- Emission checkbox
- Diffuse Reflectance checkbox
- Diffuse Radiance checkbox
- Specular Reflectance checkbox
- Specular Radiance checkbox
- Delta Reflection Emission checkbox
- Delta Reflection Reflectance checkbox
- Delta Reflection Radiance checkbox
- Delta Transmission Emission checkbox
- Delta Transmission Reflectance checkbox
- Delta Transmission Radiance checkbox
- Residual Radiance checkbox
- Output size dropdown

**UI Rendering**:
```cpp
void ModulateIllumination::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Emission", mUseEmission);
    widget.checkbox("Diffuse Reflectance", mUseDiffuseReflectance);
    widget.checkbox("Diffuse Radiance", mUseDiffuseRadiance);
    widget.checkbox("Specular Reflectance", mUseSpecularReflectance);
    widget.checkbox("Specular Radiance", mUseSpecularRadiance);
    widget.checkbox("Delta Reflection Emission", mUseDeltaReflectionEmission);
    widget.checkbox("Delta Reflection Reflectance", mUseDeltaReflectionReflectance);
    widget.checkbox("Delta Transmission Emission", mUseDeltaTransmissionEmission);
    widget.checkbox("Delta Transmission Reflectance", mUseDeltaTransmissionReflectance);
    widget.checkbox("Delta Transmission Radiance", mUseDeltaTransmissionRadiance);
    widget.checkbox("Residual Radiance", mUseResidualRadiance);
    widget.dropdown("Output size", mOutputSizeSelection);
}
```

## Architecture Patterns

### Render Pass Pattern
- Inherits from RenderPass base class
- Implements render() method
- Specifies input/output resources
- Manages shader compilation
- Parameter reflection

### Compute Pass Pattern
- Uses ComputePass base class
- Compute shader execution
- Thread group configuration (16x16)
- Resource binding

### Illumination Modulation Pattern
- Selective component enabling/disabling
- Radiance modulation
- Delta term handling
- Residual term handling

## Code Patterns

### Property Parsing
```cpp
for (const auto& [key, value] : props)
{
    if (key == kUseEmission)
        mUseEmission = value;
    else if (key == kUseDiffuseReflectance)
        mUseDiffuseReflectance = value;
    else if (key == kUseDiffuseRadiance)
        mUseDiffuseRadiance = value;
    else if (key == kUseSpecularReflectance)
        mUseSpecularReflectance = value;
    else if (key == kUseSpecularRadiance)
        mUseSpecularRadiance = value;
    else if (key == kUseDeltaReflectionEmission)
        mUseDeltaReflectionEmission = value;
    else if (key == kUseDeltaReflectionReflectance)
        mUseDeltaReflectionReflectance = value;
    else if (key == kUseDeltaTransmissionEmission)
        mUseDeltaTransmissionEmission = value;
    else if (key == kUseDeltaTransmissionReflectance)
        mUseDeltaTransmissionReflectance = value;
    else if (key == kUseDeltaTransmissionRadiance)
        mUseDeltaTransmissionRadiance = value;
    else if (key == kUseResidualRadiance)
        mUseResidualRadiance = value;
    else if (key == kOutputSize)
        mOutputSizeSelection = value;
    else
        logWarning("Unknown property '{}' in ModulateIllumination properties.", key);
}
```

### Property Serialization
```cpp
Properties ModulateIllumination::getProperties() const
{
    Properties props;
    props[kUseEmission] = mUseEmission;
    props[kUseDiffuseReflectance] = mUseDiffuseReflectance;
    props[kUseDiffuseRadiance] = mUseDiffuseRadiance;
    props[kUseSpecularReflectance] = mUseSpecularReflectance;
    props[kUseSpecularRadiance] = mUseSpecularRadiance;
    props[kUseDeltaReflectionEmission] = mUseDeltaReflectionEmission;
    props[kUseDeltaReflectionReflectance] = mUseDeltaReflectionReflectance;
    props[kUseDeltaTransmissionEmission] = mUseDeltaTransmissionEmission;
    props[kUseDeltaTransmissionReflectance] = mUseDeltaTransmissionReflectance;
    props[kUseDeltaTransmissionRadiance] = mUseDeltaTransmissionRadiance;
    props[kUseResidualRadiance] = mUseResidualRadiance;
    props[kOutputSize] = mOutputSizeSelection;
    return props;
}
```

### Compile Implementation
```cpp
void ModulateIllumination::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    mpModulateIlluminationPass = ComputePass::create(mpDevice, kShaderFile, "main", DefineList(), false);

    // Override defines.
    if (!mUseEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gEmission", "0");
    if (!mUseDiffuseReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDiffuseReflectance", "0");
    if (!mUseDiffuseRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gDiffuseRadiance", "0");
    if (!mUseSpecularReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gSpecularReflectance", "0");
    if (!mUseSpecularRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gSpecularRadiance", "0");
    if (!mUseDeltaReflectionEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaReflectionEmission", "0");
    if (!mUseDeltaReflectionReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaReflectionReflectance", "0");
    if (!mUseDeltaTransmissionEmission)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaTransmissionEmission", "0");
    if (!mUseDeltaTransmissionReflectance)
        mpModulateIlluminationPass->addDefine("is_valid_gDeltaTransmissionReflectance", "0");
    if (!mUseResidualRadiance)
        mpModulateIlluminationPass->addDefine("is_valid_gResidualRadiance", "0");
}
```

### Execute Implementation
```cpp
void ModulateIllumination::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pOutput = renderData.getTexture(kOutput);
    mFrameDim = {pOutput->getWidth(), pOutput->getHeight()};

    // For optional I/O resources, set 'is_valid_<name>' defines to inform the program of which ones it can access.
    // TODO: This should be moved to a more general mechanism using Slang.
    DefineList defineList = getValidResourceDefines(kInputChannels, renderData);

    if (mpModulateIlluminationPass->getProgram()->addDefines(defineList))
    {
        mpModulateIlluminationPass->setVars(nullptr);
    }

    auto var = mpModulateIlluminationPass->getRootVar();
    var["CB"]["frameDim"] = mFrameDim;

    auto bind = [&](const ChannelDesc& desc)
    {
        if (!desc.texname.empty())
        {
            ref<Texture> pTexture = renderData.getTexture(desc.name);
            if (pTexture && (mFrameDim.x != pTexture->getWidth() || mFrameDim.y != pTexture->getHeight()))
            {
                logError(
                    "Texture {} has dim {}x{}, not compatible with FrameDim {}x{}.",
                    pTexture->getName(),
                    pTexture->getWidth(),
                    pTexture->getHeight(),
                    mFrameDim.x,
                    mFrameDim.y
                );
            }
            var[desc.texname] = pTexture;
        }
    };
    for (const auto& channel : kInputChannels)
        bind(channel);

    var["gOutput"] = pOutput;

    mpModulateIlluminationPass->execute(pRenderContext, mFrameDim.x, mFrameDim.y);
}
```

### Resource Binding
```cpp
auto bind = [&](const ChannelDesc& desc)
{
    if (!desc.texname.empty())
    {
        ref<Texture> pTexture = renderData.getTexture(desc.name);
        if (pTexture && (mFrameDim.x != pTexture->getWidth() || mFrameDim.y != pTexture->getHeight()))
        {
            logError(
                "Texture {} has dim {}x{}, not compatible with FrameDim {}x{}.",
                pTexture->getName(),
                pTexture->getWidth(),
                pTexture->getHeight(),
                mFrameDim.x,
                mFrameDim.y
            );
        }
        var[desc.texname] = pTexture;
    }
};
for (const auto& channel : kInputChannels)
    bind(channel);
```

## Integration Points

### Render Graph Integration
- Input/output specification via reflect()
- Resource binding in execute()
- Property serialization
- Scene-independent pass

### Compute Pass Integration
- ComputePass base class
- Compute shader execution
- Thread group configuration (16x16)
- Resource binding

### UI Integration
- Gui::Widgets for controls
- Checkbox controls for each illumination component
- Output size dropdown

### Python Bindings
```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, ModulateIllumination>();
}
```

### Plugin Registration
```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, ModulateIllumination>();
}
```

## Use Cases

### Debugging and Analysis
- Enable/disable emission for debugging
- Test diffuse reflection in isolation
- Test specular reflection in isolation
- Test radiance modulation
- Test delta terms in isolation
- Test residual radiance

### Material Analysis
- Analyze emission contribution
- Analyze diffuse reflection contribution
- Analyze specular reflection contribution
- Analyze radiance modulation
- Analyze delta term contribution

### Lighting Analysis
- Test different lighting models
- Compare direct vs. indirect illumination
- Analyze light contribution breakdown

### Research and Development
- Test new lighting techniques
- Compare different BSDF models
- Analyze delta term effects
- Develop new illumination models

### Educational Use
- Demonstrate illumination components
- Show radiance modulation
- Explain delta terms
- Teach material analysis
- Visualize lighting contributions

## Performance Considerations

### GPU Computation
- Illumination modulation: O(width × height) per frame
- Simple texture fetches: O(1) per pixel per enabled component
- Simple arithmetic operations: O(1) per pixel
- 16x16 thread groups: O(width × height / 256) threads

### Memory Usage
- Input textures: O(width × height × sizeof(float4) × 10) bytes (10 optional inputs)
- Output texture: O(width × height × sizeof(float4)) bytes
- Compute pass state: O(compute_pass_state_size) bytes
- Total: O(width × height × 44) bytes

### Computational Complexity
- Illumination modulation: O(1) per pixel per enabled component
- Maximum: O(10) per pixel (all components enabled)
- Overall: O(width × height) per frame

### Performance Optimization
- 16x16 thread groups for efficient parallelization
- Simple arithmetic operations
- Minimal texture fetches
- No complex branching
- Efficient memory access patterns

## Limitations

### Feature Limitations
- Simple modulation (no advanced effects)
- No temporal accumulation
- No spatial filtering
- No adaptive quality settings
- No preset management

### UI Limitations
- No preset management
- No undo/redo functionality
- No real-time preview
- Limited configuration options

### Performance Limitations
- No performance optimization for large textures
- No caching of modulation results
- No adaptive quality settings
- No GPU compute optimization

### Integration Limitations
- Limited to Falcor's render graph system
- Requires specific input formats
- No external data sources
- Limited to Falcor's UI system

## Best Practices

### Debugging Best Practices
- Use emission checkbox for testing emissive materials
- Use diffuse reflectance for testing diffuse materials
- Use specular reflection for testing specular materials
- Test components in isolation
- Combine multiple components for comprehensive testing

### Material Analysis Best Practices
- Analyze emission contribution separately
- Test radiance modulation effects
- Compare different material properties
- Test delta term contributions

### Lighting Analysis Best Practices
- Test different lighting models
- Compare direct vs. indirect illumination
- Analyze light contribution breakdown
- Use modulation for A/B testing

### Research and Development Best Practices
- Test new lighting techniques
- Compare different BSDF models
- Analyze delta term effects
- Develop new illumination models

### Educational Use
- Demonstrate illumination components
- Show radiance modulation
- Explain delta terms
- Teach material analysis
- Visualize lighting contributions

### Integration Best Practices
- Connect to appropriate render graph inputs
- Use correct texture formats
- Handle optional inputs/outputs
- Test with different render graphs
- Validate compatibility with other passes
