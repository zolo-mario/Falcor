/**
 * Niagara mesh shader - AS frustum + cone culling.
 * AS: Per-meshlet frustum and backface cone culling, payload only visible meshlets.
 * MS: Read meshlet from NiagaraFormat, output vertices.
 * PS: Output vertex world position (wpos).
 */

#define AS_GROUP_SIZE 32
#define MESH_WGSIZE 64
#ifndef LATE
#define LATE 0
#endif
#define MESH_MAXVTX 64
#define MESH_MAXTRI 96
#define TASK_STRIDE 64

struct NiagaraVertex
{
    uint16_t vx, vy, vz;
    uint16_t tp;
    uint np;
    uint16_t tu, tv;
};

struct NiagaraMeshlet
{
    uint16_t center[3];
    uint16_t radius;
    int16_t cone_axis[3];
    int16_t cone_cutoff;
    uint dataOffset;
    uint baseVertex;
    uint16_t vertexCount;
    uint16_t triangleCount;
    uint16_t shortRefs;
    uint16_t padding;
};

struct NiagaraMeshDraw
{
    float3 position;
    float scale;
    float4 orientation;
    uint meshIndex;
    uint meshletVisibilityOffset;
    uint postPass;
    uint materialIndex;
};

struct NiagaraMeshTaskCommand
{
    uint drawId;
    uint taskOffset;
    uint taskCount;
    uint meshletVisibilityOffset;
};

struct NiagaraGlobals
{
    float4x4 projection;
    float4x4 view;
    uint meshletCount;
    float znear;
    float zfar;
    float frustum[4];
    uint clusterBackfaceEnabled;
    float pyramidWidth;
    float pyramidHeight;
    float P00;
    float P11;
    uint occlusionEnabled;
    uint clusterOcclusionEnabled;
};

// projectSphere: 2D polyhedral bounds of clipped perspective-projected 3D sphere (Mara, McGuire 2013)
// Returns false if sphere is between camera and near plane. AABB in uv [0,1] on success.
// Falcor view space: z negative for visible, near plane at z=-znear.
bool projectSphere(float3 c, float r, float znear, float P00, float P11, out float4 aabb)
{
    if (c.z > r - znear)
    {
        aabb = float4(0, 0, 0, 0);
        return false;
    }

    float3 cr = c * r;
    float czr2 = c.z * c.z - r * r;

    float vx = sqrt(c.x * c.x + czr2);
    float minx = (vx * c.x - cr.z) / (vx * c.z + cr.x);
    float maxx = (vx * c.x + cr.z) / (vx * c.z - cr.x);

    float vy = sqrt(c.y * c.y + czr2);
    float miny = (vy * c.y - cr.z) / (vy * c.z + cr.y);
    float maxy = (vy * c.y + cr.z) / (vy * c.z - cr.y);

    aabb = float4(minx * P00, miny * P11, maxx * P00, maxy * P11);
    aabb = aabb.xwzy * float4(0.5, -0.5, 0.5, -0.5) + float4(0.5, 0.5, 0.5, 0.5);
    return true;
}

cbuffer CB : register(b0)
{
    NiagaraGlobals gGlobals;
};

StructuredBuffer<NiagaraMeshTaskCommand> gTaskCommands : register(t0);
StructuredBuffer<NiagaraMeshDraw> gDraws : register(t1);
StructuredBuffer<NiagaraMeshlet> gMeshlets : register(t2);
ByteAddressBuffer gMeshletData : register(t3);
StructuredBuffer<NiagaraVertex> gVertices : register(t4);
StructuredBuffer<uint> gClusterIndices : register(t5);
StructuredBuffer<uint> gDrawVisibility : register(t6);
RWStructuredBuffer<uint> gMeshletVisibility : register(u0);
Texture2D<float> gDepthPyramid : register(t8);
SamplerState gDepthPyramid_sampler : register(s0);

struct Payload
{
    uint meshletIndices[AS_GROUP_SIZE];
};

groupshared Payload s_Payload;
groupshared uint s_VisibleCount;

float3 rotateQuat(float3 v, float4 q)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

bool coneCull(float3 center, float radius, float3 cone_axis, float cone_cutoff, float3 camera_pos)
{
    return dot(center - camera_pos, cone_axis) >= cone_cutoff * length(center - camera_pos) + radius;
}

