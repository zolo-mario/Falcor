/**
 * D3D12 Meshlet Cull - Amplification + Mesh + Pixel shader pipeline.
 * AS: Per-meshlet frustum and normal-cone culling.
 * MS: Output meshlet vertices/primitives.
 * PS: Blinn-Phong shading, meshlet visualization.
 * Migrated from DirectX-Graphics-Samples D3D12MeshShaders/MeshletCull.
 */

#define AS_GROUP_SIZE 32
#define CULL_FLAG 0x1
#define MESHLET_FLAG 0x2

// Shared structures (match D3D12 Shared.h, MeshletUtils.hlsli)
struct Instance
{
    float4x4 World;
    float4x4 WorldInvTrans;
    float Scale;
    uint Flags;
};

struct Constants
{
    float4x4 View;
    float4x4 ViewProj;
    float4 Planes[6];
    float3 ViewPosition;
    uint HighlightedIndex;
    float3 CullViewPosition;
    uint SelectedIndex;
    uint DrawMeshlets;
};

struct MeshInfo
{
    uint IndexSize;
    uint MeshletCount;
    uint LastMeshletVertCount;
    uint LastMeshletPrimCount;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

struct CullData
{
    float4 BoundingSphere;
    uint NormalCone;
    float ApexOffset;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
};

struct VertexOut
{
    float4 PositionHS : SV_Position;
    float3 PositionVS : POSITION0;
    float3 Normal : NORMAL0;
    uint MeshletIndex : COLOR0;
};

struct Payload
{
    uint MeshletIndices[AS_GROUP_SIZE];
};

// MeshletUtils
bool isConeDegenerate(CullData c)
{
    return (c.NormalCone >> 24) == 0xff;
}

float4 unpackCone(uint packed)
{
    float4 v;
    v.x = float((packed >> 0) & 0xFF) / 255.0;
    v.y = float((packed >> 8) & 0xFF) / 255.0;
    v.z = float((packed >> 16) & 0xFF) / 255.0;
    v.w = float((packed >> 24) & 0xFF) / 255.0;
    v.xyz = v.xyz * 2.0 - 1.0;
    return v;
}

float3 rotateVector(float3 v0, float3 axis, float angle)
{
    float cs = cos(angle);
    return cs * v0 + sin(angle) * cross(axis, v0) + (1 - cs) * dot(axis, v0) * axis;
}

// Resource bindings
ConstantBuffer<Constants> gConstants : register(b0);
ConstantBuffer<MeshInfo> gMeshInfo : register(b1);
ConstantBuffer<Instance> gInstance : register(b2);
StructuredBuffer<Vertex> gVertices : register(t0);
StructuredBuffer<Meshlet> gMeshlets : register(t1);
ByteAddressBuffer gUniqueVertexIndices : register(t2);
StructuredBuffer<uint> gPrimitiveIndices : register(t3);
StructuredBuffer<CullData> gMeshletCullData : register(t4);

// Amplification Shader
bool isVisible(CullData c, float4x4 world, float scale, float3 viewPos)
{
    if ((gInstance.Flags & CULL_FLAG) == 0)
        return true;

    float4 center = mul(float4(c.BoundingSphere.xyz, 1), world);
    float radius = c.BoundingSphere.w * scale;

    for (int i = 0; i < 6; ++i)
    {
        if (dot(center, gConstants.Planes[i]) < -radius)
            return false;
    }

    if (isConeDegenerate(c))
        return true;

    float4 normalCone = unpackCone(c.NormalCone);
    float3 axis = normalize(mul(float4(normalCone.xyz, 0), world)).xyz;
    float3 apex = center.xyz - axis * c.ApexOffset * scale;
    float3 view = normalize(viewPos - apex);

    if (dot(view, -axis) > normalCone.w)
        return false;

    return true;
}

groupshared Payload s_Payload;

[numthreads(AS_GROUP_SIZE, 1, 1)]
void ampMain(
    in uint gtid : SV_GroupThreadID,
    in uint dtid : SV_DispatchThreadID,
    in uint gid : SV_GroupID)
{
    bool visible = false;
    if (dtid < gMeshInfo.MeshletCount)
    {
        visible = isVisible(
            gMeshletCullData[dtid],
            gInstance.World,
            gInstance.Scale,
            gConstants.CullViewPosition);
    }

    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.MeshletIndices[index] = dtid;
    }

    GroupMemoryBarrierWithGroupSync();
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
}

// Mesh Shader helpers
uint3 unpackPrimitive(uint primitive)
{
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint getVertexIndex(Meshlet m, uint localIndex)
{
    uint idx = m.VertOffset + localIndex;
    if (gMeshInfo.IndexSize == 4)
        return gUniqueVertexIndices.Load(idx * 4);
    uint wordOffset = idx & 0x1;
    uint byteOffset = (idx / 2) * 4;
    uint indexPair = gUniqueVertexIndices.Load(byteOffset);
    return (indexPair >> (wordOffset * 16)) & 0xffff;
}

uint3 getPrimitive(Meshlet m, uint index)
{
    return unpackPrimitive(gPrimitiveIndices[m.PrimOffset + index]);
}

VertexOut getVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    Vertex v = gVertices[vertexIndex];
    float4 positionWS = mul(float4(v.Position, 1), gInstance.World);
    VertexOut vout;
    vout.PositionVS = mul(positionWS, gConstants.View).xyz;
    vout.PositionHS = mul(positionWS, gConstants.ViewProj);
    vout.Normal = mul(float4(v.Normal, 0), gInstance.WorldInvTrans).xyz;
    vout.MeshletIndex = meshletIndex;
    return vout;
}

// Mesh Shader
static const uint MAX_VERTS = 64;
static const uint MAX_PRIMS = 126;

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void meshMain(
    in uint dtid : SV_DispatchThreadID,
    in uint gtid : SV_GroupThreadID,
    in uint gid : SV_GroupID,
    in payload Payload payload,
    out vertices VertexOut verts[MAX_VERTS],
    out indices uint3 tris[MAX_PRIMS])
{
    uint meshletIndex = payload.MeshletIndices[gid];
    if (meshletIndex >= gMeshInfo.MeshletCount)
        return;

    Meshlet m = gMeshlets[meshletIndex];
    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.VertCount)
    {
        uint vertexIndex = getVertexIndex(m, gtid);
        verts[gtid] = getVertexAttributes(meshletIndex, vertexIndex);
    }

    if (gtid < m.PrimCount)
    {
        tris[gtid] = getPrimitive(m, gtid);
    }
}

// Pixel Shader
float3 luminosity(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)).xxx;
}

float4 psMain(VertexOut pin) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 color = 0.8;
    float shininess = 16;

    if (gConstants.DrawMeshlets && (gInstance.Flags & MESHLET_FLAG) != 0)
    {
        color = float3(
            float(pin.MeshletIndex & 1),
            float(pin.MeshletIndex & 3) / 4,
            float(pin.MeshletIndex & 7) / 8);

        if (gConstants.HighlightedIndex != 0xFFFFFFFF ||
            gConstants.SelectedIndex != 0xFFFFFFFF)
        {
            if (pin.MeshletIndex != gConstants.SelectedIndex &&
                pin.MeshletIndex != gConstants.HighlightedIndex)
            {
                color = lerp(color, luminosity(color) + 0.2, 0.8);
            }
        }
    }

    float3 normal = normalize(pin.Normal);
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(pin.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);
    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * color;
    return float4(finalColor, 1);
}
