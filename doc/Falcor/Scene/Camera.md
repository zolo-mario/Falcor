# Camera - Camera System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Camera** - Camera implementation
- [x] **CameraController** - Camera controller base class
- [x] **OrbiterCameraController** - Orbit camera controller
- [x] **FirstPersonCameraController** - First-person camera controller
- [x] **SixDoFCameraController** - Six-DOF camera controller

### External Dependencies

- **Core/Object** - Base object class
- **Core/Program** - Shader program management (ShaderVar)
- **Utils/Math** - Mathematical utilities (Vector, Matrix, AABB, Ray)
- **Utils/SampleGenerators** - Sample pattern generators (CPUSampleGenerator)
- **Utils/UI** - GUI widgets
- **Utils/Timing** - CPU timing (CpuTimer)
- **Utils/InputTypes** - Input event types (MouseEvent, KeyboardEvent, GamepadState)
- **Scene/Animation** - Animation system (Animatable)

## Module Overview

The Camera system provides comprehensive camera support for the Falcor rendering framework. It handles camera parameters, projection matrices, view matrices, frustum culling, camera jittering, and multiple camera controllers for different interaction modes. The system supports both pinhole and orthographic cameras with depth-of-field effects.

## Component Specifications

### Camera

**File**: [`Camera.h`](Source/Falcor/Scene/Camera/Camera.h:1)

**Purpose**: Core camera class for view and projection matrix computation.

**Key Features**:
- Pinhole and orthographic camera models
- Depth-of-field support (aperture, focal distance)
- Camera jittering for anti-aliasing
- Frustum culling
- Persistent view and projection matrices
- Previous frame data for motion blur
- Camera bounds for movement constraints
- Animation integration

**Core Data Structures**:

**CameraData** (from CameraData.slang):
```cpp
struct CameraData {
    float3 posW;                    // Camera world-space position
    float3 target;                  // Camera world-space target (look-at point)
    float3 up;                      // Camera world-space up vector
    float focalLength;              // Focal length in mm
    float frameHeight;              // Film plane height in mm
    float frameWidth;               // Film plane width in mm
    float focalDistance;            // Focal distance in scene units (for DoF)
    float apertureRadius;            // Aperture radius in scene units (for DoF)
    float shutterSpeed;             // Shutter speed in seconds
    float ISOSpeed;                 // Film speed (ISO)
    float aspectRatio;              // Aspect ratio (width/height)
    float nearZ;                   // Near plane depth
    float farZ;                    // Far plane depth
    float jitterX;                  // Subpixel jitter X (divided by screen width)
    float jitterY;                  // Subpixel jitter Y (divided by screen height)
    
    // Matrices
    float4x4 viewMat;              // View matrix
    float4x4 projMat;              // Projection matrix
    float4x4 viewProjMat;          // View-projection matrix
    float4x4 viewProjMatNoJitter;  // View-projection matrix without jitter
    float4x4 invViewProj;          // Inverse view-projection matrix
    float4x4 prevViewMat;           // Previous frame view matrix
    float4x4 prevViewProjMatNoJitter; // Previous frame view-projection matrix without jitter
    
    // Frustum planes
    float4 frustumPlanes[6];       // Camera frustum planes
    
    // Previous frame data
    float3 prevPosW;                // Previous frame world position
    
    // Ray tracing vectors
    float3 cameraW;                 // Camera forward vector
    float3 cameraU;                 // Camera right vector
    float3 cameraV;                 // Camera up vector
};
```

**Changes Enum** (from [`Camera::Changes`](Source/Falcor/Scene/Camera/Camera.h:264)):
- **None** - No changes
- **Movement** - Position, target, or up changed
- **Exposure** - Focal distance, aperture, shutter speed, or ISO changed
- **FocalDistance** - Focal distance changed
- **Jitter** - Jitter changed
- **Frustum** - Focal length, aspect ratio, or depth range changed
- **Aperture** - Aperture radius changed
- **History** - Previous frame matrix changed (motion vectors)

**Core Methods**:

**Construction**:
- [`Camera()`](Source/Falcor/Scene/Camera/Camera.h:55) - Constructor with name
- [`create()`](Source/Falcor/Scene/Camera/Camera.h:53) - Static factory method

