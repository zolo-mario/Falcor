# Lights - Scene Light Management System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Light** - Base light class
- [x] **PointLight** - Point light source
- [x] **DirectionalLight** - Directional light source
- [x] **DistantLight** - Distant light source
- [x] **AnalyticAreaLight** - Analytic area light base class
- [x] **RectLight** - Rectangular area light source
- [x] **DiscLight** - Disc area light source
- [x] **SphereLight** - Sphere area light source

### External Dependencies

- **Core/Object** - Base object class
- **Core/Macros** - Macro definitions
- **Core/Program** - Shader program management (ShaderVar)
- **Utils/Math** - Mathematical utilities (Vector, Matrix)
- **Utils/UI** - GUI widgets
- **Utils/Color** - Color utilities (ColorHelpers)
- **Scene/Animation** - Animation system (Animatable)
- **Scene/SceneCache** - Scene caching (SceneCache)
- **Utils/Logger** - Logging utilities
- **Utils/Scripting** - Script bindings (ScriptBindings, ScriptWriter)

## Module Overview

The Lights system provides comprehensive light management for Falcor rendering framework. It supports multiple light types including point, directional, distant, and analytic area lights with various shapes (rect, disc, sphere). The system integrates with the animation system for dynamic light updates and provides shader data binding for rendering.

## Component Specifications

### Light

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:1)

**Purpose**: Base class for all light sources.

**Key Features**:
- Animation integration via Animatable
- Shader data binding
- Active/inactive state management
- Intensity control
- Change tracking for scene updates
- UI rendering for light controls
- Power calculation for light picking

**Core Data Structures**:

**LightData** (from LightData.slang):
```cpp
struct LightData {
    float3 posW;           ///< World-space position
    float3 dirW;           ///< World-space direction
    float3 intensity;        ///< Light intensity (RGB)
    float3 tangent;         ///< Light tangent
    float3 bitangent;        ///< Light bitangent
    float4 transMat;         ///< Object-to-world transformation matrix
    float4 transMatIT;      ///< Inverse transpose of transformation matrix
};
```

**LightType Enum** (from LightData.slang):
- **Point** - Point light
- **Directional** - Directional light
- **AnalyticArea** - Analytic area light
- **Rect** - Rectangular area light
- **Disc** - Disc area light
- **Sphere** - Sphere area light

**Changes Enum** (from [`Light::Changes`](Source/Falcor/Scene/Lights/Light.h:99)):
- **None** - No changes
- **Active** - Light active state changed
- **Position** - Light position changed
- **Direction** - Light direction changed
- **Intensity** - Light intensity changed
- **SurfaceArea** - Light surface area changed (opening angle, penumbra angle, cosSubtendedAngle, surfaceArea)

**Core Methods**:

**Construction**:
- [`Light()`](Source/Falcor/Scene/Lights/Light.h:120) - Constructor with name and type
- [`~Light()`](Source/Falcor/Scene/Lights/Light.h:49) - Virtual destructor

**State Management**:
- [`setActive()`](Source/Falcor/Scene/Lights/Light.h:81) - Set active state
- [`isActive()`](Source/Falcor/Scene/Lights/Light.h:85) - Check if active
- [`beginFrame()`](Source/Falcor/Scene/Lights/Light.h:111) - Begin new frame
- [`getChanges()`](Source/Falcor/Scene/Lights/Light.h:115) - Get changes from previous frame

**Intensity Control**:
- [`setIntensity()`](Source/Falcor/Scene/Lights/Light.h:93) - Set light intensity
- [`getIntensity()`](Source/Falcor/Scene/Lights/Light.h:97) - Get light intensity

**Data Access**:
- [`getType()`](Source/Falcor/Scene/Lights/Light.h:65) - Get light type
- [`getData()`](Source/Falcor/Scene/Lights/Light.h:69) - Get light data
- [`getShaderStructSize()`](Source/Falcor/Scene/Lights/Light.h:89) - Get shader struct size

**Naming**:
- [`setName()`](Source/Falcor/Scene/Lights/Light.h:73) - Set light name
- [`getName()`](Source/Falcor/Scene/Lights/Light.h:77) - Get light name

**Shader Integration**:
- [`bindShaderData()`](Source/Falcor/Scene/Lights/Light.h:53) - Bind light data to shader var

**Animation**:
- [`updateFromAnimation()`](Source/Falcor/Scene/Lights/Light.h:117) - Update from animation transform

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Lights/Light.h:57) - Render UI elements

