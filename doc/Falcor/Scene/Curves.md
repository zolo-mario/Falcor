# Curves - Curve System

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **AnimatedVertexCache** - Vertex cache for dynamic curves and meshes
- [x] **Curve Tessellation** - Curve tessellation shaders (UpdateCurveVertices, UpdateCurveAABBs, UpdateCurvePolyTubeVertices)

### External Dependencies

- **Core/API** - Graphics API (Device, RenderContext, Buffer, ComputePass)
- **Core/Program** - Shader program management (ComputePass)
- **Scene/Animation** - Animation system (Animation, AnimationController)
- **Scene/Scene** - Scene integration
- **Scene/SceneTypes** - Scene type definitions (CurveDesc, StaticCurveVertexData, DynamicCurveVertexData)
- **Utils/Math** - Mathematical utilities (Vector, Matrix)

## Module Overview

The Curves system provides comprehensive curve rendering support for Falcor rendering framework. It handles curve tessellation, dynamic vertex caching for animated curves, and integration with the scene graph. The system supports multiple tessellation modes including linear swept spheres and poly-tubes.

## Component Specifications

### Curve Data Structures

**CurveDesc** (from [`CurveDesc`](Source/Falcor/Scene/SceneTypes.slang:311)):
```cpp
struct CurveDesc {
    uint vbOffset;      ///< Offset into global curve vertex buffer.
    uint ibOffset;      ///< Offset into global curve index buffer.
    uint vertexCount;   ///< Vertex count.
    uint indexCount;    ///< Index count.
    uint degree;        ///< Polynomial degree of curve; linear (1) by default.
    uint materialID;    ///< Material ID.
    uint getSegmentCount() const { return indexCount; }
};
```

**StaticCurveVertexData** (from [`StaticCurveVertexData`](Source/Falcor/Scene/SceneTypes.slang:326)):
```cpp
struct StaticCurveVertexData {
    float3 position;    ///< Position.
    float radius;       ///< Radius of sphere at curve ends.
    float2 texCrd;      ///< Texture coordinates.
};
```

**DynamicCurveVertexData** (from [`DynamicCurveVertexData`](Source/Falcor/Scene/Animation/SharedTypes.slang)):
```cpp
struct DynamicCurveVertexData {
    float3 position;    ///< Position (animated).
};
```

### Curve Tessellation Modes

**CurveTessellationMode** (from [`CurveTessellationMode`](Source/Falcor/Scene/Curves/CurveConfig.h)):
- **LinearSweptSphere** - Linear swept spheres (default)
- **PolyTube** - Poly-tube tessellation

### AnimatedVertexCache

**File**: [`AnimatedVertexCache.h`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:1)

**Purpose**: Vertex cache for dynamic curves and meshes.

**Key Features**:
- Dynamic vertex caching for animated curves and meshes
- Multiple tessellation modes (LSS, poly-tube)
- GPU-based vertex updates
- Previous vertex data for motion blur
- Time-based interpolation
- Infinity behavior support
- Efficient buffer management

**Core Data Structures**:

**CachedCurve** (from [`CachedCurve`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:46)):
```cpp
struct CachedCurve {
    CurveTessellationMode tessellationMode = CurveTessellationMode::LinearSweptSphere;
    CurveOrMeshID geometryID;                              ///< ID of curve or mesh this data is animating.
    std::vector<double> timeSamples;                         ///< Time samples for keyframes.
    
    // Shared among all frames.
    // We assume topology doesn't change during animation.
    std::vector<uint32_t> indexData;                       ///< Index data (topology).
    
    // vertexData[i][j] represents at the i-th keyframe, cache data of j-th vertex.
    std::vector<std::vector<DynamicCurveVertexData>> vertexData;  ///< Vertex data per keyframe.
};
```

**CachedMesh** (from [`CachedMesh`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:63)):
```cpp
struct CachedMesh {
    MeshID meshID{ MeshID::kInvalidID };                 ///< ID of mesh this data is animating.
    std::vector<double> timeSamples;                         ///< Time samples for keyframes.
    
    // vertexData[i][j] represents at the i-th keyframe, cache data of j-th vertex.
    std::vector<std::vector<PackedStaticVertexData>> vertexData;  ///< Vertex data per keyframe.
};
```

