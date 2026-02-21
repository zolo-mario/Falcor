/**
 * MeshletRender - D3D12 Mesh Shaders sample migrated to Falcor.
 * Mesh shader + Pixel shader pipeline. No amplification shader.
 * Requires Shader Model 6.5.
 */

struct Constants
{
    float4x4 World;
    float4x4 WorldView;
    float4x4 WorldViewProj;
    uint DrawMeshlets;
};

struct MeshInfo
{
    uint IndexBytes;
    uint MeshletOffset;
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

struct GpuMeshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

cbuffer CB : register(b0)
{
    Constants Globals;
};

ConstantBuffer<MeshInfo> MeshInfoCB : register(b1);

StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<GpuMeshlet> Meshlets : register(t1);
ByteAddressBuffer UniqueVertexIndices : register(t2);
StructuredBuffer<uint> PrimitiveIndices : register(t3);

uint3 unpackPrimitive(uint primitive)
{
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 getPrimitive(GpuMeshlet m, uint index)
{
    return unpackPrimitive(PrimitiveIndices[m.PrimOffset + index]);
}

uint getVertexIndex(GpuMeshlet m, uint localIndex)
{
    localIndex = m.VertOffset + localIndex;
    if (MeshInfoCB.IndexBytes == 4)
    {
        return UniqueVertexIndices.Load(localIndex * 4);
    }
    else
    {
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;
        uint indexPair = UniqueVertexIndices.Load(byteOffset);
        return (indexPair >> (wordOffset * 16)) & 0xffff;
    }
}

VertexOut getVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    Vertex v = Vertices[vertexIndex];
    VertexOut vout;
    vout.PositionVS = mul(float4(v.Position, 1), Globals.WorldView).xyz;
    vout.PositionHS = mul(float4(v.Position, 1), Globals.WorldViewProj);
    vout.Normal = mul(float4(v.Normal, 0), Globals.World).xyz;
    vout.MeshletIndex = meshletIndex;
    return vout;
}

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void meshMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out indices uint3 tris[126],
    out vertices VertexOut verts[64])
{
    GpuMeshlet m = Meshlets[MeshInfoCB.MeshletOffset + gid];

    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        tris[gtid] = getPrimitive(m, gtid);
    }

    if (gtid < m.VertCount)
    {
        uint vertexIndex = getVertexIndex(m, gtid);
        verts[gtid] = getVertexAttributes(gid, vertexIndex);
    }
}

float4 psMain(VertexOut input) : SV_TARGET
{
    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;
    if (Globals.DrawMeshlets)
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

    return float4(finalColor, 1);
}