**Camera Parameters**:
- [`setName()`](Source/Falcor/Scene/Camera/Camera.h:60) - Set camera name
- [`getName()`](Source/Falcor/Scene/Camera/Camera.h:64) - Get camera name
- [`setAspectRatio()`](Source/Falcor/Scene/Camera/Camera.h:68) - Set aspect ratio
- [`getAspectRatio()`](Source/Falcor/Scene/Camera/Camera.h:72) - Get aspect ratio
- [`setFocalLength()`](Source/Falcor/Scene/Camera/Camera.h:76) - Set focal length in mm
- [`getFocalLength()`](Source/Falcor/Scene/Camera/Camera.h:80) - Get focal length in mm
- [`setFrameHeight()`](Source/Falcor/Scene/Camera/Camera.h:84) - Set film plane height in mm
- [`getFrameHeight()`](Source/Falcor/Scene/Camera/Camera.h:88) - Get film plane height in mm
- [`setFrameWidth()`](Source/Falcor/Scene/Camera/Camera.h:92) - Set film plane width in mm
- [`getFrameWidth()`](Source/Falcor/Scene/Camera/Camera.h:96) - Get film plane width in mm
- [`setFocalDistance()`](Source/Falcor/Scene/Camera/Camera.h:100) - Set focal distance in scene units
- [`getFocalDistance()`](Source/Falcor/Scene/Camera/Camera.h:104) - Get focal distance in scene units
- [`setApertureRadius()`](Source/Falcor/Scene/Camera/Camera.h:108) - Set aperture radius in scene units
- [`getApertureRadius()`](Source/Falcor/Scene/Camera/Camera.h:112) - Get aperture radius in scene units
- [`setShutterSpeed()`](Source/Falcor/Scene/Camera/Camera.h:116) - Set shutter speed in seconds
- [`getShutterSpeed()`](Source/Falcor/Scene/Camera/Camera.h:120) - Get shutter speed in seconds
- [`setISOSpeed()`](Source/Falcor/Scene/Camera/Camera.h:124) - Set film speed (ISO)
- [`getISOSpeed()`](Source/Falcor/Scene/Camera/Camera.h:128) - Get film speed (ISO)

**Transform**:
- [`setPosition()`](Source/Falcor/Scene/Camera/Camera.h:144) - Set world-space position
- [`getPosition()`](Source/Falcor/Scene/Camera/Camera.h:132) - Get world-space position
- [`setTarget()`](Source/Falcor/Scene/Camera/Camera.h:152) - Set world-space target
- [`getTarget()`](Source/Falcor/Scene/Camera/Camera.h:140) - Get world-space target
- [`setUpVector()`](Source/Falcor/Scene/Camera/Camera.h:148) - Set world-space up vector
- [`getUpVector()`](Source/Falcor/Scene/Camera/Camera.h:136) - Get world-space up vector

**Depth Range**:
- [`setDepthRange()`](Source/Falcor/Scene/Camera/Camera.h:156) - Set near and far planes
- [`setNearPlane()`](Source/Falcor/Scene/Camera/Camera.h:160) - Set near plane
- [`getNearPlane()`](Source/Falcor/Scene/Camera/Camera.h:164) - Get near plane
- [`setFarPlane()`](Source/Falcor/Scene/Camera/Camera.h:168) - Set far plane
- [`getFarPlane()`](Source/Falcor/Scene/Camera/Camera.h:172) - Get far plane

**Jitter**:
- [`setJitter()`](Source/Falcor/Scene/Camera/Camera.h:188) - Set subpixel jitter
- [`getJitterX()`](Source/Falcor/Scene/Camera/Camera.h:189) - Get jitter X
- [`getJitterY()`](Source/Falcor/Scene/Camera/Camera.h:190) - Get jitter Y
- [`setPatternGenerator()`](Source/Falcor/Scene/Camera/Camera.h:178) - Set jitter pattern generator
- [`getPatternGenerator()`](Source/Falcor/Scene/Camera/Camera.h:182) - Get pattern generator