float3 decodeHalf3(uint16_t vx, uint16_t vy, uint16_t vz)
{
    return float3(f16tof32((uint)vx), f16tof32((uint)vy), f16tof32((uint)vz));
}

[numthreads(AS_GROUP_SIZE, 1, 1)]
void ampMain(
    in uint gtid : SV_GroupThreadID,
    in uint dtid : SV_DispatchThreadID,
    in uint gid : SV_GroupID)
{
    uint base = gid * AS_GROUP_SIZE;
    uint count = min(AS_GROUP_SIZE, gGlobals.meshletCount - base);
    uint linearIdx = base + gtid;

    bool visible = false;
    bool skip = false;
    if (gtid < count)
    {
        uint ci = gClusterIndices[linearIdx];
        uint commandId = ci & 0xFFFFFF;
        uint drawId = commandId / TASK_STRIDE;
        uint mgi = ci >> 24;

        NiagaraMeshTaskCommand command = gTaskCommands[drawId];
        NiagaraMeshDraw meshDraw = gDraws[command.drawId];
        NiagaraMeshlet meshlet = gMeshlets[command.taskOffset + mgi];

        uint mvi = command.meshletVisibilityOffset + mgi;
        uint meshletVisibilityBit = (gMeshletVisibility[mvi >> 5] >> (mvi & 31)) & 1u;
        uint lateDrawVisibility = gDrawVisibility[drawId] & 1u;

        if (gGlobals.clusterOcclusionEnabled != 0)
        {
            // Early: only render meshlets that were visible last frame
            if (LATE == 0 && meshletVisibilityBit == 0)
                visible = false;
            else
                visible = true;

            // Late: skip meshlets we already rendered in Early
            if (LATE != 0 && lateDrawVisibility == 1 && meshletVisibilityBit != 0)
                skip = true;
        }
        else
        {
            visible = true;
        }

        float3 center = decodeHalf3(meshlet.center[0], meshlet.center[1], meshlet.center[2]);
        center = rotateQuat(center, meshDraw.orientation) * meshDraw.scale + meshDraw.position;
        center = mul(gGlobals.view, float4(center, 1)).xyz;

        float radius = f16tof32((uint)meshlet.radius) * meshDraw.scale;

        float3 cone_axis = float3((float)meshlet.cone_axis[0], (float)meshlet.cone_axis[1], (float)meshlet.cone_axis[2]) / 127.0;
        cone_axis = mul((float3x3)gGlobals.view, rotateQuat(cone_axis, meshDraw.orientation));
        float cone_cutoff = (float)meshlet.cone_cutoff / 127.0;

        visible = visible && (center.z * gGlobals.frustum[1] - abs(center.x) * gGlobals.frustum[0] > -radius);
        visible = visible && (center.z * gGlobals.frustum[3] - abs(center.y) * gGlobals.frustum[2] > -radius);
        visible = visible && (center.z + radius < -gGlobals.znear);
        visible = visible && (center.z - radius > -gGlobals.zfar);
        visible = visible && (gGlobals.clusterBackfaceEnabled == 0 || !coneCull(center, radius, cone_axis, cone_cutoff, float3(0, 0, 0)));

        // Late pass: occlusion test against depth pyramid
        if (LATE != 0 && gGlobals.clusterOcclusionEnabled != 0 && gGlobals.occlusionEnabled != 0 && visible)
        {
            float4 aabb;
            if (projectSphere(center, radius, gGlobals.znear, gGlobals.P00, gGlobals.P11, aabb))
            {
                float w = (aabb.z - aabb.x) * gGlobals.pyramidWidth;
                float h = (aabb.w - aabb.y) * gGlobals.pyramidHeight;
                float level = floor(log2(max(w, h)));
                    float2 uv = (aabb.xy + aabb.zw) * 0.5;
                    float depth = gDepthPyramid.SampleLevel(gDepthPyramid_sampler, uv, level);
                    float depthSphere = gGlobals.znear / (-center.z - radius);
                    // depth >= 0.999: empty/cleared, don't cull
                    visible = visible && (depth >= 0.999 || depthSphere > depth);
            }
        }

        // Late pass: update meshlet visibility buffer
        if (LATE != 0 && gGlobals.clusterOcclusionEnabled != 0)
        {
            if (visible)
                InterlockedOr(gMeshletVisibility[mvi >> 5], 1u << (mvi & 31));
            else
                InterlockedAnd(gMeshletVisibility[mvi >> 5], ~(1u << (mvi & 31)));
        }
    }

    if (gtid == 0)
        s_VisibleCount = 0;
    GroupMemoryBarrierWithGroupSync();

    if (visible && !skip)
    {
        uint idx = 0;
        InterlockedAdd(s_VisibleCount, 1, idx);
        s_Payload.meshletIndices[idx] = linearIdx;
    }

    GroupMemoryBarrierWithGroupSync();
    uint visibleCount = s_VisibleCount;
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}

