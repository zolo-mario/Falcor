# DisplacementUpdate - Displacement Update Compute Shader

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **DisplacementUpdate.cs.slang** - Displacement update compute shader (102 lines)

### External Dependencies

- **Scene/Scene.slang** - Scene module
- **Scene/Displacement/DisplacementMapping.slang** - Displacement mapping shaders
- **Utils/Math/AABB.slang** - AABB data structure
- **Scene/Displacement/DisplacementUpdateTask.slang** - Displacement update task structure

## Module Overview

DisplacementUpdate provides a compute shader for computing AABBs for displaced triangles. It processes displacement update tasks organized by mesh and triangle range, computing axis-aligned bounding boxes for displaced triangles. The shader uses a fixed thread count (256 threads) and processes triangles in fixed strides, supporting both precise and imprecise shell bounds calculation modes.

## Component Specifications

### Constants

- [`kUsePreciseShellBounds`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:33) - `static const bool` - Use precise shell bounds calculation (true)

### Constant Buffer

**Purpose**: Constant buffer for displacement update parameters.

**Fields**:
- [`gTaskCount`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:37) - `uint` - Number of displacement update tasks
- [`gTasks`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:39) - `StructuredBuffer<DisplacementUpdateTask>` - Displacement update tasks
- [`gAABBs`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:40) - `RWStructuredBuffer<AABB>` - Output AABB buffer

### Main Compute Shader

**Purpose**: This kernel is used for computing AABBs for displaced triangles.

**Work Organization**:
- Work is organized in tasks (described by DisplacementUpdateTask)
- Each task computes AABBs for a range of triangles from a single mesh
- A fixed number of threads (DisplacementUpdateTask::kThreadCount) is launched for each task
- Processing triangles in a fixed stride of kThreadCount

**Parameters**:
- [`dispatchThreadId`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:50) - `uint3` - Dispatch thread ID (SV_DispatchThreadID)
  - [`x`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:50) - Thread index within task
  - [`y`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:50) - Task index

**Local Variables**:
- [`threadIndex`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:52) - `uint` - Thread index within task
- [`taskIndex`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:53) - `uint` - Task index

**Early Exit**:
- Returns early if threadIndex >= kThreadCount or taskIndex >= gTaskCount

**Task Loading**:
- [`task`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:57) - `DisplacementUpdateTask` - Load task from gTasks[taskIndex]
- [`materialID`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:59) - `uint` - Material ID from task.meshID

**Displacement Data Loading**:
- [`displacementData`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:61) - `DisplacementData` - Load displacement data for materialID
  - Implementation: `gScene.materials.loadDisplacementData(materialID, displacementData);`

**Global Expansion**:
- [`globalExpansion`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:64) - `float` - Conservative global expansion
  - Implementation: `displacementData.getConservativeGlobalExpansion()`

**Iteration Calculation**:
- [`iterationCount`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:65) - `uint` - Number of iterations
  - Implementation: `(task.count + DisplacementUpdateTask::kThreadCount - 1) / DisplacementUpdateTask::kThreadCount`

**Main Loop**:
- Iterates over triangles in task range
- Computes AABB for each triangle
- Writes AABB to gAABBs

**Triangle Processing**:
- [`index`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:69) - `uint` - Triangle index within task
- [`triangleIndex`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:72) - `uint` - Global triangle index
  - Implementation: `iteration * DisplacementUpdateTask::kThreadCount + threadIndex`
- [`AABBIndex`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:73) - `uint` - AABB index in output buffer
  - Implementation: `task.AABBIndex + index`

**Early Exit**:
- Returns early if index >= task.count

**Index Loading**:
- [`indices`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:75) - `uint3` - Load triangle indices from scene
  - Implementation: `gScene.getIndices(task.meshID, triangleIndex)`

**Vertex Loading**:
- [`vertices`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:76) - `StaticVertexData[3]` - Load triangle vertices from scene
  - Implementation: `{ gScene.getVertex(indices[0]), gScene.getVertex(indices[1]), gScene.getVertex(indices[2]) }`