**Matrices**:
- [`getViewMatrix()`](Source/Falcor/Scene/Camera/Camera.h:209) - Get view matrix
- [`getPrevViewMatrix()`](Source/Falcor/Scene/Camera/Camera.h:213) - Get previous frame view matrix
- [`getProjMatrix()`](Source/Falcor/Scene/Camera/Camera.h:217) - Get projection matrix
- [`getViewProjMatrix()`](Source/Falcor/Scene/Camera/Camera.h:221) - Get view-projection matrix
- [`getViewProjMatrixNoJitter()`](Source/Falcor/Scene/Camera/Camera.h:225) - Get view-projection matrix without jitter
- [`getInvViewProjMatrix()`](Source/Falcor/Scene/Camera/Camera.h:229) - Get inverse view-projection matrix
- [`setProjectionMatrix()`](Source/Falcor/Scene/Camera/Camera.h:233) - Set persistent projection matrix
- [`setViewMatrix()`](Source/Falcor/Scene/Camera/Camera.h:237) - Set persistent view matrix
- [`togglePersistentProjectionMatrix()`](Source/Falcor/Scene/Camera/Camera.h:242) - Toggle persistent projection matrix
- [`togglePersistentViewMatrix()`](Source/Falcor/Scene/Camera/Camera.h:243) - Toggle persistent view matrix

**Culling**:
- [`isObjectCulled()`](Source/Falcor/Scene/Camera/Camera.h:248) - Check if object should be culled

**Ray Tracing**:
- [`computeRayPinhole()`](Source/Falcor/Scene/Camera/Camera.h:205) - Compute camera ray for pinhole model
- [`computeScreenSpacePixelSpreadAngle()`](Source/Falcor/Scene/Camera/Camera.h:196) - Compute pixel spread angle

**Frame Management**:
- [`beginFrame()`](Source/Falcor/Scene/Camera/Camera.h:280) - Begin frame (store previous frame data)
- [`getChanges()`](Source/Falcor/Scene/Camera/Camera.h:284) - Get changes since last frame

**Shader Integration**:
- [`bindShaderData()`](Source/Falcor/Scene/Camera/Camera.h:252) - Bind camera data to shader var

**Animation**:
- [`updateFromAnimation()`](Source/Falcor/Scene/Camera/Camera.h:258) - Update camera from animation transform

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Camera/Camera.h:262) - Render camera UI

**Scripting**:
- [`getScript()`](Source/Falcor/Scene/Camera/Camera.h:286) - Get camera script
- [`dumpProperties()`](Source/Falcor/Scene/Camera/Camera.h:288) - Dump camera properties

## Technical Details

### Camera Model

**Pinhole Camera**:
- Standard perspective projection
- Focal length determines field of view
- Aspect ratio determines viewport shape
- Near/far planes determine depth range

**Orthographic Camera**:
- Zero focal length = infinite field of view
- Orthographic projection
- Half look-at length as viewport size

**Aspect Ratio Handling** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:99)):
```cpp
if (mPreserveHeight)
{
    // Set frame width based on height and aspect ratio
    mData.frameWidth = mData.frameHeight * mData.aspectRatio;
}
else
{
    // Set frame height based on width and aspect ratio
    mData.frameHeight = mData.frameWidth / mData.aspectRatio;
}
```

### View Matrix Computation

**Look-At Matrix** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:123)):
```cpp
mData.viewMat = math::matrixFromLookAt(mData.posW, mData.target, 
                                       mData.up, 
                                       math::Handedness::RightHanded);
```

**Components**:
- Position: Camera world-space position
- Target: Camera look-at point
- Up: Camera world-space up vector
- Handedness: Right-handed coordinate system

### Projection Matrix Computation

**Perspective Projection** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:133)):
```cpp
const float fovY = focalLengthToFovY(mData.focalLength, mData.frameHeight);
mData.projMat = math::perspective(fovY, mData.aspectRatio, 
                                    mData.nearZ, mData.farZ);
```

