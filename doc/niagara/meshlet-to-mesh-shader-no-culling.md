# Niagara Meshlet → Mesh Shader 相关代码（无剔除）

本文档从 `Source/Niagara/niagara/` 提取 meshlet 到 mesh shader 为止的相关代码，**忽略所有 culling 逻辑**（TASK_CULL=0, MESH_CULL=0）。

---

## 1. 配置常量 (config.h)

```c
// Task shader 工作组大小；每个 task 线程最多产生一个 meshlet
#define TASK_WGSIZE 64

// Mesh shader 工作组大小；一个 mesh shader 工作组并行处理整个 meshlet
#define MESH_WGSIZE 64

// Meshlet 内最大顶点数和三角形数
#define MESH_MAXVTX 64
#define MESH_MAXTRI 96

// 3D 分块 dispatch 时 X 维的 cluster 数量（须为 256 的因子）
#define CLUSTER_TILE 16
```

---

## 2. 数据结构 (mesh.h 精简)

```glsl
// 顶点：位置、法线/切向打包、UV
struct Vertex
{
    float16_t vx, vy, vz;
    uint16_t tp;   // packed tangent: 8-8 octahedral
    uint np;       // packed normal: 10-10-10-2 + bitangent sign
    float16_t tu, tv;
};

// Meshlet：包围球、cone、顶点/三角形数据偏移
struct Meshlet
{
    float16_t center[3];
    float16_t radius;
    int8_t cone_axis[3];
    int8_t cone_cutoff;

    uint dataOffset;    // meshletdata 中的起始偏移（uint32 单位）
    uint baseVertex;    // 顶点缓冲区中的基址
    uint8_t vertexCount;
    uint8_t triangleCount;
    uint8_t shortRefs;  // 1=顶点引用用 uint16 打包，0=uint32
    uint8_t padding;
};

// 每个 draw 的变换与材质
struct MeshDraw
{
    vec3 position;
    float scale;
    vec4 orientation;

    uint meshIndex;
    uint meshletVisibilityOffset;  // 无剔除时未使用
    uint postPass;
    uint materialIndex;
};

// 每个 draw 对应的 meshlet 任务命令（无剔除时每 draw 一条）
struct MeshTaskCommand
{
    uint drawId;
    uint taskOffset;    // 该 draw 在 meshlets 中的起始索引
    uint taskCount;     // 该 draw 的 meshlet 数量
    uint lateDrawVisibility;
    uint meshletVisibilityOffset;
};

// 全局：投影、视口、剔除参数（无剔除时仅用 view/projection）
struct CullData
{
    mat4 view;
    float P00, P11, znear, zfar;
    float frustum[4];
    float lodTarget;
    float pyramidWidth, pyramidHeight;
    uint drawCount;
    int cullingEnabled;
    int lodEnabled;
    int occlusionEnabled;
    int clusterOcclusionEnabled;
    int clusterBackfaceEnabled;
    uint postPass;
};

struct Globals
{
    mat4 projection;
    CullData cullData;
    float screenWidth, screenHeight;
};
```

---

## 3. clusterIndices 格式

**格式**：`commandId | (mgi << 24)`

- `commandId`：`ci & 0xFFFFFF`，对应 `taskCommands[commandId]`
- `mgi`：`ci >> 24`，该 command 内的 meshlet 局部索引
- `mi = command.taskOffset + mgi`：meshlet 全局索引

**无剔除时**：每个可见 meshlet 写入一条 `clusterIndices[index] = commandId | (mgi << 24)`。可由 CPU 预计算或由 clustercull（CULL=0）在 GPU 上填充。

---

## 4. Mesh Shader (meshlet.mesh.glsl, TASK=false, MESH_CULL=0)