**InterpolationInfo** (from [`calculateInterpolation()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:42)):
```cpp
struct InterpolationInfo {
    uint2 keyframeIndices;  ///< Indices of keyframes for interpolation.
    float t;                ///< Interpolation factor [0, 1].
};
```

**Core Methods**:

**Construction**:
- [`AnimatedVertexCache()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:76) - Constructor with device, scene, previous vertex data, and caches

**Animation Control**:
- [`setIsLooped()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:79) - Enable/disable global looping
- [`isLooped()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:81) - Check if looped
- [`setPreInfinityBehavior()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:83) - Set pre-infinity behavior
- [`hasAnimations()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:85) - Check if has animations
- [`hasCurveAnimations()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:87) - Check if has curve animations
- [`hasMeshAnimations()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:89) - Check if has mesh animations
- [`getGlobalAnimationLength()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:91) - Get global animation length

**Animation**:
- [`animate()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:93) - Animate curves and meshes
- [`copyToPrevVertices()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:95) - Copy current vertices to previous
- [`getPrevCurveVertexData()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:97) - Get previous curve vertex data buffer

**Memory**:
- [`getMemoryUsageInBytes()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.h:99) - Get GPU memory usage

## Technical Details

### Curve Initialization

**Curve Keyframe Initialization** (from [`initCurveKeyframes()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:200)):
```cpp
void initCurveKeyframes()
{
    // Align time samples across vertex caches.
    mCurveKeyframeTimes.clear();
    for (size_t i = 0; i < mCachedCurves.size(); i++)
    {
        mCurveKeyframeTimes.insert(mCurveKeyframeTimes.end(), 
                                 mCachedCurves[i].timeSamples.begin(), 
                                 mCachedCurves[i].timeSamples.end());
    }
    std::sort(mCurveKeyframeTimes.begin(), mCurveKeyframeTimes.end());
    mCurveKeyframeTimes.erase(std::unique(mCurveKeyframeTimes.begin(), 
                                       mCurveKeyframeTimes.end()), 
                          mCurveKeyframeTimes.end());
    mGlobalCurveAnimationLength = mCurveKeyframeTimes.empty() ? 0 : mCurveKeyframeTimes.back();
}
```

**Purpose**: Merge time samples from all cached curves into a unified timeline.

**Steps**:
1. Collect all time samples from cached curves
2. Insert into unified timeline
3. Sort time samples
4. Remove duplicates
5. Set global animation length

### LSS (Linear Swept Spheres) Tessellation

**Purpose**: Tessellate curves into linear swept spheres.

**Buffer Binding** (from [`bindCurveLSSBuffers()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:214)):
```cpp
void bindCurveLSSBuffers()
{
    // Compute curve vertex and index (segment) count.
    mCurveVertexCount = 0;
    mCurveIndexCount = 0;
    for (uint32_t i = 0; i < mCachedCurves.size(); i++)
    {
        if (mCachedCurves[i].tessellationMode != CurveTessellationMode::LinearSweptSphere) continue;
        
        mCurveVertexCount += (uint32_t)mCachedCurves[i].vertexData[0].size();
        mCurveIndexCount += (uint32_t)mCachedCurves[i].indexData.size();
    }
    
    // Create buffers for vertex positions in curve vertex caches.
    ResourceBindFlags vbBindFlags = ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess;
    mpCurveVertexBuffers.resize(mCurveKeyframeTimes.size());
    for (uint32_t i = 0; i < mCurveKeyframeTimes.size(); i++)
    {
        mpCurveVertexBuffers[i] = mpDevice->createStructuredBuffer(
            sizeof(DynamicCurveVertexData), 
            mCurveVertexCount, 
            vbBindFlags, 
            MemoryType::DeviceLocal, 
            nullptr, 
            false);
        mpCurveVertexBuffers[i]->setName("AnimatedVertexCache::mpCurveVertexBuffers[" + std::to_string(i) + "]");
    }
    
    // Create buffers for previous vertex positions.
    mpPrevCurveVertexBuffer = mpDevice->createStructuredBuffer(
        sizeof(DynamicCurveVertexData), 
        mCurveVertexCount, 
        vbBindFlags, 
        MemoryType::DeviceLocal, 
        nullptr, 
        false);
    mpPrevCurveVertexBuffer->setName("AnimatedVertexCache::mpPrevCurveVertexBuffer");
    
    // Initialize vertex buffers with cached positions.
    // [Implementation details for vertex buffer initialization...]
}
```

**Vertex Buffer Initialization**:
- Create vertex buffers for each keyframe
- Initialize with positions from first keyframe
- Create previous vertex buffer for motion blur

### Poly-Tube Tessellation

**Purpose**: Tessellate curves into poly-tubes.

**Buffer Binding** (from [`bindCurvePolyTubeBuffers()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:299)):
```cpp
void bindCurvePolyTubeBuffers()
{
    std::vector<PerCurveMetadata> curveMetadata;
    std::vector<PerMeshMetadata> meshMetadata;
    
    // Compute curve vertex and index (segment) count.
    mCurvePolyTubeVertexCount = 0;
    mCurvePolyTubeIndexCount = 0;
    for (uint32_t i = 0; i < mCachedCurves.size(); i++)
    {
        const auto& cache = mCachedCurves[i];
        
        if (cache.tessellationMode != CurveTessellationMode::PolyTube) continue;
        
        PerCurveMetadata curveMeta;
        curveMeta.indexCount = (uint32_t)cache.indexData.size();
        curveMeta.indexOffset = mCurvePolyTubeIndexCount;
        curveMeta.vertexCount = (uint32_t)cache.vertexData[0].size();
        curveMeta.vertexOffset = mCurvePolyTubeVertexCount;
        curveMetadata.push_back(curveMeta);
        
        PerMeshMetadata meshMeta;
        meshMeta.vertexCount = mpScene->getMesh(MeshID{ cache.geometryID }).vertexCount;
        meshMeta.sceneVbOffset = mpScene->getMesh(MeshID{ cache.geometryID }).vbOffset;
        meshMeta.prevVbOffset = mpScene->getMesh(MeshID{ cache.geometryID }).prevVbOffset;
        meshMetadata.push_back(meshMeta);
        
        mMaxCurvePolyTubeVertexCount = std::max(curveMeta.vertexCount, mMaxCurvePolyTubeVertexCount);
        mCurvePolyTubeVertexCount += curveMeta.vertexCount;
        mCurvePolyTubeIndexCount += curveMeta.indexCount;
    }
    
    // [Implementation details for metadata buffers...]
}
```

