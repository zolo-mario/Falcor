# Mesh Shader Stage

> **Relevant source files**
> * [src/config.h](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h)
> * [src/shaders/clustersubmit.comp.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/clustersubmit.comp.glsl)
> * [src/shaders/meshlet.mesh.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl)
> * [src/shaders/tasksubmit.comp.glsl](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/tasksubmit.comp.glsl)

## Purpose and Scope

This document describes the mesh shader stage of Niagara's GPU-driven rendering pipeline. The mesh shader is responsible for transforming vertices, culling triangles, and emitting final geometry for rasterization. It receives visible meshlets from either the task shader stage (see [Task Shader Stage](/zeux/niagara/7.2-task-shader-stage)) or from compute-based cluster submission, processes vertices in parallel, and outputs primitives with interpolated attributes.

For information about the earlier culling stages, see [Draw Culling Pipeline](/zeux/niagara/7.1-draw-culling-pipeline) and [Task Shader Stage](/zeux/niagara/7.2-task-shader-stage). For information about the subsequent rasterization and G-buffer generation, see [G-Buffer Generation](/zeux/niagara/8.2-g-buffer-generation).

---

## Mesh Shader Overview

The mesh shader stage processes individual meshlets that have survived earlier culling stages. It operates as the final programmable stage before rasterization, transforming vertices from object space to clip space and optionally performing per-triangle culling to eliminate degenerate or sub-pixel triangles.

### Key Responsibilities

| Responsibility | Description |
| --- | --- |
| **Vertex Transformation** | Transforms vertex positions using object-to-world transformations and view-projection matrices |
| **Attribute Processing** | Unpacks and transforms normals and tangents, passes through texture coordinates |
| **Triangle Assembly** | Reads meshlet indices and emits primitive topology |
| **Triangle Culling** | Optionally culls backfacing, zero-area, and sub-pixel triangles |
| **Output Generation** | Writes vertex attributes and primitive indices for rasterization |

### Execution Modes

The mesh shader supports two execution modes controlled by the `TASK` constant:

* **TASK=true**: Receives meshlet indices from the task shader via task payload
* **TASK=false**: Reads meshlet indices directly from a cluster indices buffer

**Sources:** [src/shaders/meshlet.mesh.glsl L1-L207](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L1-L207)

---

## Data Flow Architecture

```

```

**Diagram: Mesh Shader Data Flow**

**Sources:** [src/shaders/meshlet.mesh.glsl L89-L206](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L89-L206)

---

## Input Data Structures

The mesh shader accesses multiple buffer bindings to retrieve geometry data:

### Buffer Bindings

| Binding | Buffer | Data Structure | Purpose |
| --- | --- | --- | --- |
| 0 | `TaskCommands` | `MeshTaskCommand[]` | Maps cluster indices to draw IDs and meshlet offsets |
| 1 | `Draws` | `MeshDraw[]` | Per-instance transformation and material data |
| 2 | `Meshlets` | `Meshlet[]` | Meshlet descriptors with vertex/triangle counts and data offsets |
| 3 | `MeshletData*` | `uint[]`, `uint16_t[]`, `uint8_t[]` | Packed vertex references and triangle indices |
| 4 | `Vertices` | `Vertex[]` | Vertex position, normal, tangent, and UV data |
| 5 | `ClusterIndices` | `uint[]` | Visible cluster indices (only when TASK=false) |

### Cluster Index Format

The cluster index (`ci`) is a 32-bit packed value retrieved at [src/shaders/meshlet.mesh.glsl L94](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L94-L94)

:

```javascript
bits 0-23:  Command index into TaskCommands buffer
bits 24-31: Meshlet offset within the command's meshlet range
```

This encoding allows a single `uint` to reference both the draw command and the specific meshlet within that draw's meshlet range.

### Workgroup ID Mapping

When `TASK=false`, the 3D workgroup ID is converted to a 1D cluster index using a tiled layout:

```
clusterIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * 256 + gl_WorkGroupID.z * CLUSTER_TILE
```

