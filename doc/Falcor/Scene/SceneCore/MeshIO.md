# MeshIO - Mesh I/O Operations

## Module State Machine

**Status**: Complete

## Dependency Graph

### Sub-modules (Complete)

- [x] **MeshIO.cs.slang** - Mesh I/O compute shader

### External Dependencies

- **Scene/SceneTypes** - Scene type definitions
- **Scene/Scene** - Scene interface

## Module Overview

MeshIO provides compute shaders for mesh I/O operations including loading mesh data from the scene and updating mesh data in the scene. It supports reading vertex positions, texture coordinates, and triangle indices from the scene, and writing vertex positions, normals, tangents, and texture coordinates to the scene. The module uses parameter blocks for configuration and supports 16-bit and 32-bit index formats.

## Component Specifications

### MeshLoader Structure

**Purpose**: Load mesh data from the scene.

**Key Features**:
- Load triangle indices from scene
- Load vertex positions from scene
- Load texture coordinates from scene
- Support 16-bit and 32-bit index formats
- Vertex buffer and index buffer offset support
- Output to RWStructuredBuffer

**Fields**:
- [`vertexCount`](Source/Falcor/Scene/MeshIO.cs.slang:33) - `uint` - Number of vertices in the mesh
- [`vbOffset`](Source/Falcor/Scene/MeshIO.cs.slang:34) - `uint` - Vertex buffer offset in the scene
- [`triangleCount`](Source/Falcor/Scene/MeshIO.cs.slang:35) - `uint` - Number of triangles in the mesh
- [`ibOffset`](Source/Falcor/Scene/MeshIO.cs.slang:36) - `uint` - Index buffer offset in the scene
- [`use16BitIndices`](Source/Falcor/Scene/MeshIO.cs.slang:37) - `bool` - Whether to use 16-bit indices (true) or 32-bit indices (false)
- [`scene`](Source/Falcor/Scene/MeshIO.cs.slang:39) - `ParameterBlock<Scene>` - Scene parameter block for scene data access
- [`positions`](Source/Falcor/Scene/MeshIO.cs.slang:42) - `RWStructuredBuffer<float3>` - Output buffer for vertex positions
- [`texcrds`](Source/Falcor/Scene/MeshIO.cs.slang:43) - `RWStructuredBuffer<float3>` - Output buffer for texture coordinates (stored as float3 with z=0)
- [`triangleIndices`](Source/Falcor/Scene/MeshIO.cs.slang:44) - `RWStructuredBuffer<uint3>` - Output buffer for triangle indices

**Methods**:
- [`getMeshIndices(uint triangleId)`](Source/Falcor/Scene/MeshIO.cs.slang:46) - Get triangle indices from the scene
  - Returns early if triangleId >= triangleCount
  - Calls [`scene.getLocalIndices(ibOffset, triangleId, use16BitIndices)`](Source/Falcor/Scene/MeshIO.cs.slang:49) to get vertex indices
  - Stores indices in [`triangleIndices[triangleId]`](Source/Falcor/Scene/MeshIO.cs.slang:50)
  - Supports both 16-bit and 32-bit index formats