**Protected Members**:
- [`mName`](Source/Falcor/Scene/Lights/Light.h:130) - Light name
- [`mActive`](Source/Falcor/Scene/Lights/Light.h:131) - Active state
- [`mActiveChanged`](Source/Falcor/Scene/Lights/Light.h:132) - Active state changed flag
- [`mUiLightIntensityColor`](Source/Falcor/Scene/Lights/Light.h:135) - UI color for intensity
- [`mUiLightIntensityScale`](Source/Falcor/Scene/Lights/Light.h:136) - UI intensity scale
- [`mData`](Source/Falcor/Scene/Lights/Light.h:137) - Current light data
- [`mPrevData`](Source/Falcor/Scene/Lights/Light.h:138) - Previous frame light data
- [`mChanges`](Source/Falcor/Scene/Lights/Light.h:139) - Change flags
- [`mHasAnimation`](Source/Falcor/Scene/Lights/Light.h:145) - Animation flag

**Constants**:
- [`kDataSize`](Source/Falcor/Scene/Lights/Light.h:122) - Size of LightData struct

### PointLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:147)

**Purpose**: Point light with quadratic attenuation.

**Key Features**:
- World-space position and direction
- Opening angle for spot light conversion
- Penumbra angle for spot light conversion
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:150) - Static factory method
- [`PointLight()`](Source/Falcor/Scene/Lights/Light.h:152) - Constructor with name

**Position Control**:
- [`setWorldPosition()`](Source/Falcor/Scene/Lights/Light.h:165) - Set world-space position
- [`getWorldPosition()`](Source/Falcor/Scene/Lights/Light.h:179) - Get world-space position

**Direction Control**:
- [`setWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:170) - Set world-space direction
- [`getWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:183) - Get world-space direction

**Spot Light Parameters**:
- [`setOpeningAngle()`](Source/Falcor/Scene/Lights/Light.h:175) - Set cone opening half-angle
- [`getOpeningAngle()`](Source/Falcor/Scene/Lights/Light.h:196) - Get cone opening half-angle
- [`setPenumbraAngle()`](Source/Falcor/Scene/Lights/Light.h:192) - Set penumbra half-angle
- [`getPenumbraAngle()`](Source/Falcor/Scene/Lights/Light.h:187) - Get penumbra half-angle

**Power Calculation** (from [`getPower()`](Source/Falcor/Scene/Lights/Light.h:189)):
```cpp
return luminance(mData.intensity) * 4.f * (float)M_PI;
```

**Animation**:
- [`updateFromAnimation()`](Source/Falcor/Scene/Lights/Light.h:198) - Update from animation transform

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Lights/Light.h:157) - Render UI elements

### DirectionalLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:204)

**Purpose**: Directional light source.

**Key Features**:
- World-space direction
- Scene parameters (center, radius)
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:207) - Static factory method
- [`DirectionalLight()`](Source/Falcor/Scene/Lights/Light.h:209) - Constructor with name

**Direction Control**:
- [`setWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:217) - Set world-space direction
- [`getWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:227) - Get world-space direction

**Scene Parameters**:
- [`setWorldParams()`](Source/Falcor/Scene/Lights/Light.h:222) - Set scene parameters (center, radius)

**Power Calculation** (from [`getPower()`](Source/Falcor/Scene/Lights/Light.h:230)):
```cpp
return 0.f; // Directional lights have no power (infinite)
```

**Animation**:
- [`updateFromAnimation()`](Source/Falcor/Scene/Lights/Light.h:232) - Update from animation transform

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Lights/Light.h:214) - Render UI elements

### DistantLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:239)

**Purpose**: Distant light source (directional with infinite distance).

**Key Features**:
- World-space direction
- Half-angle subtended by light
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:242) - Static factory method
- [`DistantLight()`](Source/Falcor/Scene/Lights/Light.h:244) - Constructor with name