**AABB Computation**:
- [`aabb`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:78) - `AABB` - AABB for triangle
  - Initialize: `aabb.invalidate()`

**Imprecise Shell Bounds**:
- If `!kUsePreciseShellBounds`:
  - For each vertex (i = 0 to 2):
    - Include vertex position in AABB
    - Expand min point by globalExpansion
    - Expand max point by globalExpansion

**Precise Shell Bounds**:
- Else:
  - Compute shell min/max displacement for triangle
  - For each vertex (i = 0 to 2):
    - Include displaced vertex position in AABB
    - Displacement: `vertices[i].position + (vertices[i].normal * shellMinMax.x/y)`

**AABB Output**:
- [`gAABBs[AABBIndex]`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:100) - Write AABB to output buffer

## Technical Details

### Thread Organization

**Thread Group Size**:
- 256 threads per thread group
- 1 thread group in Y dimension
- 1 thread group in Z dimension
- `[numthreads(256, 1, 1)]` attribute

**Thread Indexing**:
- Thread index within task: `dispatchThreadId.x`
- Task index: `dispatchThreadId.y`
- Global thread index: `iteration * kThreadCount + threadIndex`

**Work Distribution**:
- Each task processes a range of triangles
- Threads within task process triangles in fixed stride
- Fixed stride: `kThreadCount` (256 triangles per iteration)

### AABB Computation

**Imprecise Shell Bounds**:
- Includes original vertex positions in AABB
- Expands min point by globalExpansion
- Expands max point by globalExpansion
- Faster but less accurate
- Does not account for displacement

**Precise Shell Bounds**:
- Computes shell min/max displacement for triangle
- Includes displaced vertex positions in AABB
- More accurate but slower
- Accounts for displacement in bounds calculation

**Shell Min/Max**:
- Computed using `displacementData.getShellMinMax()`
- Returns min/max displacement values
- Used for precise bounds calculation

### AABB Structure

**AABB Methods**:
- [`invalidate()`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:79) - Initialize AABB to invalid state
- [`include()`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:83) - Include point in AABB
- [`minPoint`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:85) - Min point of AABB
- [`maxPoint`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:86) - Max point of AABB

**AABB Expansion**:
- Min point: `minPoint - globalExpansion`
- Max point: `maxPoint + globalExpansion`

### Scene Access

**Index Loading**:
- [`gScene.getIndices(task.meshID, triangleIndex)`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:75) - Load triangle indices from scene
- Returns: `uint3` - Three vertex indices

**Vertex Loading**:
- [`gScene.getVertex(indices[i])`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:76) - Load vertex from scene
- Returns: `StaticVertexData` - Vertex data including position, normal, tangent, texture coordinate

**Material Loading**:
- [`gScene.materials.loadDisplacementData(materialID, displacementData)`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:62) - Load displacement data for material
- Returns: `DisplacementData` - Displacement data structure

### Displacement Data Access

**Global Expansion**:
- [`displacementData.getConservativeGlobalExpansion()`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:64) - Get conservative global expansion
- Returns: `float` - Maximum expansion distance

**Shell Min/Max**:
- [`displacementData.getShellMinMax(vertices[0].texCrd, vertices[1].texCrd, vertices[2].texCrd)`](Source/Falcor/Scene/Displacement/DisplacementUpdate.cs.slang:90) - Get shell min/max displacement for triangle
- Returns: `float2` - Min/max displacement values

## Integration Points

### Scene Integration

**Scene Access**:
- Access scene indices: `gScene.getIndices()`
- Access scene vertices: `gScene.getVertex()`
- Access scene materials: `gScene.materials.loadDisplacementData()`

**Material Integration**:
- Load displacement data for material
- Access displacement texture and samplers
- Access displacement parameters (scale, bias, size)

### Displacement Mapping Integration

**DisplacementData Extension**:
- Use `displacementData.getConservativeGlobalExpansion()`
- Use `displacementData.getShellMinMax()`
- Use displacement mapping functions

