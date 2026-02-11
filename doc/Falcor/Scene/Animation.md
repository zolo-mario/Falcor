# Animation - Animation System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **Animation** - Animation data and interpolation
- [x] **AnimationController** - Animation controller with skinning and vertex caching
- [x] **AnimatedVertexCache** - Vertex cache for dynamic meshes and curves

### External Dependencies

- **Core/Object** - Base object class
- **Core/API** - Graphics API (Device, RenderContext, Buffer, ComputePass)
- **Utils/Math** - Mathematical utilities (Vector, Matrix, Quaternion)
- **Utils/UI** - GUI widgets
- **Scene/Scene** - Scene integration
- **Scene/SceneTypes** - Scene type definitions

## Module Overview

The Animation system provides comprehensive animation support for the Falcor rendering framework. It handles keyframe-based animation, skinning for skeletal meshes, vertex caching for dynamic meshes and curves, and integration with the scene graph. The system supports both CPU and GPU-based animation updates.

## Component Specifications

### Animation

**File**: [`Animation.h`](Source/Falcor/Scene/Animation/Animation.h:1)

**Purpose**: Core animation class for keyframe-based animation.

**Key Features**:
- Keyframe-based animation system
- Multiple interpolation modes (Linear, Hermite)
- Pre- and post-infinity behaviors
- Animation warping support
- Efficient keyframe lookup with caching
- Transform matrix composition

**Core Data Structures**:

**Keyframe** (from [`Animation::Keyframe`](Source/Falcor/Scene/Animation/Animation.h:63)):
```cpp
struct Keyframe {
    double time = 0;                  // Keyframe timestamp
    float3 translation = float3(0, 0, 0); // Translation vector
    float3 scaling = float3(1, 1, 1);    // Scaling vector
    quatf rotation = quatf::identity();   // Rotation quaternion
};
```

**Enums**:

**InterpolationMode** (from [`Animation::InterpolationMode`](Source/Falcor/Scene/Animation/Animation.h:49)):
- **Linear** - Linear interpolation between keyframes
- **Hermite** - Hermite spline interpolation (smooth curves)

**Behavior** (from [`Animation::Behavior`](Source/Falcor/Scene/Animation/Animation.h:55)):
- **Constant** - Hold constant outside keyframe range
- **Linear** - Linear extrapolation outside keyframe range
- **Cycle** - Loop animation continuously
- **Oscillate** - Ping-pong animation (forward then backward)

**Core Methods**:

**Construction**:
- [`Animation()`](Source/Falcor/Scene/Animation/Animation.h:78) - Constructor with name, node ID, and duration
- [`create()`](Source/Falcor/Scene/Animation/Animation.h:71) - Static factory method

**Keyframe Management**:
- [`addKeyframe()`](Source/Falcor/Scene/Animation/Animation.h:132) - Add keyframe (sorted by time)
- [`getKeyframe()`](Source/Falcor/Scene/Animation/Animation.h:139) - Get keyframe at specific time
- [`doesKeyframeExists()`](Source/Falcor/Scene/Animation/Animation.h:150) - Check if keyframe exists
- [`getKeyframes()`](Source/Falcor/Scene/Animation/Animation.h:144) - Get all keyframes

**Animation Control**:
- [`animate()`](Source/Falcor/Scene/Animation/Animation.h:156) - Compute animation transform at given time

**Configuration**:
- [`setPreInfinityBehavior()`](Source/Falcor/Scene/Animation/Animation.h:102) - Set behavior before first keyframe
- [`setPostInfinityBehavior()`](Source/Falcor/Scene/Animation/Animation.h:110) - Set behavior after last keyframe
- [`setInterpolationMode()`](Source/Falcor/Scene/Animation/Animation.h:118) - Set interpolation mode
- [`setEnableWarping()`](Source/Falcor/Scene/Animation/Animation.h:126) - Enable/disable warping
- [`setNodeID()`](Source/Falcor/Scene/Animation/Animation.h:90) - Set animated node ID