This layout is defined in [src/shaders/clustersubmit.comp.glsl L30-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/clustersubmit.comp.glsl#L30-L38)

 and uses `CLUSTER_TILE=16` from [src/config.h L22](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L22-L22)

 to balance locality across different GPU architectures.

**Sources:** [src/shaders/meshlet.mesh.glsl L20-L63](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L20-L63)

 [src/shaders/meshlet.mesh.glsl L94](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L94-L94)

 [src/shaders/clustersubmit.comp.glsl L30-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/clustersubmit.comp.glsl#L30-L38)

 [src/config.h L22](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L22-L22)

---

## Vertex Processing Pipeline

### Vertex Data Unpacking

The mesh shader processes vertices in parallel, with each thread handling one or more vertices depending on the workgroup size configuration. Vertex data is unpacked from the compact `Vertex` structure:

```

```

**Diagram: Vertex Data Unpacking Process**

The vertex loop at [src/shaders/meshlet.mesh.glsl L125-L162](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L125-L162)

 retrieves vertex indices using either 16-bit or 32-bit references based on the `shortRefs` flag in the meshlet descriptor. The `unpackTBN()` function decodes octahedral-encoded normals and tangents (see [Shader Utilities and Math](/zeux/niagara/6.3-shader-utilities-and-math)).

### Transformation Pipeline

Each vertex undergoes the following transformations:

1. **Object Space to World Space**: Position, normal, and tangent are rotated by the instance's orientation quaternion and scaled
2. **World Space to Clip Space**: World position is transformed by the view-projection matrix

```
wpos = rotateQuat(position, orientation) * scale + position
clip = projection * view * vec4(wpos, 1)
```

The implementation at [src/shaders/meshlet.mesh.glsl L136-L140](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L136-L140)

 applies these transformations using quaternion rotation for normals and tangents to maintain orthogonality.

### Output Attributes

The mesh shader writes the following per-vertex attributes:

| Attribute | Type | Description |
| --- | --- | --- |
| `gl_Position` | `vec4` | Clip-space position for rasterization |
| `out_drawId` | `uint` | Draw command ID for material lookup |
| `out_uv` | `vec2` | Texture coordinates |
| `out_normal` | `vec3` | World-space normal |
| `out_tangent` | `vec4` | World-space tangent with handedness in .w |
| `out_wpos` | `vec3` | World-space position for lighting |

**Sources:** [src/shaders/meshlet.mesh.glsl L125-L162](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L125-L162)

 [src/shaders/meshlet.mesh.glsl L136-L140](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L136-L140)

---

## Triangle Processing and Culling

### Triangle Index Assembly

After vertices are processed, the mesh shader reads triangle indices from the meshlet data buffer. Triangle indices are stored as 8-bit unsigned integers in a packed format:

```
offset = indexOffset * 4 + triangleIndex * 3
a = meshletData8[offset]
b = meshletData8[offset + 1]
c = meshletData8[offset + 2]
```

These indices reference vertices within the meshlet's local vertex buffer (0 to `vertexCount-1`), not global vertex indices. The implementation is at [src/shaders/meshlet.mesh.glsl L168-L173](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L168-L173)

### Triangle Culling (Optional)

When `MESH_CULL=1` (controlled by [src/config.h L11](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L11-L11)

), the mesh shader performs per-triangle culling to eliminate triangles that would not contribute to the final image. This is disabled by default (`MESH_CULL=0`) because most culling is already performed in the task shader stage.

```

```

**Diagram: Triangle Culling Pipeline**

The culling implementation at [src/shaders/meshlet.mesh.glsl L175-L198](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L175-L198)

 performs:

1. **Backface Culling**: Eliminates triangles facing away from the camera based on winding order
2. **Zero-Area Culling**: Culls triangles with zero or negative area in screen space
3. **Sub-Pixel Culling**: Removes triangles smaller than one pixel using subpixel precision bounds
4. **Depth Safety**: Only applies culling when all vertices are in front of the perspective plane

The subpixel precision is set to 1/256 at [src/shaders/meshlet.mesh.glsl L189](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L189-L189)

 matching typical hardware subpixel precision bits.

**Sources:** [src/shaders/meshlet.mesh.glsl L168-L206](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L168-L206)

 [src/config.h L11](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L11-L11)

---

## Workgroup Organization

### Thread Distribution

The mesh shader is configured with the following workgroup parameters:

| Parameter | Configuration | Description |
| --- | --- | --- |
| `local_size_x` | `MESH_WGSIZE` (64) | Number of threads per workgroup |
| `local_size_y` | 1 | Single row of threads |
| `local_size_z` | 1 | Single layer of threads |
| `max_vertices` | `MESH_MAXVTX` (64) | Maximum vertices per meshlet |
| `max_primitives` | `MESH_MAXTRI` (96) | Maximum triangles per meshlet |

These constants are defined in [src/config.h L4-L15](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L4-L15)

### Parallel Loop Structure

The mesh shader uses conditional loop continuation to handle cases where meshlet size exceeds workgroup size:

```
for (uint i = ti; i < vertexCount; )
{
    // Process vertex i
    
    #if MESH_MAXVTX <= MESH_WGSIZE
        break;  // Single iteration per thread
    #else
        i += MESH_WGSIZE;  // Multiple iterations per thread
    #endif
}
```

With the default configuration (`MESH_MAXVTX=64`, `MESH_WGSIZE=64`), each thread processes exactly one vertex and one or more triangles. If `MESH_MAXTRI > MESH_WGSIZE`, threads loop to process multiple triangles.

### Shared Memory Usage

When triangle culling is enabled (`MESH_CULL=1`), the mesh shader uses shared memory to store vertex clip-space data for culling decisions:

```

```

Each vertex stores `(screenX, screenY, clipW)` in shared memory at [src/shaders/meshlet.mesh.glsl L150](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L150-L150)

 A barrier synchronization at [src/shaders/meshlet.mesh.glsl L165](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L165-L165)

 ensures all vertex data is written before triangle culling begins.

**Sources:** [src/shaders/meshlet.mesh.glsl L17-L18](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L17-L18)

 [src/shaders/meshlet.mesh.glsl L86-L87](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L86-L87)

 [src/shaders/meshlet.mesh.glsl L125-L162](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L125-L162)

 [src/shaders/meshlet.mesh.glsl L168-L206](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L168-L206)

 [src/config.h L4-L15](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L4-L15)

---

## Output Generation

### Mesh Output Declaration

The mesh shader calls `SetMeshOutputsEXT()` at [src/shaders/meshlet.mesh.glsl L110](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L110-L110)

 to declare the number of vertices and primitives it will emit. This must be called before writing any output data and can be zero if the meshlet should be culled entirely (e.g., when `ci == ~0` at [src/shaders/meshlet.mesh.glsl L96-L100](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L96-L100)

).

### Vertex Outputs

Vertex data is written to built-in and custom output arrays:

```

```

These arrays are indexed by the local vertex index (0 to `vertexCount-1`) and are declared at [src/shaders/meshlet.mesh.glsl L65-L69](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L65-L69)

### Primitive Outputs

Triangle topology is written using:

```

```

When culling is enabled, the cull flag can be set:

```

```

These outputs at [src/shaders/meshlet.mesh.glsl L173](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L173-L173)

 and [src/shaders/meshlet.mesh.glsl L197](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L197-L197)

 define the triangle topology that will be rasterized.

### Debug Visualization

When `DEBUG=1` is defined at [src/shaders/meshlet.mesh.glsl L14](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L14-L14)

 the shader replaces normals with per-meshlet color values computed from a hash function at [src/shaders/meshlet.mesh.glsl L119-L121](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L119-L121)

 allowing visualization of meshlet boundaries.

**Sources:** [src/shaders/meshlet.mesh.glsl L96-L110](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L96-L110)

 [src/shaders/meshlet.mesh.glsl L142-L147](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L142-L147)

 [src/shaders/meshlet.mesh.glsl L173](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L173-L173)

 [src/shaders/meshlet.mesh.glsl L197](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L197-L197)

---

## Integration with Pipeline Stages

### Task Shader Payload

When `TASK=true`, the mesh shader receives cluster indices through a task payload structure:

```

```

The task shader (see [Task Shader Stage](/zeux/niagara/7.2-task-shader-stage)) populates this payload with up to 32 cluster indices, and the mesh shader workgroup retrieves its assigned index at [src/shaders/meshlet.mesh.glsl L94](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L94-L94)

### Dispatch Organization

The compute shaders `tasksubmit.comp.glsl` and `clustersubmit.comp.glsl` convert 1D cluster counts into 2D/3D dispatch dimensions for the mesh shader pipeline:

```

```

**Diagram: Dispatch Configuration Paths**

The task submit shader at [src/shaders/tasksubmit.comp.glsl L32-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/tasksubmit.comp.glsl#L32-L38)

 generates dispatch dimensions `X×64×1` where X is limited to 65535. The cluster submit shader at [src/shaders/clustersubmit.comp.glsl L30-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/clustersubmit.comp.glsl#L30-L38)

 uses a `16×Y×16` layout to balance performance across different GPU architectures.

**Sources:** [src/shaders/meshlet.mesh.glsl L72](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L72-L72)

 [src/shaders/tasksubmit.comp.glsl L32-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/tasksubmit.comp.glsl#L32-L38)

 [src/shaders/clustersubmit.comp.glsl L30-L38](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/clustersubmit.comp.glsl#L30-L38)

---

## Configuration Reference

### Compile-Time Constants

The mesh shader behavior is controlled by several configuration constants:

| Constant | Default | Purpose |
| --- | --- | --- |
| `TASK` | `false` | Specialization constant: use task payload vs. direct buffer read |
| `MESH_WGSIZE` | 64 | Workgroup size (threads per meshlet) |
| `MESH_MAXVTX` | 64 | Maximum vertices per meshlet |
| `MESH_MAXTRI` | 96 | Maximum triangles per meshlet |
| `MESH_CULL` | 0 | Enable per-triangle culling |
| `DEBUG` | 0 | Enable meshlet color visualization |
| `CLUSTER_TILE` | 16 | X-dimension for cluster dispatch layout |
| `CLUSTER_LIMIT` | 16777216 | Maximum visible clusters (2^24) |

The `TASK` constant at [src/shaders/meshlet.mesh.glsl L12](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L12-L12)

 is set via pipeline specialization constant, allowing the same shader to be compiled for both execution modes.

### Performance Tuning

* **MESH_CULL**: Typically disabled because task shader culling is more efficient for meshlet-granularity culling
* **MESH_WGSIZE**: Should match MESH_MAXVTX for optimal single-pass vertex processing
* **MESH_MAXTRI**: Set to 96 to balance triangle throughput with register pressure
* **CLUSTER_TILE**: Set to 16 for balanced performance on discrete and integrated GPUs

**Sources:** [src/shaders/meshlet.mesh.glsl L12-L15](https://github.com/zeux/niagara/blob/6f3fb529/src/shaders/meshlet.mesh.glsl#L12-L15)

 [src/config.h L4-L15](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L4-L15)

 [src/config.h L22](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L22-L22)

 [src/config.h L28](https://github.com/zeux/niagara/blob/6f3fb529/src/config.h#L28-L28)