**AABB Integration**:
- Use AABB data structure for bounding box computation
- Write AABBs to output buffer
- Support for ray tracing and culling

### Task System Integration

**DisplacementUpdateTask Integration**:
- Load tasks from `gTasks` buffer
- Access task mesh ID, triangle index, AABB index, count
- Process triangles in task range

## Architecture Patterns

### Compute Shader Pattern

- Thread group-based parallel processing
- Fixed thread count per task
- Efficient memory access patterns
- Coalesced memory access for indices and vertices

### AABB Computation Pattern

- Axis-aligned bounding box computation
- Support for both precise and imprecise modes
- Efficient inclusion of points in AABB
- Conservative expansion for robustness

### Task Processing Pattern

- Task-based work distribution
- Fixed stride processing within tasks
- Early exit for out-of-bounds threads
- Efficient iteration over triangle range

## Code Patterns

### Main Compute Shader Pattern

```slang
[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint threadIndex = dispatchThreadId.x;
    const uint taskIndex = dispatchThreadId.y;

    if (threadIndex >= DisplacementUpdateTask::kThreadCount || taskIndex >= gTaskCount) return;

    const DisplacementUpdateTask task = gTasks[taskIndex];
    const uint materialID = gScene.meshes[task.meshID].materialID;

    DisplacementData displacementData;
    gScene.materials.loadDisplacementData(materialID, displacementData);

    const float globalExpansion = displacementData.getConservativeGlobalExpansion();
    const uint iterationCount = (task.count + DisplacementUpdateTask::kThreadCount - 1) / DisplacementUpdateTask::kThreadCount;

    for (uint iteration = 0; iteration < iterationCount; ++iteration)
    {
        const uint index = iteration * DisplacementUpdateTask::kThreadCount + threadIndex;
        if (index >= task.count) return;

        const uint triangleIndex = task.triangleIndex + index;
        const uint AABBIndex = task.AABBIndex + index;

        const uint3 indices = gScene.getIndices(task.meshID, triangleIndex);
        StaticVertexData vertices[3] = { gScene.getVertex(indices[0]), gScene.getVertex(indices[1]), gScene.getVertex(indices[2]) };

        AABB aabb;
        aabb.invalidate();

        if (!kUsePreciseShellBounds)
        {
            for (uint i = 0; i < 3; ++i) aabb.include(vertices[i].position);

            aabb.minPoint -= globalExpansion;
            aabb.maxPoint += globalExpansion;
        }
        else
        {
            const float2 shellMinMax = displacementData.getShellMinMax(vertices[0].texCrd, vertices[1].texCrd, vertices[2].texCrd);

            for (uint i = 0; i < 3; ++i)
            {
                aabb.include(vertices[i].position + (vertices[i].normal * shellMinMax.x));
                aabb.include(vertices[i].position + (vertices[i].normal * shellMinMax.y));
            }
        }

        gAABBs[AABBIndex] = aabb;
    }
}
```

## Use Cases

### Displacement AABB Computation

- **Precise Shell Bounds**:
  - Use precise shell bounds for accurate AABB computation
  - Compute shell min/max displacement for each triangle
  - Include displaced vertex positions in AABB
  - More accurate but slower

- **Imprecise Shell Bounds**:
  - Use imprecise shell bounds for faster AABB computation
  - Include original vertex positions in AABB
  - Expand AABB by global expansion
  - Faster but less accurate

### Task-Based Processing

- **Parallel AABB Computation**:
  - Process multiple triangles in parallel
  - Use fixed thread count for efficiency
  - Efficient memory access patterns

- **Work Distribution**:
  - Distribute work across tasks
  - Process triangles in fixed strides
  - Early exit for out-of-bounds threads

### Scene Integration

- **Scene Data Access**:
  - Access scene indices and vertices
  - Access material displacement data
  - Compute AABBs for displaced triangles

## Performance Considerations

### Memory Performance

**Coalesced Memory Access**:
- Efficient index and vertex loading
- Structured buffer access patterns
- Minimal memory overhead

**Thread Efficiency**:
- Fixed thread count (256) for efficiency
- Efficient work distribution
- Early exit for idle threads