**Query**:
- [`getName()`](Source/Falcor/Scene/Animation/Animation.h:82) - Get animation name
- [`getNodeID()`](Source/Falcor/Scene/Animation/Animation.h:86) - Get animated node ID
- [`getDuration()`](Source/Falcor/Scene/Animation/Animation.h:94) - Get animation duration
- [`getPreInfinityBehavior()`](Source/Falcor/Scene/Animation/Animation.h:98) - Get pre-infinity behavior
- [`getPostInfinityBehavior()`](Source/Falcor/Scene/Animation/Animation.h:106) - Get post-infinity behavior
- [`getInterpolationMode()`](Source/Falcor/Scene/Animation/Animation.h:114) - Get interpolation mode
- [`isWarpingEnabled()`](Source/Falcor/Scene/Animation/Animation.h:122) - Check if warping enabled

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Animation/Animation.h:160) - Render animation UI

## Technical Details

### Animation Pipeline

**Keyframe Storage**:
- Keyframes stored in sorted vector by time
- Cached frame index for efficient lookup
- O(n) insertion, O(log n) lookup with caching

**Interpolation**:

**Linear Interpolation** (from [`interpolateLinear()`](Source/Falcor/Scene/Animation/Animation.cpp:86)):
```cpp
Keyframe interpolateLinear(const Keyframe& k0, const Keyframe& k1, float t)
```
- Translation: Linear interpolation (lerp)
- Scaling: Linear interpolation (lerp)
- Rotation: Spherical linear interpolation (slerp)
- Time: Linear interpolation

**Hermite Interpolation** (from [`interpolateHermite()`](Source/Falcor/Scene/Animation/Animation.cpp:96)):
```cpp
Keyframe interpolateHermite(const Keyframe& k0, const Keyframe& k1,
                          const Keyframe& k2, const Keyframe& k3, float t)
```
- Translation: Bezier Hermite spline interpolation
- Scaling: Linear interpolation between k1 and k2
- Rotation: Bezier Hermite slerp
- Time: Linear interpolation between k1 and k2

**Bezier Hermite Implementation** (from [`interpolateHermite()`](Source/Falcor/Scene/Animation/Animation.cpp:50)):
```cpp
float3 interpolateHermite(const float3& p0, const float3& p1,
                         const float3& p2, const float3& p3, float t)
{
    float3 b0 = p1;
    float3 b1 = p1 + (p2 - p0) * 0.5f / 3.f;
    float3 b2 = p2 - (p3 - p1) * 0.5f / 3.f;
    float3 b3 = p2;

    float3 q0 = lerp(b0, b1, t);
    float3 q1 = lerp(b1, b2, t);
    float3 q2 = lerp(b2, b3, t);

    float3 qq0 = lerp(q0, q1, t);
    float3 qq1 = lerp(q1, q2, t);

    return lerp(qq0, qq1, t);
}
```

**Animation Evaluation** (from [`Animation::animate()`](Source/Falcor/Scene/Animation/Animation.cpp:114)):
1. Calculate sample time (handle out-of-range times)
2. Determine if linear extrapolation needed
3. Interpolate keyframes
4. Compose transform matrix: T × R × S

**Infinity Behaviors**:

**Constant** (from [`calcSampleTime()`](Source/Falcor/Scene/Animation/Animation.cpp:237)):
```cpp
modifiedTime = std::clamp(currentTime, firstKeyframeTime, lastKeyframeTime);
```

**Cycle** (from [`calcSampleTime()`](Source/Falcor/Scene/Animation/Animation.cpp:240)):
```cpp
modifiedTime = firstKeyframeTime + std::fmod(currentTime - firstKeyframeTime, duration);
if (modifiedTime < firstKeyframeTime) modifiedTime += duration;
```

**Oscillate** (from [`calcSampleTime()`](Source/Falcor/Scene/Animation/Animation.cpp:245)):
```cpp
double offset = std::fmod(currentTime - firstKeyframeTime, 2 * duration);
if (offset < 0) offset += 2 * duration;
if (offset > duration) offset = 2 * duration - offset;
modifiedTime = firstKeyframeTime + offset;
```

