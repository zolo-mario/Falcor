/**
 * D3D12 Meshlet Instancing - Mesh Shader + Pixel Shader
 * Ported from DirectX-Graphics-Samples D3D12MeshShaders/MeshletInstancing
 * Requires Shader Model 6.5, mesh shader support.
 */

#define MAX_VERTS 64
#define MAX_PRIMS 126

struct Constants
{
    float4x4 View;
    float4x4 ViewProj;
    uint DrawMeshlets;
};

struct DrawParams
{
    uint InstanceCount;
    uint InstanceOffset;
};

struct MeshInfo
{
    uint IndexBytes;
    uint MeshletCount;
    uint MeshletOffset;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

struct Instance
{
    float4x4 World;
    float4x4 WorldInvTranspose;
};

struct VertexOut
{
    float4 PositionHS : SV_Position;
    float3 PositionVS : POSITION0;
    float3 Normal : NORMAL0;
    uint MeshletIndex : COLOR0;
};

cbuffer Globals : register(b0)
{
    Constants Globals_data;
};

cbuffer DrawParamsCB : register(b1)
{
    DrawParams DrawParams_data;
};

cbuffer MeshInfoCB : register(b2)
{
    MeshInfo MeshInfo_data;
};

StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<Meshlet> Meshlets : register(t1);
ByteAddressBuffer UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);
StructuredBuffer<Instance> Instances : register(t4);

uint3 unpackPrimitive(uint primitive)
{
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 getPrimitive(Meshlet m, uint index)
{
    return unpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

uint getVertexIndex(Meshlet m, uint localIndex)
{
    uint idx = m.VertOffset + localIndex;
    if (MeshInfo_data.IndexBytes == 4)
    {
        return UniqueVertexIndices.Load(idx * 4);
    }
    else
    {
        uint wordOffset = (idx & 0x1);
        uint byteOffset = (idx / 2) * 4;
        uint indexPair = UniqueVertexIndices.Load(byteOffset);
        return (indexPair >> (wordOffset * 16)) & 0xffff;
    }
}

VertexOut getVertexAttributes(uint meshletIndex, uint vertexIndex, uint instanceIndex)
{
    Instance n = Instances[DrawParams_data.InstanceOffset + instanceIndex];
    Vertex v = Vertices[vertexIndex];

    float4 positionWS = mul(float4(v.Position, 1), n.World);

    VertexOut vout;
    vout.PositionVS = mul(positionWS, Globals_data.View).xyz;
    vout.PositionHS = mul(positionWS, Globals_data.ViewProj);
    vout.Normal = mul(float4(v.Normal, 0), n.WorldInvTranspose).xyz;
    vout.MeshletIndex = meshletIndex;

    return vout;
}

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void meshMain(
    uint gtid : SV_GroupIndex,
    uint gid : SV_GroupID,
    out vertices VertexOut verts[MAX_VERTS],
    out indices uint3 tris[MAX_PRIMS])
{
    uint meshletIndex = gid / DrawParams_data.InstanceCount;
    Meshlet m = Meshlets[meshletIndex];

    uint startInstance = gid % DrawParams_data.InstanceCount;
    uint instanceCount = 1;

    if (meshletIndex == MeshInfo_data.MeshletCount - 1)
    {
        uint instancesPerGroup = min(MAX_VERTS / m.VertCount, MAX_PRIMS / m.PrimCount);
        uint unpackedGroupCount = (MeshInfo_data.MeshletCount - 1) * DrawParams_data.InstanceCount;
        uint packedIndex = gid - unpackedGroupCount;

        startInstance = packedIndex * instancesPerGroup;
        instanceCount = min(DrawParams_data.InstanceCount - startInstance, instancesPerGroup);
    }

    uint vertCount = m.VertCount * instanceCount;
    uint primCount = m.PrimCount * instanceCount;

    SetMeshOutputCounts(vertCount, primCount);

    if (gtid < vertCount)
    {
        uint readIndex = gtid % m.VertCount;
        uint instanceId = gtid / m.VertCount;
        uint vertexIndex = getVertexIndex(m, readIndex);
        uint instanceIndex = startInstance + instanceId;
        verts[gtid] = getVertexAttributes(meshletIndex, vertexIndex, instanceIndex);
    }

    if (gtid < primCount)
    {
        uint readIndex = gtid % m.PrimCount;
        uint instanceId = gtid / m.PrimCount;
        tris[gtid] = getPrimitive(m, readIndex) + (m.VertCount * instanceId);
    }
}

struct PSOut
{
    float4 color : SV_TARGET;
};

PSOut psMain(VertexOut input)
{
    PSOut output;

    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;
    if (Globals_data.DrawMeshlets)
    {
        uint meshletIndex = input.MeshletIndex;
        diffuseColor = float3(
            float(meshletIndex & 1),
            float(meshletIndex & 3) / 4,
            float(meshletIndex & 7) / 8);
        shininess = 16.0;
    }
    else
    {
        diffuseColor = 0.8;
        shininess = 64.0;
    }

    float3 normal = normalize(input.Normal);
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.PositionVS);
    float3 halfAngle = normalize(lightDir + viewDir);
    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);
    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * diffuseColor;

    output.color = float4(finalColor, 1);
    return output;
}