struct MeshVertexOut
{
    float4 posH : SV_Position;
    float3 wpos : TEXCOORD0;
};

[outputtopology("triangle")]
[numthreads(MESH_WGSIZE, 1, 1)]
void meshMain(
    in uint ti : SV_GroupIndex,
    in uint3 groupID : SV_GroupID,
    in payload Payload payload,
    OutputVertices<MeshVertexOut, MESH_MAXVTX> verts,
    OutputIndices<uint3, MESH_MAXTRI> triangles)
{
    uint linearIdx = payload.meshletIndices[groupID.x];
    uint ci = gClusterIndices[linearIdx];

    uint vertexCount = 0;
    uint triangleCount = 0;
    uint dataOffset = 0;
    uint baseVertex = 0;
    bool shortRefs = false;
    uint mi = 0;
    NiagaraMeshTaskCommand command;

    if (ci != 0xFFFFFFFF)
    {
        uint commandId = ci & 0xFFFFFF;
        uint drawId = commandId / TASK_STRIDE;
        uint mgi = ci >> 24;

        command = gTaskCommands[drawId];
        mi = command.taskOffset + mgi;
        vertexCount = gMeshlets[mi].vertexCount;
        triangleCount = gMeshlets[mi].triangleCount;
        dataOffset = gMeshlets[mi].dataOffset;
        baseVertex = gMeshlets[mi].baseVertex;
        shortRefs = gMeshlets[mi].shortRefs == 1;
    }

    SetMeshOutputCounts(vertexCount, triangleCount);

    if (ci == 0xFFFFFFFF)
        return;

    NiagaraMeshDraw meshDraw = gDraws[command.drawId];
    uint vertexOffset = dataOffset;
    uint indexOffset = dataOffset + (shortRefs ? (vertexCount + 1) / 2 : vertexCount);

    for (uint i = ti; i < vertexCount; i += MESH_WGSIZE)
    {
        uint vi;
        if (shortRefs)
        {
            uint packed = gMeshletData.Load((vertexOffset + i / 2) * 4);
            vi = (i % 2 == 0) ? (packed & 0xFFFF) : (packed >> 16);
            vi += baseVertex;
        }
        else
        {
            vi = gMeshletData.Load((vertexOffset + i) * 4) + baseVertex;
        }

        NiagaraVertex v = gVertices[vi];
        float3 position = decodeHalf3(v.vx, v.vy, v.vz);
        float3 wpos = rotateQuat(position, meshDraw.orientation) * meshDraw.scale + meshDraw.position;
        float4 clip = mul(gGlobals.projection, mul(gGlobals.view, float4(wpos, 1)));

        MeshVertexOut outVtx;
        outVtx.posH = clip;
        outVtx.wpos = wpos;
        verts[i] = outVtx;
    }

    for (uint i = ti; i < triangleCount; i += MESH_WGSIZE)
    {
        uint offset = indexOffset * 4 + i * 3;
        uint packed = gMeshletData.Load(offset);
        uint a = packed & 0xFF;
        uint b = (packed >> 8) & 0xFF;
        uint c = (packed >> 16) & 0xFF;
        triangles[i] = uint3(a, b, c);
    }
}

struct PSOut
{
    float4 wpos : SV_TARGET0;
};

PSOut psMain(MeshVertexOut inData)
{
    PSOut outData;
    outData.wpos = float4(inData.wpos, 1.0);
    return outData;
}