### WARPING

**Purpose**: Enable cyclic keyframe access for looping animations.

**Implementation** (from [`adjacentFrame()`](Source/Falcor/Scene/Animation/Animation.cpp:177)):
```cpp
auto adjacentFrame = [this] (size_t frame, int32_t offset = 1)
{
    size_t count = mKeyframes.size();
    return mEnableWarping ? (frame + count + offset) % count
                           : std::clamp(frame + offset, (size_t)0, count - 1);
};
```

**Use Cases**:
- Seamless looping animations
- Hermite interpolation with fewer than 4 keyframes
- Continuous animation cycles

### TRANSFORM COMPOSITION

**Animation Transform** (from [`Animation::animate()`](Source/Falcor/Scene/Animation/Animation.cpp:150)):
```cpp
float4x4 T = math::matrixFromTranslation(interpolated.translation);
float4x4 R = math::matrixFromQuat(interpolated.rotation);
float4x4 S = math::matrixFromScaling(interpolated.scaling);
float4x4 transform = mul(mul(T, R), S); // T × R × S
```

**Order**: Translation → Rotation → Scaling

### AnimationController

**File**: [`AnimationController.h`](Source/Falcor/Scene/Animation/AnimationController.h:1)

**Purpose**: Manages animations, skinning, and vertex caches for scene.

**Key Features**:
- Multiple animation management
- Scene graph transform updates
- Skinning for skeletal meshes
- Vertex caching for dynamic meshes and curves
- GPU-based animation updates
- Efficient matrix updates
- Previous frame data for motion blur

**Core Data Structures**:

**Matrix Storage**:
- [`mLocalMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:166) - Local transform matrices
- [`mGlobalMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:167) - Global (world) transform matrices
- [`mInvTransposeGlobalMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:168) - Inverse transpose world matrices
- [`mMatricesChanged`](Source/Falcor/Scene/Animation/AnimationController.h:169) - Change flags per matrix

**Skinning Data**:
- [`mMeshBindMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:188) - Mesh bind matrices
- [`mSkinningMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:189) - Skinning matrices
- [`mInvTransposeSkinningMatrices`](Source/Falcor/Scene/Animation/AnimationController.h:190) - Inverse transpose skinning matrices

**GPU Buffers**:
- [`mpWorldMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:181) - World matrices buffer
- [`mpPrevWorldMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:182) - Previous world matrices buffer
- [`mpInvTransposeWorldMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:183) - Inverse transpose world matrices buffer
- [`mpPrevInvTransposeWorldMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:184) - Previous inverse transpose world matrices buffer
- [`mpSkinningMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:195) - Skinning matrices buffer
- [`mpMeshBindMatricesBuffer`](Source/Falcor/Scene/Animation/AnimationController.h:193) - Mesh bind matrices buffer
- [`mpSkinningVertexData`](Source/Falcor/Scene/Animation/AnimationController.h:197) - Skinning vertex data buffer
- [`mpPrevVertexData`](Source/Falcor/Scene/Animation/AnimationController.h:198) - Previous vertex data buffer

**Animation State**:
- [`mAnimations`](Source/Falcor/Scene/Animation/AnimationController.h:164) - List of animations
- [`mNodesEdited`](Source/Falcor/Scene/Animation/AnimationController.h:165) - Node edit flags
- [`mEnabled`](Source/Falcor/Scene/Animation/AnimationController.h:172) - Animation enabled flag
- [`mLoopAnimations`](Source/Falcor/Scene/Animation/AnimationController.h:177) - Global loop flag
- [`mTime`](Source/Falcor/Scene/Animation/AnimationController.h:174) - Current time
- [`mPrevTime`](Source/Falcor/Scene/Animation/AnimationController.h:175) - Previous time
- [`mGlobalAnimationLength`](Source/Falcor/Scene/Animation/AnimationController.h:178) - Global animation length

**Core Methods**:

**Construction**:
- [`AnimationController()`](Source/Falcor/Scene/Animation/AnimationController.h:55) - Constructor with device, scene, skinning data, and animations

**Animation Control**:
- [`animate()`](Source/Falcor/Scene/Animation/AnimationController.h:109) - Run animation system
- [`setEnabled()`](Source/Falcor/Scene/Animation/AnimationController.h:87) - Enable/disable animations
- [`setIsLooped()`](Source/Falcor/Scene/Animation/AnimationController.h:95) - Enable/disable global looping

**Vertex Cache Management**:
- [`addAnimatedVertexCaches()`](Source/Falcor/Scene/Animation/AnimationController.h:59) - Add vertex caches

**Query**:
- [`hasAnimations()`](Source/Falcor/Scene/Animation/AnimationController.h:63) - Check if has animations
- [`hasSkinnedMeshes()`](Source/Falcor/Scene/Animation/AnimationController.h:67) - Check if has skinned meshes
- [`hasAnimatedVertexCaches()`](Source/Falcor/Scene/Animation/AnimationController.h:71) - Check if has animated vertex caches
- [`hasAnimatedCurveCaches()`](Source/Falcor/Scene/Animation/AnimationController.h:75) - Check if has animated curve caches
- [`hasAnimatedMeshCaches()`](Source/Falcor/Scene/Animation/AnimationController.h:79) - Check if has animated mesh caches
- [`isEnabled()`](Source/Falcor/Scene/Animation/AnimationController.h:91) - Check if enabled
- [`isLooped()`](Source/Falcor/Scene/Animation/AnimationController.h:99) - Check if looped
- [`isMatrixChanged()`](Source/Falcor/Scene/Animation/AnimationController.h:113) - Check if matrix changed

**Matrix Access**:
- [`getLocalMatrices()`](Source/Falcor/Scene/Animation/AnimationController.h:118) - Get local matrices
- [`getGlobalMatrices()`](Source/Falcor/Scene/Animation/AnimationController.h:123) - Get global matrices
- [`getInvTransposeGlobalMatrices()`](Source/Falcor/Scene/Animation/AnimationController.h:127) - Get inverse transpose global matrices

**Buffer Access**:
- [`getPrevVertexData()`](Source/Falcor/Scene/Animation/AnimationController.h:136) - Get previous vertex data buffer
- [`getPrevCurveVertexData()`](Source/Falcor/Scene/Animation/AnimationController.h:141) - Get previous curve vertex data buffer

**Memory**:
- [`getMemoryUsageInBytes()`](Source/Falcor/Scene/Animation/AnimationController.h:145) - Get GPU memory usage

**UI**:
- [`renderUI()`](Source/Falcor/Scene/Animation/AnimationController.h:131) - Render animation controller UI

## Technical Details

### Animation Pipeline

**Update Flow** (from [`AnimationController::animate()`](Source/Falcor/Scene/Animation/AnimationController.cpp:195)):
1. Check for edited scene nodes
2. Calculate animation time (with looping)
3. Check for enable/disable transitions
4. Update local matrices from animations
5. Update world matrices (propagate through scene graph)
6. Upload changed matrices to GPU
7. Execute skinning pass
8. Update vertex caches

### Matrix Updates

**Local Matrix Update** (from [`updateLocalMatrices()`](Source/Falcor/Scene/Animation/AnimationController.cpp:292)):
```cpp
for (auto& pAnimation : mAnimations)
{
    NodeID nodeID = pAnimation->getNodeID();
    mLocalMatrices[nodeID.get()] = pAnimation->animate(time);
    mMatricesChanged[nodeID.get()] = true;
}
```