```glsl
// TASK=false：直接使用 clusterIndices，不经过 task shader
layout (constant_id = 1) const bool TASK = false;

#define MESH_CULL 0  // 无 mesh 内三角形剔除

layout(local_size_x = MESH_WGSIZE, local_size_y = 1, local_size_z = 1) in;
layout(triangles, max_vertices = MESH_MAXVTX, max_primitives = MESH_MAXTRI) out;

layout(push_constant) uniform block { Globals globals; };

layout(binding = 0) readonly buffer TaskCommands { MeshTaskCommand taskCommands[]; };
layout(binding = 1) readonly buffer Draws { MeshDraw draws[]; };
layout(binding = 2) readonly buffer Meshlets { Meshlet meshlets[]; };
layout(binding = 3) readonly buffer MeshletData { uint meshletData[]; };
layout(binding = 3) readonly buffer MeshletData16 { uint16_t meshletData16[]; };
layout(binding = 3) readonly buffer MeshletData8 { uint8_t meshletData8[]; };
layout(binding = 4) readonly buffer Vertices { Vertex vertices[]; };
layout(binding = 5) readonly buffer ClusterIndices { uint clusterIndices[]; };

// 输出到 fragment shader
layout(location = 0) out flat uint out_drawId[];
layout(location = 1) out vec2 out_uv[];
layout(location = 2) out vec3 out_normal[];
layout(location = 3) out vec4 out_tangent[];
layout(location = 4) out vec3 out_wpos[];

void main()
{
    uint ti = gl_LocalInvocationIndex;

    // 1D/3D 索引：无 task 时用 groupID 直接索引 clusterIndices
    // 3D: groupID.x + groupID.y*256 + groupID.z*CLUSTER_TILE
    uint ci = TASK ? payload.clusterIndices[gl_WorkGroupID.x]
                   : clusterIndices[gl_WorkGroupID.x + gl_WorkGroupID.y * 256 + gl_WorkGroupID.z * CLUSTER_TILE];

    // 无效 cluster（已剔除占位）
    if (ci == ~0)
    {
        SetMeshOutputsEXT(0, 0);
        return;
    }

    // 解析 cluster 索引
    MeshTaskCommand command = taskCommands[ci & 0xffffff];
    uint mi = command.taskOffset + (ci >> 24);

    MeshDraw meshDraw = draws[command.drawId];

    uint vertexCount = uint(meshlets[mi].vertexCount);
    uint triangleCount = uint(meshlets[mi].triangleCount);

    SetMeshOutputsEXT(vertexCount, triangleCount);

    uint dataOffset = meshlets[mi].dataOffset;
    uint baseVertex = meshlets[mi].baseVertex;
    bool shortRefs = uint(meshlets[mi].shortRefs) == 1;
    uint vertexOffset = dataOffset;
    uint indexOffset = dataOffset + (shortRefs ? (vertexCount + 1) / 2 : vertexCount);

    // 顶点循环：每个线程处理 i, i+MESH_WGSIZE, ...
    for (uint i = ti; i < vertexCount; )
    {
        uint vi = shortRefs
            ? uint(meshletData16[vertexOffset * 2 + i]) + baseVertex
            : meshletData[vertexOffset + i] + baseVertex;

        vec3 position = vec3(vertices[vi].vx, vertices[vi].vy, vertices[vi].vz);
        vec2 texcoord = vec2(vertices[vi].tu, vertices[vi].tv);

        vec3 normal;
        vec4 tangent;
        unpackTBN(vertices[vi].np, uint(vertices[vi].tp), normal, tangent);

        normal = rotateQuat(normal, meshDraw.orientation);
        tangent.xyz = rotateQuat(tangent.xyz, meshDraw.orientation);

        vec3 wpos = rotateQuat(position, meshDraw.orientation) * meshDraw.scale + meshDraw.position;
        vec4 clip = globals.projection * (globals.cullData.view * vec4(wpos, 1));

        gl_MeshVerticesEXT[i].gl_Position = clip;
        out_drawId[i] = command.drawId;
        out_uv[i] = texcoord;
        out_normal[i] = normal;
        out_tangent[i] = tangent;
        out_wpos[i] = wpos;

        i += MESH_WGSIZE;
    }

    // 三角形索引循环
    for (uint i = ti; i < triangleCount; )
    {
        uint offset = indexOffset * 4 + i * 3;
        uint a = uint(meshletData8[offset]), b = uint(meshletData8[offset + 1]), c = uint(meshletData8[offset + 2]);

        gl_PrimitiveTriangleIndicesEXT[i] = uvec3(a, b, c);

        i += MESH_WGSIZE;
    }
}
```

---

## 5. Task Shader 无剔除路径 (meshlet.task.glsl, TASK_CULL=0)

当使用 task + mesh 管线且无剔除时，task shader 仅负责把 `commandId | mgi` 写入 payload，并 `EmitMeshTasksEXT`：