**Orthographic Projection** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:139)):
```cpp
const float halfLookAtLength = length(mData.posW - mData.target) * 0.5f;
mData.projMat = math::ortho(-halfLookAtLength, halfLookAtLength,
                               -halfLookAtLength, halfLookAtLength,
                               mData.nearZ, mData.farZ);
```

### Jitter

**Purpose**: Subpixel jittering for anti-aliasing.

**Jitter Matrix** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:148)):
```cpp
float4x4 jitterMat = math::matrixFromTranslation(
    float3(2.0f * mData.jitterX, 2.0f * mData.jitterY, 0.0f));
mData.projMat = mul(jitterMat, mData.projMat);
```

**Jitter Values**:
- Expressed as subpixel quantities divided by screen resolution
- Example: 0.5 / Width = half pixel offset
- Applied to projection matrix

**Pattern Generator** (from [`beginFrame()`](Source/Falcor/Scene/Camera/Camera.cpp:56)):
```cpp
if (mJitterPattern.pGenerator)
{
    float2 jitter = mJitterPattern.pGenerator->next();
    jitter *= mJitterPattern.scale;
    setJitterInternal(jitter.x, jitter.y);
}
```

### Frustum Culling

**Purpose**: Efficient culling of objects outside camera view.

**Frustum Planes** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:159)):
```cpp
float4x4 tempMat = transpose(mData.viewProjMat);
for (int i = 0; i < 6; i++)
{
    float4 plane = (i & 1) ? tempMat.getCol(i >> 1) 
                                     : -tempMat.getCol(i >> 1);
    if (i != 5) // Z range is [0, w]
    {
        plane += tempMat.getCol(3);
    }
    
    mFrustumPlanes[i].xyz = plane.xyz();
    mFrustumPlanes[i].sign = math::sign(mFrustumPlanes[i].xyz);
    mFrustumPlanes[i].negW = -plane.w;
}
```

**Frustum Test** (from [`isObjectCulled()`](Source/Falcor/Scene/Camera/Camera.cpp:246)):
```cpp
for (int plane = 0; plane < 6; plane++)
{
    float3 signedHalfExtent = 0.5f * box.extent() * mFrustumPlanes[plane].sign;
    float dr = dot(box.center() + signedHalfExtent, mFrustumPlanes[plane].xyz);
    isInside = isInside && (dr > mFrustumPlanes[plane].negW);
}
return !isInside;
```

### Ray Tracing

**Purpose**: Compute camera rays for ray tracing.

**Pinhole Ray** (from [`computeRayPinhole()`](Source/Falcor/Scene/Camera/Camera.cpp:303)):
```cpp
Ray ray;
ray.origin = mData.posW;

// Compute sample position in screen space
float2 p = (float2(pixel) + float2(0.5f, 0.5f)) / float2(frameDim);
if (applyJitter) p += float2(-mData.jitterX, mData.jitterY);

float2 ndc = float2(2.0f, -2.0f) * p + float2(-1.0f, 1.0f);

// Compute normalized ray direction
ray.dir = normalize(ndc.x * mData.cameraU + ndc.y * mData.cameraV + mData.cameraW);

float invCos = 1.f / dot(normalize(mData.cameraW), ray.dir);
ray.tMin = mData.nearZ * invCos;
ray.tMax = mData.farZ * invCos;

return ray;
```

**Camera Vectors** (from [`calculateCameraParameters()`](Source/Falcor/Scene/Camera/Camera.cpp:175)):
```cpp
mData.cameraW = normalize(mData.target - mData.posW) * mData.focalDistance;
mData.cameraU = normalize(cross(mData.cameraW, mData.up));
mData.cameraV = normalize(cross(mData.cameraU, mData.cameraW));

const float ulen = mData.focalDistance * std::tan(fovY * 0.5f) * mData.aspectRatio;
mData.cameraU *= ulen;
const float vlen = mData.focalDistance * std::tan(fovY * 0.5f);
mData.cameraV *= vlen;
```

### Frame Management

**Purpose**: Track changes and maintain previous frame data.

**Begin Frame** (from [`beginFrame()`](Source/Falcor/Scene/Camera/Camera.cpp:54)):
1. Apply jitter pattern if set
2. Calculate camera parameters
3. Store previous frame data on first frame
4. Compare with previous frame data
5. Set change flags