**World Matrix Update** (from [`updateWorldMatrices()`](Source/Falcor/Scene/Animation/AnimationController.cpp:303)):
```cpp
for (size_t i = 0; i < mGlobalMatrices.size(); i++)
{
    // Propagate matrix change flag to children
    if (sceneGraph[i].parent != NodeID::Invalid())
    {
        mMatricesChanged[i] = mMatricesChanged[i] ||
                           mMatricesChanged[sceneGraph[i].parent.get()];
    }

    if (!mMatricesChanged[i] && !updateAll) continue;

    mGlobalMatrices[i] = mLocalMatrices[i];

    if (sceneGraph[i].parent != NodeID::Invalid())
    {
        mGlobalMatrices[i] = mul(mGlobalMatrices[sceneGraph[i].parent.get()],
                                   mGlobalMatrices[i]);
    }

    mInvTransposeGlobalMatrices[i] = transpose(inverse(mGlobalMatrices[i]));

    if (mpSkinningPass)
    {
        mSkinningMatrices[i] = mul(mGlobalMatrices[i],
                                    sceneGraph[i].localToBindSpace);
        mInvTransposeSkinningMatrices[i] = transpose(inverse(mSkinningMatrices[i]));
    }
}
```

**Matrix Upload** (from [`uploadWorldMatrices()`](Source/Falcor/Scene/Animation/AnimationController.cpp:334)):
- Upload all matrices on first update
- Upload only changed matrices on subsequent updates
- Detect ranges of consecutive changed matrices for efficient upload

### Skinning

**Purpose**: GPU-based skeletal animation for skinned meshes.

**Skinning Pass** (from [`createSkinningPass()`](Source/Falcor/Scene/Animation/AnimationController.cpp:395)):
- Compute pass for vertex skinning
- Shader: [`Skinning.slang`](Source/Falcor/Scene/Animation/Skinning.slang:1)
- Supports up to 4 bones per vertex

**SkinnedVertexData** (from SceneTypes.slang):
```cpp
struct SkinnedVertexData {
    uint4 boneIDs;      // Bone indices (up to 4)
    float4 boneWeights;  // Bone weights (normalized)
};
```

**Skinning Matrices**:
- **Mesh Bind Matrix**: World transform at bind time
- **Local to Bind Space**: Inverse bind transform (for bones)
- **Skinning Matrix**: World × LocalToBindSpace

**Skinning Execution** (from [`executeSkinningPass()`](Source/Falcor/Scene/Animation/AnimationController.cpp:456)):
1. Update skinning matrices buffer
2. Update inverse transpose skinning matrices buffer
3. Bind static vertex data
4. Bind skinning vertex data
5. Bind previous vertex data (for motion blur)
6. Execute compute pass

### Vertex Caching

**Purpose**: Cache vertex data for dynamic meshes and curves.

**AnimatedVertexCache**:
- Manages vertex caches for meshes and curves
- Supports multiple tessellation modes
- Updates vertex data on GPU
- Provides previous vertex data for motion blur

**Cache Types**:
- **Mesh Cache**: Dynamic mesh vertex data
- **Curve Cache**: Dynamic curve vertex data

**Cache Update**:
1. Interpolate vertex positions
2. Upload to GPU
3. Update previous vertex data

### Performance Optimizations

**Matrix Caching**:
- Cached frame index for keyframe lookup
- Change flags per matrix
- Range-based matrix upload
- Swap buffers for previous frame data

**GPU Updates**:
- Structured buffers for matrix storage
- Compute pass for skinning
- Efficient buffer updates with range detection

**Memory Management**:
- Split buffers for vertex data
- Previous frame data for motion blur
- Lazy resource creation

## Integration Points

### Scene Integration

**Scene Graph**:
- Animation updates node transforms
- World matrices propagate through hierarchy
- Inverse transpose matrices for normals

**Material Integration**:
- Animation affects material transforms
- Previous vertex data for motion blur

**Render Integration**:
- Animation updates before rendering
- Matrix buffers bound to shaders
- Skinning pass executed before rasterization/ray tracing

## Progress Log

- **2026-01-07T18:58:56Z**: Animation subsystem analysis completed. Analyzed Animation and AnimationController classes. Documented keyframe interpolation, skinning, vertex caching, and matrix updates. Ready to proceed to Camera subsystem analysis.

## Next Steps

Proceed to analyze Camera subsystem to understand camera system and controllers.