### Computation Performance

**AABB Computation**:
- Efficient AABB inclusion
- Minimal branching
- Conservative expansion for robustness

**Mode Selection**:
- Imprecise mode: Faster, less accurate
- Precise mode: Slower, more accurate
- Runtime selection via kUsePreciseShellBounds

## Limitations

### Feature Limitations

- **Thread Count Limitations**:
- Fixed thread count (256) may not be optimal for all workloads
- May not utilize all GPU threads efficiently

- **AABB Limitations**:
- Axis-aligned AABBs only
- No support for oriented bounding boxes
- Conservative expansion may be too conservative

- **Mode Limitations**:
- Binary mode selection (precise/imprecise)
- No adaptive mode selection
- May not be optimal for all use cases

### Performance Limitations

- **Memory Limitations**:
- May have memory access conflicts
- May not be optimal for all GPU architectures
- May have cache inefficiencies

- **Computation Limitations**:
- Imprecise mode may be too conservative
- Precise mode may be too slow
- No adaptive mode selection

### Integration Limitations

- **Scene System Coupling**:
- Tightly coupled to scene system
- Requires specific scene data structures
- Not suitable for standalone use

## Best Practices

### Thread Organization

- **Thread Group Size**:
- Use appropriate thread group size for workloads
- Consider GPU architecture
- Balance thread count and work distribution

- **Early Exit**:
- Use early exit for idle threads
- Minimize unnecessary computations
- Improve performance

### AABB Computation

- **Mode Selection**:
- Use precise mode for accuracy
- Use imprecise mode for performance
- Choose mode based on use case

- **Conservative Expansion**:
- Use appropriate global expansion value
- Balance accuracy and performance
- Prevent precision issues

### Memory Access

- **Coalesced Access**:
- Use coalesced memory access patterns
- Minimize memory access conflicts
- Improve cache efficiency

- **Structured Buffers**:
- Use appropriate buffer layouts
- Minimize padding overhead
- Optimize for GPU performance

## Progress Log

- **2026-01-08T01:18:00Z**: DisplacementUpdate analysis completed. Analyzed DisplacementUpdate.cs.slang (102 lines) containing compute shader for computing AABBs for displaced triangles. Documented constant kUsePreciseShellBounds for controlling shell bounds calculation, constant buffer CB with gTaskCount, gTasks, and gAABBs, main compute shader with [numthreads(256, 1, 1)] attribute, dispatchThreadId parameter (SV_DispatchThreadID) with x (thread index) and y (task index), threadIndex and taskIndex local variables, early exit for out-of-bounds threads, task loading from gTasks, materialID from task.meshID, displacementData loading with gScene.materials.loadDisplacementData(), globalExpansion calculation with displacementData.getConservativeGlobalExpansion(), iterationCount calculation, main loop for triangle processing with index, triangleIndex, and AABBIndex local variables, early exit for index >= task.count, indices loading with gScene.getIndices(), vertices loading with gScene.getVertex(), AABB initialization with aabb.invalidate(), imprecise shell bounds calculation (including original vertex positions, expanding min/max points by globalExpansion), precise shell bounds calculation (computing shell min/max with displacementData.getShellMinMax(), including displaced vertex positions with vertices[i].position + (vertices[i].normal * shellMinMax.x/y)), AABB output to gAABBs[AABBIndex], imports (Scene/Scene, Scene/Displacement/DisplacementMapping, Utils/Math/AABB, Scene/Displacement/DisplacementUpdateTask), compute shader pattern with thread group-based parallel processing, AABB computation pattern, task processing pattern, integration points (scene, material, displacement mapping, AABB, task system), use cases (displacement AABB computation, task-based processing, scene integration), performance considerations (memory performance, thread efficiency, computation performance, mode selection), limitations (thread count limitations, AABB limitations, mode limitations, memory limitations, computation limitations, integration limitations), and best practices (thread organization, AABB computation, mode selection, conservative expansion, memory access, structured buffers). Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The DisplacementUpdate module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