**Change Detection** (from [`beginFrame()`](Source/Falcor/Scene/Camera/Camera.cpp:72)):
```cpp
mChanges = is_set(mChanges, Changes::Movement | Changes::Frustum) 
             ? Changes::History : Changes::None;

if (any(mPrevData.posW != mData.posW)) mChanges |= Changes::Movement;
if (any(mPrevData.up != mData.up)) mChanges |= Changes::Movement;
if (any(mPrevData.target != mData.target)) mChanges |= Changes::Movement;
if (any(mPrevData.focalDistance != mData.focalDistance)) mChanges |= Changes::FocalDistance;
if (any(mPrevData.apertureRadius != mData.apertureRadius)) mChanges |= Changes::Aperture | Changes::Exposure;
if (any(mPrevData.shutterSpeed != mData.shutterSpeed)) mChanges |= Changes::Exposure;
if (any(mPrevData.ISOSpeed != mData.ISOSpeed)) mChanges |= Changes::Exposure;
if (any(mPrevData.focalLength != mData.focalLength)) mChanges |= Changes::Frustum;
if (any(mPrevData.aspectRatio != mData.aspectRatio)) mChanges |= Changes::Frustum;
if (any(mPrevData.nearZ != mData.nearZ)) mChanges |= Changes::Frustum;
if (any(mPrevData.farZ != mData.farZ)) mChanges |= Changes::Frustum;
if (any(mPrevData.frameHeight != mData.frameHeight)) mChanges |= Changes::Frustum;
if (any(mPrevData.frameWidth != mData.frameWidth)) mChanges |= Changes::Frustum;
if (any(mPrevData.jitterX != mData.jitterX)) mChanges |= Changes::Jitter;
if (any(mPrevData.jitterY != mData.jitterY)) mChanges |= Changes::Jitter;
```

### CameraController

**File**: [`CameraController.h`](Source/Falcor/Scene/Camera/CameraController.h:1)

**Purpose**: Base class for camera controllers.

**Key Features**:
- Abstract interface for camera control
- Input event handling (mouse, keyboard, gamepad)
- Camera speed control
- World up-direction control
- Camera bounds for movement constraints

**UpDirection Enum** (from [`CameraController::UpDirection`](Source/Falcor/Scene/Camera/CameraController.h:48)):
- **XPos** - X-axis positive
- **XNeg** - X-axis negative
- **YPos** - Y-axis positive
- **YNeg** - Y-axis negative
- **ZPos** - Z-axis positive
- **ZNeg** - Z-axis negative

**Core Methods**:

**Input Handling**:
- [`onMouseEvent()`](Source/Falcor/Scene/Camera/CameraController.h:57) - Handle mouse events
- [`onKeyEvent()`](Source/Falcor/Scene/Camera/CameraController.h:61) - Handle keyboard events
- [`onGamepadState()`](Source/Falcor/Scene/Camera/CameraController.h:65) - Handle gamepad state
- [`resetInputState()`](Source/Falcor/Scene/Camera/CameraController.h:91) - Reset input state

**Update**:
- [`update()`](Source/Falcor/Scene/Camera/CameraController.h:70) - Update camera position and orientation

**Configuration**:
- [`setUpDirection()`](Source/Falcor/Scene/Camera/CameraController.h:74) - Set world up-direction
- [`getUpDirection()`](Source/Falcor/Scene/Camera/CameraController.h:78) - Get world up-direction
- [`setCameraSpeed()`](Source/Falcor/Scene/Camera/CameraController.h:83) - Set camera speed
- [`getCameraSpeed()`](Source/Falcor/Scene/Camera/CameraController.h:87) - Get camera speed
- [`setCameraBounds()`](Source/Falcor/Scene/Camera/CameraController.h:96) - Set camera bounds

### OrbiterCameraController

**Purpose**: Orbit camera controller for model inspection.

**Controls**:
- Left mouse click + movement: Orbit around model
- Mouse wheel: Zoom in/out

**Core Methods**:

**Model Parameters** (from [`setModelParams()`](Source/Falcor/Scene/Camera/CameraController.cpp:60)):
```cpp
void setModelParams(const float3& center, float radius, float initialDistanceInRadius)
{
    mModelCenter = center;
    mModelRadius = radius;
    mCameraDistance = initialDistanceInRadius;
    mRotation = float3x3::identity();
    mbDirty = true;
}
```

**Mouse Events** (from [`onMouseEvent()`](Source/Falcor/Scene/Camera/CameraController.cpp:69)):
- **Wheel**: Adjust camera distance
- **ButtonDown**: Start orbit
- **ButtonUp**: Stop orbit
- **Move**: Rotate camera

**Update** (from [`update()`](Source/Falcor/Scene/Camera/CameraController.cpp:114)):
```cpp
if(mpCamera && mbDirty)
{
    mbDirty = false;
    mShouldRotate = false;
    mpCamera->setTarget(mModelCenter);
    
    float3 camPos = mModelCenter;
    camPos += mul(float3(0, 0, 1), mRotation) * mModelRadius * mCameraDistance;
    mpCamera->setPosition(camPos);
    
    float3 up(0, 1, 0);
    up = mul(up, mRotation);
    mpCamera->setUpVector(up);
    return true;
}
```

### FirstPersonCameraController

**Purpose**: First-person camera controller for navigation.

**Controls**:
- **W/S/A/D**: Move forward/backward/strafe left/right
- **Q/E**: Move down/up
- **Left mouse button + movement**: Rotate camera
- **Right mouse button + movement**: Roll camera (6-DOF only)
- **Shift**: Faster movement
- **Ctrl**: Slower movement

**Direction Enum** (from [`Direction`](Source/Falcor/Scene/Camera/CameraController.cpp:204)):
- **Forward** - Move forward
- **Backward** - Move backward
- **Right** - Strafe right
- **Left** - Strafe left
- **Up** - Move up
- **Down** - Move down

**Keyboard Events** (from [`onKeyEvent()`](Source/Falcor/Scene/Camera/CameraController.cpp:149)):
```cpp
switch(event.key)
{
    case Input::Key::W: mMovement[Direction::Forward] = keyPressed; break;
    case Input::Key::S: mMovement[Direction::Backward] = keyPressed; break;
    case Input::Key::A: mMovement[Direction::Right] = keyPressed; break;
    case Input::Key::D: mMovement[Direction::Left] = keyPressed; break;
    case Input::Key::Q: mMovement[Direction::Down] = keyPressed; break;
    case Input::Key::E: mMovement[Direction::Up] = keyPressed; break;
}
```

**Speed Modifiers** (from [`onKeyEvent()`](Source/Falcor/Scene/Camera/CameraController.cpp:187)):
```cpp
mSpeedModifier = 1.0f;
if (event.hasModifier(Input::Modifier::Ctrl)) mSpeedModifier = 0.25f;
else if (event.hasModifier(Input::Modifier::Shift)) mSpeedModifier = 10.0f;
```

**Gamepad Support** (from [`onGamepadState()`](Source/Falcor/Scene/Camera/CameraController.cpp:213)):
- Dead zone filtering
- Power curve for non-linear response
- Left stick: Movement
- Right stick: Rotation
- Triggers: Up/down movement

**Dead Zone** (from [`applyDeadZone()`](Source/Falcor/Scene/Camera/CameraController.cpp:195)):
```cpp
inline float applyDeadZone(const float v, const float deadZone)
{
    return v * std::max(length(v) - deadZone, 0.f) / (1.f - deadZone);
}
```

**Power Curve** (from [`applyPowerCurve()`](Source/Falcor/Scene/Camera/CameraController.cpp:203)):
```cpp
inline float applyPowerCurve(const float v, const float power)
{
    return std::pow(std::fabs(v), power) * (v < 0.f ? -1.f : 1.f);
}
```