**Direction Control**:
- [`setWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:263) - Set world-space direction
- [`getWorldDirection()`](Source/Falcor/Scene/Lights/Light.h:267) - Get world-space direction

**Angle Control**:
- [`setAngle()`](Source/Falcor/Scene/Lights/Light.h:254) - Set half-angle
- [`getAngle()`](Source/Falcor/Scene/Lights/Light.h:258) - Get half-angle

**Power Calculation** (from [`getPower()`](Source/Falcor/Scene/Lights/Light.h:270)):
```cpp
return 0.f; // Distant lights have no power (infinite)
```

**Animation**:
- [`updateFromAnimation()`](Source/Falcor/Scene/Lights/Light.h:274) - Update from animation transform

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Lights/Light.h:249) - Render UI elements

### AnalyticAreaLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:284)

**Purpose**: Base class for analytic area lights.

**Key Features**:
- Scaling control (x, y, z)
- Transform matrix control
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`AnalyticAreaLight()`](Source/Falcor/Scene/Lights/Light.h:314) - Constructor with name and type

**Scaling Control**:
- [`setScaling()`](Source/Falcor/Scene/Lights/Light.h:292) - Set scaling factors
- [`getScaling()`](Source/Falcor/Scene/Lights/Light.h:296) - Get scaling factors

**Transform Control**:
- [`setTransformMatrix()`](Source/Falcor/Scene/Lights/Light.h:305) - Set transform matrix
- [`getTransformMatrix()`](Source/Falcor/Scene/Lights/Light.h:308) - Get transform matrix

**Power Calculation** (from [`getPower()`](Source/Falcor/Scene/Lights/Light.h:300)):
```cpp
return luminance(mData.intensity) * 4.f * (float)M_PI * mData.surfaceArea;
```

**Animation**:
- [`update()`](Source/Falcor/Scene/Lights/Light.h:311) - Update transformation matrices

**Protected Members**:
- [`mScaling`](Source/Falcor/Scene/Lights/Light.h:319) - Scaling factors
- [`mTransformMatrix`](Source/Falcor/Scene/Lights/Light.h:320) - Transform matrix minus scaling
- [`mTransformMatrixIT`](Source/Falcor/Scene/Lights/Light.h:321) - Inverse transpose of transform matrix

### RectLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:326)

**Purpose**: Rectangular area light source.

**Key Features**:
- Rectangular dimensions (rx, ry)
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:330) - Static factory method
- [`RectLight()`](Source/Falcor/Scene/Lights/Light.h:332) - Constructor with name

**Animation**:
- [`update()`](Source/Falcor/Scene/Lights/Light.h:335) - Update transformation matrices

**Protected Members**:
- [`mRx`](Source/Falcor/Scene/Lights/Light.h:369) - X dimension
- [`mRy`](Source/Falcor/Scene/Lights/Light.h:370) - Y dimension

### DiscLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:340)

**Purpose**: Disc area light source.

**Key Features**:
- Disc dimensions (rx, ry)
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:344) - Static factory method
- [`DiscLight()`](Source/Falcor/Scene/Lights/Light.h:346) - Constructor with name

**Animation**:
- [`update()`](Source/Falcor/Scene/Lights/Light.h:378) - Update transformation matrices

**Protected Members**:
- [`mRx`](Source/Falcor/Scene/Lights/Light.h:375) - X dimension
- [`mRy`](Source/Falcor/Scene/Lights/Light.h:376) - Y dimension

### SphereLight

**File**: [`Light.h`](Source/Falcor/Scene/Lights/Light.h:354)

**Purpose**: Sphere area light source.

**Key Features**:
- Sphere dimensions (rx, ry, rz)
- Power calculation for light picking
- Animation integration

**Core Methods**:

**Construction**:
- [`create()`](Source/Falcor/Scene/Lights/Light.h:357) - Static factory method
- [`SphereLight()`](Source/Falcor/Scene/Lights/Light.h:359) - Constructor with name

**Animation**:
- [`update()`](Source/Falcor/Scene/Lights/Light.h:390) - Update transformation matrices

**Protected Members**:
- [`mRx`](Source/Falcor/Scene/Lights/Light.h:395) - X dimension
- [`mRy`](Source/Falcor/Scene/Lights/Light.h:396) - Y dimension
- [`mRz`](Source/Falcor/Scene/Lights/Light.h:397) - Z dimension

## Technical Details

### Light Data Structure

**LightData Components**:
- **posW**: World-space position
- **dirW**: World-space direction (normalized)
- **intensity**: Light intensity (RGB color)
- **tangent**: Light tangent vector
- **bitangent**: Light bitangent vector
- **transMat**: Object-to-world transformation matrix
- **transMatIT**: Inverse transpose of transformation matrix

### Change Tracking

**Change Detection** (from [`Light::beginFrame()`](Source/Falcor/Scene/Lights/Light.cpp:55)):
```cpp
mChanges = Changes::None;

if (mActiveChanged) mChanges |= Changes::Active;
if (any(mPrevData.posW != mData.posW)) mChanges |= Changes::Position;
if (any(mPrevData.dirW != mData.dirW)) mChanges |= Changes::Direction;
if (any(mPrevData.intensity != mData.intensity)) mChanges |= Changes::Intensity;
if (mPrevData.openingAngle != mData.openingAngle) mChanges |= Changes::SurfaceArea;
if (mPrevData.penumbraAngle != mData.penumbraAngle) mChanges |= Changes::SurfaceArea;
if (mPrevData.cosSubtendedAngle != mData.cosSubtendedAngle) mChanges |= Changes::SurfaceArea;
if (mPrevData.surfaceArea != mData.surfaceArea) mChanges |= Changes::SurfaceArea;
if (mPrevData.transMat != mData.transMat) mChanges |= Changes::Position | Changes::Direction;

mPrevData = mData;
mActiveChanged = false;

return getChanges();
```

**Change Flags**:
- **None**: No changes
- **Active**: Light active state changed
- **Position**: Light position changed
- **Direction**: Light direction changed
- **Intensity**: Light intensity changed
- **SurfaceArea**: Light surface area changed (opening angle, penumbra angle, cosSubtendedAngle, surfaceArea)

### Shader Data Binding

**Shader Binding** (from [`Light::bindShaderData()`](Source/Falcor/Scene/Lights/Light.cpp:77)):
```cpp
var.setBlob(mData);
```

**Shader Struct Size**:
- **kDataSize**: Size of LightData struct (16 bytes)

### Animation Integration

**Transform Update** (from [`Light::updateFromAnimation()`](Source/Falcor/Scene/Lights/Light.h:117)):
```cpp
void Light::updateFromAnimation(const float4x4& transform)
{
    // Base implementation - no transform needed for lights
}
```

### Power Calculation

**PointLight Power** (from [`PointLight::getPower()`](Source/Falcor/Scene/Lights/Light.cpp:189)):
```cpp
return luminance(mData.intensity) * 4.f * (float)M_PI;
```

**AnalyticAreaLight Power** (from [`AnalyticAreaLight::getPower()`](Source/Falcor/Scene/Lights/Light.cpp:353)):
```cpp
return luminance(mData.intensity) * 4.f * (float)M_PI * mData.surfaceArea;
```

**DirectionalLight Power**:
```cpp
return 0.f; // Directional lights have no power (infinite)
```

**DistantLight Power**:
```cpp
return 0.f; // Distant lights have no power (infinite)
```

### UI Integration

**Light UI** (from [`Light::renderUI()`](Source/Falcor/Scene/Lights/Light.cpp:140)):
```cpp
bool active = isActive();
if (widget.checkbox("Active", active)) setActive(active);

if (mHasAnimation) widget.checkbox("Animated", mIsAnimated);

float3 color = getColorForUI();
if (widget.rgbColor("Color", color))
{
    setColorFromUI(color);
}

float intensity = getIntensityForUI();
if (widget.var("Intensity", intensity))
{
    setIntensityFromUI(intensity);
}
```

**Color Helpers** (from [`Light::getColorForUI()`](Source/Falcor/Scene/Lights/Light.cpp:88)):
```cpp
if (any((mUiLightIntensityColor * mUiLightIntensityScale) != mData.intensity))
{
    float mag = std::max(mData.intensity.x, std::max(mData.intensity.y, mData.intensity.z));
    if (mag <= 1.f)
    {
        mUiLightIntensityColor = mData.intensity;
        mUiLightIntensityScale = 1.0f;
    }
    else
    {
        mUiLightIntensityColor = mData.intensity / mag;
        mUiLightIntensityScale = mag;
    }
}
return mUiLightIntensityColor;
```

**Intensity Scaling** (from [`Light::setIntensityFromUI()`](Source/Falcor/Scene/Lights/Light.cpp:134)):
```cpp
mUiLightIntensityScale = intensity;
setIntensity(mUiLightIntensityColor * mUiLightIntensityScale);
```

## Integration Points

### Animation System

**Integration**:
- Light inherits from Animatable
- Transform updates from animation system
- Consistent with other animatable objects

### Scene Integration

**Scene Graph**:
- Light position and direction updates
- Change tracking for scene updates
- Integration with scene graph transforms

**Render Integration**:
- Light data bound to shaders
- Consistent shader binding across all light types
- Power calculation for light picking

### Performance Optimizations

**Change Tracking**:
- Efficient change detection
- Bit flag operations for multiple changes
- Previous frame data comparison

**Shader Binding**:
- Direct blob binding to shader variables
- Consistent struct size across all lights
- Efficient shader data updates

**Power Calculation**:
- Luminance-based power calculation
- Surface area integration for analytic lights
- Efficient power estimation for light picking

## Progress Log

- **2026-01-07T19:04:09Z**: Lights subsystem analysis completed. Analyzed Light base class and all light types (PointLight, DirectionalLight, DistantLight, AnalyticAreaLight, RectLight, DiscLight, SphereLight). Documented light data structures, change tracking, shader integration, power calculation, and UI integration. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (Material, SDFs, Volume, TriangleMesh, SceneCache, Transform, VertexAttrib, SceneTypes, SceneIDs) to complete Scene module analysis.