```glsl
// TASK_CULL=0 时的 else 分支
#else
    payload.clusterIndices[gl_LocalInvocationID.x] = commandId | (mgi << 24);
    EmitMeshTasksEXT(taskCount, 1, 1);
#endif
```

- `commandId = gl_WorkGroupID.x * 64 + gl_WorkGroupID.y`（2D dispatch）
- `mgi = gl_LocalInvocationID.x`
- 每个 task 工作组对应一个 MeshTaskCommand，输出该 command 下所有 meshlet 的 cluster 索引。

---

## 6. Clustercull 无剔除路径 (clustercull.comp.glsl, CULL=0)

当使用 cluster 管线（无 task，直接 mesh）且无剔除时，clustercull 仅做“透传”：

```glsl
#else
    if (mgi < taskCount)
    {
        uint index = atomicAdd(clusterCount, 1);
        if (index < CLUSTER_LIMIT)
            clusterIndices[index] = commandId | (mgi << 24);
    }
#endif
```

- `commandId = gl_WorkGroupID.x * 64 + gl_WorkGroupID.y`
- 每个 workgroup 对应一个 MeshTaskCommand，将该 command 下所有 meshlet 写入 `clusterIndices`。

---

## 7. Clustersubmit (clustersubmit.comp.glsl)

根据 `clusterCount` 设置 mesh shader 的 dispatch 维度：

```glsl
layout(binding = 0) buffer ClusterCount
{
    uint clusterCount;
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
};

layout(binding = 1) writeonly buffer ClusterIndices { uint clusterIndices[]; };

void main()
{
    uint tid = gl_LocalInvocationID.x;
    uint count = min(clusterCount, CLUSTER_LIMIT);

    if (tid == 0)
    {
        // 3D dispatch: 16*Y*16，满足 mesh shader 的索引公式
        groupCountX = CLUSTER_TILE;           // 16
        groupCountY = min((count + 255) / 256, 65535);
        groupCountZ = 256 / CLUSTER_TILE;    // 16
    }

    uint boundary = (count + 255) & ~255;
    if (count + tid < boundary)
        clusterIndices[count + tid] = ~0;  // 填充无效占位
}
```

---

## 8. C++ 侧：Cluster 管线绑定 (niagara.cpp 精简)

```cpp
// 管线：meshlet.mesh + mesh.frag，无 task
clusterProgram = createProgram(device, VK_PIPELINE_BIND_POINT_GRAPHICS,
    { &shaders["meshlet.mesh"], &shaders["mesh.frag"] }, sizeof(Globals), textureSetLayout);

// 绑定
DescriptorInfo descriptors[] = {
    dcb.buffer,   // TaskCommands (MeshTaskCommand[])
    db.buffer,    // Draws (MeshDraw[])
    mlb.buffer,   // Meshlets (Meshlet[])
    mdb.buffer,   // MeshletData (uint[])
    vb.buffer,    // Vertices (Vertex[])
    cib.buffer,   // ClusterIndices (uint[])
    /* ... textureSampler, mtb ... */
};
vkCmdPushDescriptorSetWithTemplate(..., descriptors);
vkCmdPushConstants(..., &passGlobals);
vkCmdDrawMeshTasksIndirectEXT(commandBuffer, ccb.buffer, 4, 1, 0);  // ccb 含 groupCountX/Y/Z
```

---

## 9. meshletdata 布局

- **顶点引用**（从 `dataOffset` 起）：
  - `shortRefs=0`：每个顶点 1 个 `uint`，值为 `(顶点索引 - minVertex)`
  - `shortRefs=1`：每 2 个顶点 1 个 `uint`，低 16 位和高 16 位各一个引用
- **三角形索引**（紧跟顶点引用）：
  - 每三角形 3 个 `uint8`，为 meshlet 内顶点索引 (0..vertexCount-1)
  - 存储为 `(triangleCount*3+3)/4` 个 `uint32`

---

## 10. 数据流小结（无剔除）

```
CPU/GPU 预计算 clusterIndices
    ↓
clustersubmit：写 groupCountX/Y/Z 到 ccb
    ↓
vkCmdDrawMeshTasksIndirectEXT(ccb)
    ↓
Mesh Shader：groupID → clusterIndices[groupID] → ci → command, mi
    ↓
读取 meshlets[mi], meshletData, vertices
    ↓
输出顶点 + 三角形索引 → Pixel Shader
```