**Metadata Buffers**:
- Curve metadata buffer (index count, vertex count, vertex offset)
- Mesh metadata buffer (vertex count, scene VB offset, previous VB offset)
- Vertex buffers for each keyframe
- Strand index buffer for curve connectivity

### Interpolation

**Purpose**: Interpolate vertex positions between keyframes.

**Interpolation Calculation** (from [`calculateInterpolation()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:42)):
```cpp
InterpolationInfo calculateInterpolation(double time, 
                                        const std::vector<double>& timeSamples, 
                                        Animation::Behavior preInfinityBehavior, 
                                        Animation::Behavior postInfinityBehavior)
{
    if (!std::isfinite(time))
    {
        return InterpolationInfo{ uint2(0), 0.f };
    }
    
    // Clamp to positive
    time = std::max(time, 0.0);
    
    // Post-Infinity Behavior
    if (time > timeSamples.back())
    {
        if (postInfinityBehavior == Animation::Behavior::Constant)
        {
            time = timeSamples.back();
        }
        else if (postInfinityBehavior == Animation::Behavior::Cycle)
        {
            time = std::fmod(time, timeSamples.back());
        }
    }
    
    uint2 keyframeIndices;
    float t = 0.0f;
    
    // Pre-Infinity Behavior
    if (time <= timeSamples.front())
    {
        if (preInfinityBehavior == Animation::Behavior::Constant)
        {
            keyframeIndices = uint2(0);
            t = 0.f;
        }
        else if (preInfinityBehavior == Animation::Behavior::Cycle)
        {
            keyframeIndices.x = (uint32_t)timeSamples.size() - 1;
            keyframeIndices.y = 0;
            
            t = (float)(time / timeSamples.front());
        }
    }
    // Regular Interpolation
    else
    {
        keyframeIndices.y = uint32_t(std::lower_bound(timeSamples.begin(), 
                                                   timeSamples.end(), 
                                                   time) - timeSamples.begin());
        keyframeIndices.x = keyframeIndices.y - 1;
        FALCOR_ASSERT(timeSamples[keyframeIndices.y] > timeSamples[keyframeIndices.x]);
        
        t = (float)((time - timeSamples[keyframeIndices.x]) / 
                     (timeSamples[keyframeIndices.y] - timeSamples[keyframeIndices.x]));
    }
    
    return InterpolationInfo{ keyframeIndices, t };
}
```

**Infinity Behaviors**:
- **Constant**: Clamp to keyframe range
- **Cycle**: Loop animation continuously
- **Linear**: Linear extrapolation (not used for curves)

### Vertex Update Passes

**LSS Vertex Update** (from [`createCurveLSSVertexUpdatePass()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:493)):
```cpp
void createCurveLSSVertexUpdatePass()
{
    FALCOR_ASSERT(!mCachedCurves.empty());
    
    DefineList defines;
    defines.add("CURVE_KEYFRAME_COUNT", std::to_string(mCurveKeyframeTimes.size()));
    mpScene->getMeshStaticData().getShaderDefines(defines);
    
    mpCurveVertexUpdatePass = ComputePass::create(
        mpDevice, 
        "Scene/Animation/UpdateCurveVertices.slang", 
        "main", 
        defines);
    
    auto block = mpCurveVertexUpdatePass->getRootVar()["gCurveVertexUpdater"];
    auto var = block["curvePerKeyframe"];
    
    // Bind curve vertex data.
    for (uint32_t i = 0; i < mCurveKeyframeTimes.size(); i++)
        var[i]["vertexData"] = mpCurveVertexBuffers[i];
}
```