- [`getMeshVertexData(uint vertexId)`](Source/Falcor/Scene/MeshIO.cs.slang:53) - Get vertex data from the scene
  - Returns early if vertexId >= vertexCount
  - Calls [`scene.getVertex(vertexId + vbOffset)`](Source/Falcor/Scene/MeshIO.cs.slang:56) to get vertex data
  - Extracts position from vertex data: [`vtxData.position`](Source/Falcor/Scene/MeshIO.cs.slang:57)
  - Extracts texture coordinate from vertex data: [`vtxData.texCrd`](Source/Falcor/Scene/MeshIO.cs.slang:58)
  - Stores position in [`positions[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:57)
  - Stores texture coordinate in [`texcrds[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:58) (stored as float3 with z=0)

**Implementation Details**:
- Vertex data structure: [`StaticVertexData`](Source/Falcor/Scene/MeshIO.cs.slang:56)
- Vertex data contains: position (float3), texCrd (float2)
- Texture coordinates stored as float3 with z=0 for consistency
- Bounds checking: Returns early if index is out of range

### MeshUpdater Structure

**Purpose**: Update mesh data in the scene.

**Key Features**:
- Update vertex positions in scene
- Update vertex normals in scene
- Update vertex tangents in scene
- Update texture coordinates in scene
- Vertex buffer offset support
- Input from StructuredBuffer
- Output to RWSplitVertexBuffer

**Fields**:
- [`vertexCount`](Source/Falcor/Scene/MeshIO.cs.slang:64) - `uint` - Number of vertices in the mesh
- [`vbOffset`](Source/Falcor/Scene/MeshIO.cs.slang:65) - `uint` - Vertex buffer offset in the scene
- [`positions`](Source/Falcor/Scene/MeshIO.cs.slang:67) - `StructuredBuffer<float3>` - Input buffer for vertex positions
- [`normals`](Source/Falcor/Scene/MeshIO.cs.slang:68) - `StructuredBuffer<float3>` - Input buffer for vertex normals
- [`tangents`](Source/Falcor/Scene/MeshIO.cs.slang:69) - `StructuredBuffer<float3>` - Input buffer for vertex tangents
- [`texcrds`](Source/Falcor/Scene/MeshIO.cs.slang:70) - `StructuredBuffer<float3>` - Input buffer for texture coordinates
- [`vertexData`](Source/Falcor/Scene/MeshIO.cs.slang:73) - `RWSplitVertexBuffer` - Output buffer for vertex data in the scene

**Methods**:
- [`setMeshVertexData(uint vertexId)`](Source/Falcor/Scene/MeshIO.cs.slang:75) - Set vertex data in the scene
  - Returns early if vertexId >= vertexCount
  - Creates [`StaticVertexData`](Source/Falcor/Scene/MeshIO.cs.slang:78) structure
  - Sets position: [`vtxData.position = positions[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:79)
  - Sets normal: [`vtxData.normal = normals[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:80)
  - Sets tangent: [`vtxData.tangent = float4(tangents[vertexId], 1.f)`](Source/Falcor/Scene/MeshIO.cs.slang:81) - Tangent follows orientation such that `b = cross(n, t)`
  - Sets texture coordinate: [`vtxData.texCrd = texcrds[vertexId].xy`](Source/Falcor/Scene/MeshIO.cs.slang:82)
  - Packs vertex data: [`vertexData[vertexId + vbOffset].pack(vtxData)`](Source/Falcor/Scene/MeshIO.cs.slang:83)

**Implementation Details**:
- Vertex data structure: [`StaticVertexData`](Source/Falcor/Scene/MeshIO.cs.slang:78)
- Vertex data contains: position (float3), normal (float3), tangent (float4), texCrd (float2)
- Tangent stored as float4 with w=1.f for orientation
- Tangent orientation: `b = cross(n, t)` (bitangent = cross(normal, tangent))
- Texture coordinates extracted as .xy from float3 input
- Vertex data packed using [`pack()`](Source/Falcor/Scene/MeshIO.cs.slang:83) method
- Bounds checking: Returns early if index is out of range

### Compute Shaders

**getMeshVerticesAndIndices**

**Purpose**: Get mesh vertices and indices from the scene.

**Signature**:
```slang
[numthreads(256, 1, 1)]
void getMeshVerticesAndIndices(uint3 tid: SV_DispatchThreadID)
```

**Parameters**:
- [`tid`](Source/Falcor/Scene/MeshIO.cs.slang:91) - `uint3` - Dispatch thread ID (SV_DispatchThreadID)

**Implementation**:
- Calls [`meshLoader.getMeshIndices(tid.x)`](Source/Falcor/Scene/MeshIO.cs.slang:93) to get triangle indices
- Calls [`meshLoader.getMeshVertexData(tid.x)`](Source/Falcor/Scene/MeshIO.cs.slang:94) to get vertex data
- Thread group size: 256 threads
- Uses tid.x as the index for both triangles and vertices

**Thread Group Configuration**:
- Thread group size: 256 threads
- Dispatch dimensions: Calculated based on mesh size
- Each thread processes one triangle/vertex

**setMeshVertices**

**Purpose**: Set mesh vertices in the scene.

**Signature**:
```slang
[numthreads(256, 1, 1)]
void setMeshVertices(uint3 tid: SV_DispatchThreadID)
```

**Parameters**:
- [`tid`](Source/Falcor/Scene/MeshIO.cs.slang:98) - `uint3` - Dispatch thread ID (SV_DispatchThreadID)

**Implementation**:
- Calls [`meshUpdater.setMeshVertexData(tid.x)`](Source/Falcor/Scene/MeshIO.cs.slang:100) to set vertex data
- Thread group size: 256 threads
- Uses tid.x as the vertex index

**Thread Group Configuration**:
- Thread group size: 256 threads
- Dispatch dimensions: Calculated based on vertex count
- Each thread processes one vertex

### Global Parameter Blocks

**meshLoader**

**Purpose**: Global parameter block for MeshLoader.

**Type**: `ParameterBlock<MeshLoader>`

**Usage**:
- Configured on CPU side
- Contains mesh loading parameters
- Used by getMeshVerticesAndIndices compute shader

**meshUpdater**

**Purpose**: Global parameter block for MeshUpdater.

**Type**: `ParameterBlock<MeshUpdater>`

**Usage**:
- Configured on CPU side
- Contains mesh update parameters
- Used by setMeshVertices compute shader

## Technical Details

### Mesh Loading

**Index Loading**:
- Calls [`scene.getLocalIndices(ibOffset, triangleId, use16BitIndices)`](Source/Falcor/Scene/MeshIO.cs.slang:49)
- Supports 16-bit and 32-bit index formats
- Index buffer offset: ibOffset
- Triangle ID: triangleId
- Returns uint3 of vertex indices
- Stores in [`triangleIndices[triangleId]`](Source/Falcor/Scene/MeshIO.cs.slang:50)

**Vertex Data Loading**:
- Calls [`scene.getVertex(vertexId + vbOffset)`](Source/Falcor/Scene/MeshIO.cs.slang:56)
- Vertex buffer offset: vbOffset
- Vertex ID: vertexId
- Returns StaticVertexData structure
- Extracts position: [`vtxData.position`](Source/Falcor/Scene/MeshIO.cs.slang:57)
- Extracts texture coordinate: [`vtxData.texCrd`](Source/Falcor/Scene/MeshIO.cs.slang:58)
- Stores position in [`positions[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:57)
- Stores texture coordinate in [`texcrds[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:58) (stored as float3 with z=0)

**Bounds Checking**:
- Returns early if triangleId >= triangleCount
- Returns early if vertexId >= vertexCount
- Prevents out-of-bounds access
- Improves robustness

### Mesh Updating

**Vertex Data Construction**:
- Creates StaticVertexData structure
- Sets position: [`vtxData.position = positions[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:79)
- Sets normal: [`vtxData.normal = normals[vertexId]`](Source/Falcor/Scene/MeshIO.cs.slang:80)
- Sets tangent: [`vtxData.tangent = float4(tangents[vertexId], 1.f)`](Source/Falcor/Scene/MeshIO.cs.slang:81)
- Sets texture coordinate: [`vtxData.texCrd = texcrds[vertexId].xy`](Source/Falcor/Scene/MeshIO.cs.slang:82)

**Tangent Orientation**:
- Tangent stored as float4 with w=1.f
- Orientation: `b = cross(n, t)` (bitangent = cross(normal, tangent))
- Used for correct normal mapping
- Follows standard tangent space convention

**Vertex Data Packing**:
- Calls [`vertexData[vertexId + vbOffset].pack(vtxData)`](Source/Falcor/Scene/MeshIO.cs.slang:83)
- Packs vertex data into split vertex buffer
- Vertex buffer offset: vbOffset
- Vertex ID: vertexId
- Efficient storage of vertex attributes

**Bounds Checking**:
- Returns early if vertexId >= vertexCount
- Prevents out-of-bounds access
- Improves robustness

### Compute Shader Dispatch

**Thread Group Size**:
- Thread group size: 256 threads
- Configured via `[numthreads(256, 1, 1)]` attribute
- Optimized for GPU warp/wavefront size
- Balances thread count and register pressure

**Dispatch Thread ID**:
- Thread ID: `uint3 tid: SV_DispatchThreadID`
- Uses tid.x as the index
- Each thread processes one element

**getMeshVerticesAndIndices Dispatch**:
- Processes both triangles and vertices
- Each thread processes one triangle/vertex
- Dispatch dimensions: max(triangleCount, vertexCount) / 256
- May process more elements than needed (bounds checking handles this)

**setMeshVertices Dispatch**:
- Processes vertices only
- Each thread processes one vertex
- Dispatch dimensions: vertexCount / 256
- May process more vertices than needed (bounds checking handles this)

### Index Format Support

**16-bit Indices**:
- Supported when use16BitIndices = true
- More memory efficient
- Limited to 65535 indices
- Used for smaller meshes

**32-bit Indices**:
- Supported when use16BitIndices = false
- Less memory efficient
- Supports up to 4294967295 indices
- Used for larger meshes

**Index Buffer Offset**:
- Supports multiple meshes in single buffer
- Offset: ibOffset
- Allows efficient memory usage
- Enables mesh batching

### Vertex Buffer Offset

**Purpose**: Support multiple meshes in single buffer.

**Usage**:
- Offset: vbOffset
- Added to vertex ID: `vertexId + vbOffset`
- Allows efficient memory usage
- Enables mesh batching

**Loading**:
- Scene vertex ID: `vertexId + vbOffset`
- Local vertex ID: vertexId
- Output buffer index: vertexId

**Updating**:
- Scene vertex ID: `vertexId + vbOffset`
- Local vertex ID: vertexId
- Input buffer index: vertexId

### Texture Coordinate Storage

**Loading**:
- Extracted from StaticVertexData.texCrd (float2)
- Stored as float3 with z=0
- Output buffer: RWStructuredBuffer<float3>
- Consistency: All vertex data stored as float3

**Updating**:
- Input buffer: StructuredBuffer<float3>
- Extracted as .xy from float3
- Stored in StaticVertexData.texCrd (float2)
- z component ignored

**Rationale**:
- Consistency: All vertex data stored as float3
- Compatibility: Works with existing buffer formats
- Efficiency: No format conversion needed

### StaticVertexData Structure

**Fields**:
- [`position`](Source/Falcor/Scene/MeshIO.cs.slang:57) - `float3` - Vertex position
- [`normal`](Source/Falcor/Scene/MeshIO.cs.slang:80) - `float3` - Vertex normal
- [`tangent`](Source/Falcor/Scene/MeshIO.cs.slang:81) - `float4` - Vertex tangent (with w=1.f for orientation)
- [`texCrd`](Source/Falcor/Scene/MeshIO.cs.slang:58) - `float2` - Texture coordinates

**Usage**:
- Used for loading vertex data from scene
- Used for updating vertex data in scene
- Packed into split vertex buffer
- Standard vertex data format

## Integration Points

### Scene Integration

**Scene Parameter Block**:
- [`ParameterBlock<Scene> scene`](Source/Falcor/Scene/MeshIO.cs.slang:39) - Scene parameter block
- Provides access to scene data
- Used for loading vertex and index data
- Used for updating vertex data

**Scene Methods**:
- [`scene.getLocalIndices(ibOffset, triangleId, use16BitIndices)`](Source/Falcor/Scene/MeshIO.cs.slang:49) - Get local indices from scene
- [`scene.getVertex(vertexId + vbOffset)`](Source/Falcor/Scene/MeshIO.cs.slang:56) - Get vertex data from scene
- [`vertexData[vertexId + vbOffset].pack(vtxData)`](Source/Falcor/Scene/MeshIO.cs.slang:83) - Pack vertex data into scene

### Buffer Integration

**Output Buffers (MeshLoader)**:
- [`RWStructuredBuffer<float3> positions`](Source/Falcor/Scene/MeshIO.cs.slang:42) - Output buffer for vertex positions
- [`RWStructuredBuffer<float3> texcrds`](Source/Falcor/Scene/MeshIO.cs.slang:43) - Output buffer for texture coordinates
- [`RWStructuredBuffer<uint3> triangleIndices`](Source/Falcor/Scene/MeshIO.cs.slang:44) - Output buffer for triangle indices

**Input Buffers (MeshUpdater)**:
- [`StructuredBuffer<float3> positions`](Source/Falcor/Scene/MeshIO.cs.slang:67) - Input buffer for vertex positions
- [`StructuredBuffer<float3> normals`](Source/Falcor/Scene/MeshIO.cs.slang:68) - Input buffer for vertex normals
- [`StructuredBuffer<float3> tangents`](Source/Falcor/Scene/MeshIO.cs.slang:69) - Input buffer for vertex tangents
- [`StructuredBuffer<float3> texcrds`](Source/Falcor/Scene/MeshIO.cs.slang:70) - Input buffer for texture coordinates

**Output Buffer (MeshUpdater)**:
- [`RWSplitVertexBuffer vertexData`](Source/Falcor/Scene/MeshIO.cs.slang:73) - Output buffer for vertex data in scene

### Compute Shader Integration

**Thread Group Configuration**:
- Thread group size: 256 threads
- Dispatch thread ID: SV_DispatchThreadID
- Optimized for GPU warp/wavefront size

**Compute Shaders**:
- [`getMeshVerticesAndIndices()`](Source/Falcor/Scene/MeshIO.cs.slang:91) - Load mesh vertices and indices
- [`setMeshVertices()`](Source/Falcor/Scene/MeshIO.cs.slang:98) - Update mesh vertices

**Parameter Blocks**:
- [`ParameterBlock<MeshLoader> meshLoader`](Source/Falcor/Scene/MeshIO.cs.slang:87) - Mesh loader parameter block
- [`ParameterBlock<MeshUpdater> meshUpdater`](Source/Falcor/Scene/MeshIO.cs.slang:88) - Mesh updater parameter block

### SceneTypes Integration

**StaticVertexData Structure**:
- Used for vertex data storage
- Contains position, normal, tangent, texCrd
- Standard vertex data format
- Used throughout the scene system

## Architecture Patterns

### Parameter Block Pattern

- Uses parameter blocks for configuration
- Configured on CPU side
- Accessed from GPU shaders
- Efficient data transfer

### Compute Shader Pattern

- Uses compute shaders for parallel processing
- Thread group size: 256 threads
- Dispatch thread ID for indexing
- Optimized for GPU execution

### Buffer Pattern

- Uses structured buffers for data transfer
- RWStructuredBuffer for read/write access
- StructuredBuffer for read-only access
- Efficient memory access patterns

### Offset Pattern

- Uses buffer offsets for multiple meshes
- Vertex buffer offset: vbOffset
- Index buffer offset: ibOffset
- Enables mesh batching

### Bounds Checking Pattern

- Checks bounds before accessing buffers
- Returns early if index is out of range
- Prevents out-of-bounds access
- Improves robustness

## Code Patterns

### MeshLoader Pattern

```slang
struct MeshLoader
{
    uint vertexCount;
    uint vbOffset;
    uint triangleCount;
    uint ibOffset;
    bool use16BitIndices;

    ParameterBlock<Scene> scene;

    RWStructuredBuffer<float3> positions;
    RWStructuredBuffer<float3> texcrds;
    RWStructuredBuffer<uint3> triangleIndices;

    void getMeshIndices(uint triangleId)
    {
        if (triangleId >= triangleCount) return;
        uint3 vtxIndices = scene.getLocalIndices(ibOffset, triangleId, use16BitIndices);
        triangleIndices[triangleId] = vtxIndices;
    }

    void getMeshVertexData(uint vertexId)
    {
        if (vertexId >= vertexCount) return;
        StaticVertexData vtxData = scene.getVertex(vertexId + vbOffset);
        positions[vertexId] = vtxData.position;
        texcrds[vertexId] = float3(vtxData.texCrd, 0.f);
    }
};
```

### MeshUpdater Pattern

```slang
struct MeshUpdater
{
    uint vertexCount;
    uint vbOffset;

    StructuredBuffer<float3> positions;
    StructuredBuffer<float3> normals;
    StructuredBuffer<float3> tangents;
    StructuredBuffer<float3> texcrds;

    RWSplitVertexBuffer vertexData;

    void setMeshVertexData(uint vertexId)
    {
        if (vertexId >= vertexCount) return;
        StaticVertexData vtxData;
        vtxData.position = positions[vertexId];
        vtxData.normal = normals[vertexId];
        vtxData.tangent = float4(tangents[vertexId], 1.f);
        vtxData.texCrd = texcrds[vertexId].xy;
        vertexData[vertexId + vbOffset].pack(vtxData);
    }
};
```

### Compute Shader Pattern

```slang
[numthreads(256, 1, 1)]
void getMeshVerticesAndIndices(uint3 tid: SV_DispatchThreadID)
{
    meshLoader.getMeshIndices(tid.x);
    meshLoader.getMeshVertexData(tid.x);
}

[numthreads(256, 1, 1)]
void setMeshVertices(uint3 tid: SV_DispatchThreadID)
{
    meshUpdater.setMeshVertexData(tid.x);
}
```

### Bounds Checking Pattern

```slang
void getMeshIndices(uint triangleId)
{
    if (triangleId >= triangleCount) return;
    // Process triangle
}

void getMeshVertexData(uint vertexId)
{
    if (vertexId >= vertexCount) return;
    // Process vertex
}
```

## Use Cases

### Mesh Export

- **Vertex Export**:
  - Export vertex positions from scene
  - Export texture coordinates from scene
  - Export triangle indices from scene
  - Support 16-bit and 32-bit index formats

- **Mesh Batching**:
  - Export multiple meshes from single buffer
  - Use vertex buffer offset
  - Use index buffer offset
  - Efficient memory usage

### Mesh Import

- **Vertex Import**:
  - Import vertex positions to scene
  - Import vertex normals to scene
  - Import vertex tangents to scene
  - Import texture coordinates to scene

- **Mesh Batching**:
  - Import multiple meshes to single buffer
  - Use vertex buffer offset
  - Efficient memory usage

### Mesh Modification

- **Vertex Modification**:
  - Modify vertex positions
  - Modify vertex normals
  - Modify vertex tangents
  - Modify texture coordinates

- **Dynamic Updates**:
  - Update mesh data at runtime
  - Use compute shaders for parallel updates
  - Efficient GPU-side updates

### Mesh Analysis

- **Vertex Analysis**:
  - Read vertex positions for analysis
  - Read texture coordinates for analysis
  - Read triangle indices for analysis
  - GPU-side analysis

## Performance Considerations

### GPU Computation

**Thread Group Size**:
- Thread group size: 256 threads
- Optimized for GPU warp/wavefront size
- Balances thread count and register pressure
- Efficient GPU utilization

**Parallel Processing**:
- Each thread processes one vertex/triangle
- Parallel processing of all vertices/triangles
- O(1) per vertex/triangle
- Overall: O(n) for n vertices/triangles

**Memory Access**:
- Coalesced memory access patterns
- Structured buffers for efficient access
- RWStructuredBuffer for read/write access
- StructuredBuffer for read-only access

**Bounds Checking**:
- Early return for out-of-bounds indices
- Minimal overhead for valid indices
- Prevents out-of-bounds access
- Improves robustness

### Memory Usage

**MeshLoader Memory**:
- Output buffers: positions (vertexCount * 12 bytes), texcrds (vertexCount * 12 bytes), triangleIndices (triangleCount * 12 bytes)
- Total: ~36 bytes per vertex + 12 bytes per triangle
- Efficient memory usage

**MeshUpdater Memory**:
- Input buffers: positions (vertexCount * 12 bytes), normals (vertexCount * 12 bytes), tangents (vertexCount * 12 bytes), texcrds (vertexCount * 12 bytes)
- Total: ~48 bytes per vertex
- Efficient memory usage

**Vertex Buffer Offset**:
- Supports multiple meshes in single buffer
- Reduces memory fragmentation
- Enables mesh batching
- Efficient memory usage

### Optimization Opportunities

**Thread Group Size**:
- Adjust thread group size based on mesh size
- Optimize for specific GPU architecture
- Balance thread count and register pressure
- Profile for optimal performance

**Memory Access**:
- Use shared memory for frequently accessed data
- Optimize memory access patterns
- Minimize memory bandwidth
- Use texture memory for read-only data

**Dispatch Optimization**:
- Calculate dispatch dimensions based on mesh size
- Avoid over-dispatching
- Use exact dispatch dimensions
- Minimize thread waste

## Limitations

### Feature Limitations

- **Vertex Attributes**:
  - Only supports position, normal, tangent, texCrd
  - No support for other vertex attributes
  - Limited to StaticVertexData format

- **Index Format**:
  - Only supports 16-bit and 32-bit indices
  - No support for other index formats
  - Limited to standard index formats

- **Mesh Topology**:
  - Only supports triangle meshes
  - No support for other mesh topologies
  - Limited to triangle lists

### Performance Limitations

- **Bounds Checking**:
  - Bounds checking adds overhead
  - Early return for out-of-bounds indices
  - May impact performance for small meshes
  - Necessary for robustness

- **Thread Group Size**:
  - Fixed thread group size: 256 threads
  - May not be optimal for all mesh sizes
  - May cause thread waste for small meshes
  - May cause register pressure for large meshes

### Integration Limitations

- **Scene Coupling**:
  - Tightly coupled to Falcor scene system
  - Requires scene to be initialized
  - Not suitable for standalone use
  - Requires specific scene implementation

- **Buffer Format**:
  - Requires specific buffer formats
  - RWStructuredBuffer for output
  - StructuredBuffer for input
  - Not suitable for other buffer formats

### Debugging Limitations

- **Limited Error Reporting**:
  - No detailed error messages
  - No logging of failures
  - Difficult to debug mesh issues
  - No validation of input parameters

## Best Practices

### Mesh Loading

- **Bounds Checking**:
  - Always check bounds before accessing buffers
  - Use early return for out-of-bounds indices
  - Prevent out-of-bounds access
  - Improve robustness

- **Index Format**:
  - Use 16-bit indices for small meshes
  - Use 32-bit indices for large meshes
  - Choose appropriate format based on mesh size
  - Optimize memory usage

- **Buffer Offsets**:
  - Use vertex buffer offset for multiple meshes
  - Use index buffer offset for multiple meshes
  - Enable mesh batching
  - Efficient memory usage

### Mesh Updating

- **Vertex Data Construction**:
  - Construct StaticVertexData correctly
  - Set all vertex attributes
  - Use correct tangent orientation
  - Follow standard conventions

- **Tangent Orientation**:
  - Set tangent.w = 1.f for orientation
  - Follow orientation: b = cross(n, t)
  - Use correct tangent space convention
  - Ensure correct normal mapping

- **Texture Coordinates**:
  - Extract .xy from float3 input
  - Ignore z component
  - Store in StaticVertexData.texCrd (float2)
  - Follow standard conventions

### Compute Shader Dispatch

- **Thread Group Size**:
  - Use thread group size of 256 threads
  - Optimize for GPU warp/wavefront size
  - Balance thread count and register pressure
  - Profile for optimal performance

- **Dispatch Dimensions**:
  - Calculate dispatch dimensions based on mesh size
  - Avoid over-dispatching
  - Use exact dispatch dimensions
  - Minimize thread waste

- **Thread ID Usage**:
  - Use tid.x as the index
  - Each thread processes one element
  - Use bounds checking for safety
  - Prevent out-of-bounds access

### Performance Optimization

- **Memory Access**:
  - Use coalesced memory access patterns
  - Use structured buffers for efficient access
  - Minimize memory bandwidth
  - Optimize memory access patterns

- **Parallel Processing**:
  - Use compute shaders for parallel processing
  - Process all vertices/triangles in parallel
  - Optimize for GPU execution
  - Maximize GPU utilization

- **Buffer Management**:
  - Use vertex buffer offset for multiple meshes
  - Use index buffer offset for multiple meshes
  - Enable mesh batching
  - Efficient memory usage

### Debugging

- **Validate Input Parameters**:
  - Check vertex count is valid
  - Check triangle count is valid
  - Check buffer offsets are valid
  - Check buffer formats are correct

- **Visual Debugging**:
  - Visualize vertex positions
  - Visualize texture coordinates
  - Visualize triangle indices
  - Verify mesh data

- **Performance Profiling**:
  - Measure compute shader execution time
  - Profile memory access patterns
  - Identify bottlenecks
  - Optimize hot paths

## Progress Log

- **2026-01-07T23:59:00Z**: MeshIO analysis completed. Analyzed MeshIO.cs.slang (101 lines) containing MeshLoader and MeshUpdater structures, plus compute shaders for mesh I/O operations. Documented MeshLoader for loading mesh data (positions, texture coordinates, triangle indices) from the scene with support for 16-bit and 32-bit index formats, vertex buffer and index buffer offsets, and output to RWStructuredBuffer. Documented MeshUpdater for updating mesh data (positions, normals, tangents, texture coordinates) in the scene with vertex buffer offset support, input from StructuredBuffer, and output to RWSplitVertexBuffer. Documented compute shaders getMeshVerticesAndIndices and setMeshVertices with thread group size of 256 threads, bounds checking, and parallel processing. Created comprehensive technical specification covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.

## Completion Status

The MeshIO module is complete. All source files have been analyzed and documented with comprehensive technical specifications covering component specifications, technical details, integration points, architecture patterns, code patterns, use cases, performance considerations, limitations, and best practices.