**Update** (from [`update()`](Source/Falcor/Scene/Camera/CameraController.cpp:238)):
```cpp
float elapsedTime = std::min(0.1f, (float)mTimer.delta());

// Rotation
if (mShouldRotate || anyGamepadRotation)
{
    float3 camPos = mpCamera->getPosition();
    float3 camTarget = mpCamera->getTarget();
    float3 camUp = b6DoF ? mpCamera->getUpVector() : getUpVector();
    
    float3 viewDir = normalize(camTarget - camPos);
    
    if (mIsLeftButtonDown || anyGamepadRotation)
    {
        float3 sideway = cross(viewDir, normalize(camUp));
        float2 mouseRotation = mIsLeftButtonDown ? mMouseDelta * mSpeedModifier 
                                                   : float2(0.f);
        float2 gamepadRotation = anyGamepadRotation ? mGamepadRightStick * kGamepadRotationSpeed 
                                                      : float2(0.f);
        float2 rotation = mouseRotation + gamepadRotation;
        
        // Rotate around x-axis
        quatf qy = math::quatFromAngleAxis(rotation.y, sideway);
        float3x3 rotY = math::matrixFromQuat(qy);
        viewDir = mul(viewDir, rotY);
        camUp = mul(camUp, rotY);
        
        // Rotate around y-axis
        quatf qx = math::quatFromAngleAxis(rotation.x, camUp);
        float3x3 rotX = math::matrixFromQuat(qx);
        viewDir = mul(viewDir, rotX);
        
        mpCamera->setTarget(camPos + viewDir);
        mpCamera->setUpVector(camUp);
        dirty = true;
    }
}

// Movement
if (mMovement.any() || anyGamepadMovement)
{
    float3 movement(0, 0, 0);
    movement.z += mMovement.test(Direction::Forward) ? 1 : 0;
    movement.z += mMovement.test(Direction::Backward) ? -1 : 0;
    movement.x += mMovement.test(Direction::Left) ? 1 : 0;
    movement.x += mMovement.test(Direction::Right) ? -1 : 0;
    movement.y += mMovement.test(Direction::Up) ? 1 : 0;
    movement.y += mMovement.test(Direction::Down) ? -1 : 0;
    
    float3 camPos = mpCamera->getPosition();
    float3 camTarget = mpCamera->getTarget();
    float3 camUp = mpCamera->getUpVector();
    
    float3 viewDir = normalize(camTarget - camPos);
    float3 sideway = cross(viewDir, normalize(camUp));
    
    float curMove = mSpeedModifier * mSpeed * elapsedTime;
    camPos += movement.z * curMove * viewDir;
    camPos += movement.x * curMove * sideway;
    camPos += movement.y * curMove * camUp;
    
    if (mBounds.valid())
        camPos = clamp(camPos, mBounds.minPoint, mBounds.maxPoint);
    
    camTarget = camPos + viewDir;
    
    mpCamera->setPosition(camPos);
    mpCamera->setTarget(camTarget);
    dirty = true;
}
```

## Integration Points

### Scene Integration

**Animation System**:
- Camera inherits from Animatable
- Update from animation transforms
- Integration with scene graph

**Render Integration**:
- Camera data bound to shaders
- View-projection matrix for rendering
- Frustum culling for optimization

### Input Integration

**Mouse Events**:
- Button down/up tracking
- Movement delta tracking
- Wheel zoom support

**Keyboard Events**:
- Movement key tracking
- Modifier key support (Shift, Ctrl)
- Speed modification

**Gamepad Support**:
- Dead zone filtering
- Power curve for non-linear response
- Left/right stick support
- Trigger support

## Performance Optimizations

**Dirty Flagging**:
- Only recalculate matrices when dirty
- Efficient change tracking
- Lazy parameter computation

**Frustum Culling**:
- AABB vs. frustum test
- Efficient plane-based culling
- Early rejection of invisible objects

**Jitter Pattern**:
- Sample generator for anti-aliasing
- Subpixel jittering
- Temporal accumulation support

## Progress Log

- **2026-01-07T19:00:11Z**: Camera subsystem analysis completed. Analyzed Camera, CameraController, OrbiterCameraController, and FirstPersonCameraController classes. Documented camera models, view/projection matrices, jittering, frustum culling, and controller implementations. Ready to proceed to Curves subsystem analysis.

## Next Steps

Proceed to analyze Curves subsystem to understand curve tessellation and implementation.