**LSS AABB Update** (from [`createCurveLSSAABBUpdatePass()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:509)):
```cpp
void createCurveLSSAABBUpdatePass()
{
    FALCOR_ASSERT(mCurveLSSCount > 0);
    
    mpCurveAABBUpdatePass = ComputePass::create(
        mpDevice, 
        "Scene/Animation/UpdateCurveAABBs.slang", 
        "main");
    
    auto block = mpCurveAABBUpdatePass->getRootVar()["gCurveAABBUpdater"];
    block["curveIndexData"] = mpCurveIndexBuffer;
}
```

**Poly-Tube Vertex Update** (from [`createCurvePolyTubeVertexUpdatePass()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:519)):
```cpp
void createCurvePolyTubeVertexUpdatePass()
{
    FALCOR_ASSERT(mCurvePolyTubeCount > 0);
    
    DefineList defines;
    defines.add("CURVE_KEYFRAME_COUNT", std::to_string(mCurveKeyframeTimes.size()));
    mpScene->getMeshStaticData().getShaderDefines(defines);
    
    mpCurvePolyTubeVertexUpdatePass = ComputePass::create(
        mpDevice, 
        "Scene/Animation/UpdateCurvePolyTubeVertices.slang", 
        "main", 
        defines);
    
    auto block = mpCurvePolyTubeVertexUpdatePass->getRootVar()["gCurvePolyTubeVertexUpdater"];
    
    // Bind curve vertex data.
    for (uint32_t i = 0; i < mCurveKeyframeTimes.size(); i++)
        var[i]["vertexData"] = mpCurvePolyTubeVertexBuffers[i];
}
```

### Animation Execution

**Purpose**: Animate curves and meshes.

**Animation Loop** (from [`animate()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:141)):
```cpp
bool AnimatedVertexCache::animate(RenderContext* pRenderContext, double time)
{
    if (!hasAnimations()) return false;
    
    if (!mCachedCurves.empty())
    {
        double curveTime = mLoopAnimations ? std::fmod(time, mGlobalCurveAnimationLength) : time;
        InterpolationInfo interpolationInfo = calculateInterpolation(
            curveTime, 
            mCurveKeyframeTimes, 
            mPreInfinityBehavior, 
            Animation::Behavior::Constant);
        
        if (mCurveLSSCount > 0)
        {
            executeCurveLSSVertexUpdatePass(pRenderContext, interpolationInfo);
            executeCurveLSSAABBUpdatePass(pRenderContext);
        }
        
        if (mCurvePolyTubeCount > 0)
        {
            executeCurvePolyTubeVertexUpdatePass(pRenderContext, interpolationInfo);
        }
    }
    
    if (!mCachedMeshes.empty())
    {
        executeMeshVertexUpdatePass(pRenderContext, time);
    }
    
    return true;
}
```

**LSS Vertex Update** (from [`executeCurveLSSVertexUpdatePass()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:561)):
```cpp
void executeCurveLSSVertexUpdatePass(RenderContext* pRenderContext, 
                                      const InterpolationInfo& info, 
                                      bool copyPrev)
{
    if (!mpCurveVertexUpdatePass) return;
    
    FALCOR_PROFILE(pRenderContext, "update curve vertices");
    
    auto block = mpCurveVertexUpdatePass->getRootVar()["gCurveVertexUpdater"];
    block["keyframeIndices"] = info.keyframeIndices;
    block["t"] = info.t;
    block["copyPrev"] = copyPrev;
    
    block["curveVertices"] = mpScene->mpRtVao->getVertexBuffer(0);
    block["prevCurveVertices"] = mpPrevCurveVertexBuffer;
    
    uint32_t dimX = (1 << 16);
    uint32_t dimY = (uint32_t)std::ceil((float)mCurveVertexCount / dimX);
    
    block["dimX"] = dimX;
    block["vertexCount"] = mCurveVertexCount;
    
    mpCurveVertexUpdatePass->execute(pRenderContext, dimX, dimY, 1);
}
```

**Poly-Tube Vertex Update** (from [`executeCurvePolyTubeVertexUpdatePass()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:600)):
```cpp
void executeCurvePolyTubeVertexUpdatePass(RenderContext* pRenderContext, 
                                      const InterpolationInfo& info, 
                                      bool copyPrev)
{
    if (!mpCurvePolyTubeVertexUpdatePass) return;
    
    FALCOR_PROFILE(pRenderContext, "Update curve poly-tube vertices");
    
    auto block = mpCurvePolyTubeVertexUpdatePass->getRootVar()["gCurvePolyTubeVertexUpdater"];
    block["keyframeIndices"] = info.keyframeIndices;
    block["t"] = info.t;
    block["copyPrev"] = copyPrev;
    
    block["perMeshData"] = mpCurvePolyTubeMeshMetadataBuffer;
    block["prevVertexData"] = mpPrevVertexData;
    
    block["vertexCount"] = mCurvePolyTubeVertexCount;
    block["indexCount"] = mCurvePolyTubeIndexCount;
    
    mpCurvePolyTubeVertexUpdatePass->execute(pRenderContext, 
                                          mMaxCurvePolyTubeVertexCount * 4, 
                                          mCurvePolyTubeCount, 
                                          1);
}
```

### Previous Vertex Data

**Purpose**: Copy current vertices to previous for motion blur.

**Copy to Previous** (from [`copyToPrevVertices()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:172)):
```cpp
void AnimatedVertexCache::copyToPrevVertices(RenderContext* pRenderContext)
{
    executeCurveLSSVertexUpdatePass(pRenderContext, InterpolationInfo{ uint2(0), 0.f }, true);
    executeCurvePolyTubeVertexUpdatePass(pRenderContext, InterpolationInfo{ uint2(0), 0.f }, true);
    executeMeshVertexUpdatePass(pRenderContext, 0.0f, true);
}
```

### Memory Management

**Memory Usage** (from [`getMemoryUsageInBytes()`](Source/Falcor/Scene/Animation/AnimatedVertexCache.cpp:185)):
```cpp
uint64_t AnimatedVertexCache::getMemoryUsageInBytes() const
{
    uint64_t m = 0;
    
    // Curve buffers
    for (size_t i = 0; i < mpCurveVertexBuffers.size(); i++)
        m += mpCurveVertexBuffers[i] ? mpCurveVertexBuffers[i]->getSize() : 0;
    m += mpPrevCurveVertexBuffer ? mpPrevCurveVertexBuffer->getSize() : 0;
    m += mpCurveIndexBuffer ? mpCurveIndexBuffer->getSize() : 0;
    
    // Mesh buffers
    for (size_t i = 0; i < mpMeshVertexBuffers.size(); i++)
        m += mpMeshVertexBuffers[i] ? mpMeshVertexBuffers[i]->getSize() : 0;
    m += mpMeshInterpolationBuffer ? mpMeshInterpolationBuffer->getSize() : 0;
    m += mpMeshMetadataBuffer ? mpMeshMetadataBuffer->getSize() : 0;
    
    return m;
}
```

## Integration Points

### Scene Integration

**Animation System**:
- AnimatedVertexCache integrates with AnimationController
- Time-based animation updates
- Interpolation between keyframes
- Previous frame data for motion blur

**Render Integration**:
- GPU-based vertex updates
- Compute passes for vertex interpolation
- AABB updates for ray tracing

**Material Integration**:
- Curve material binding
- Texture coordinate support

## Performance Optimizations

**Keyframe Alignment**:
- Unified timeline across all cached curves
- Sorted time samples for efficient lookup
- Duplicate removal for memory efficiency

**Buffer Management**:
- Separate buffers per keyframe
- Previous vertex buffer for motion blur
- Efficient buffer updates with range detection

**GPU Updates**:
- Compute passes for vertex interpolation
- Parallel vertex updates
- Efficient AABB updates for ray tracing

## Progress Log

- **2026-01-07T19:02:05Z**: Curves subsystem analysis completed. Analyzed AnimatedVertexCache, curve tessellation modes (LSS, poly-tube), vertex update passes, and interpolation system. Documented curve data structures, tessellation implementation, and GPU-based vertex updates. Ready to proceed to remaining Scene subsystems analysis.

## Next Steps

Proceed to analyze remaining Scene subsystems (Lights, Material, SDFs, Volume, SceneBuilder, and other utilities) to complete Scene module analysis.
