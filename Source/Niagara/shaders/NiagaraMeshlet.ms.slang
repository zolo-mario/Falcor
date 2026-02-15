/**
 * Niagara mesh shader - meshlet -> mesh -> PS, no culling.
 * Based on meshlet.mesh.glsl (TASK=false path). Outputs meshletId for PS.
 */

static const uint MESH_WGSIZE = 64;
static const uint MESH_MAXVTX = 64;
static const uint MESH_MAXTRI = 96;

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
    int8_t cone_axis[3];
    int8_t cone_cutoff;
    uint dataOffset;
    uint baseVertex;
    uint8_t vertexCount;
    uint8_t triangleCount;
    uint8_t shortRefs;
    uint8_t padding;
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
    uint lateDrawVisibility;
    uint meshletVisibilityOffset;
};

struct NiagaraCullData
{
    float4x4 view;
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

struct NiagaraGlobals
{
    float4x4 projection;
    NiagaraCullData cullData;
    float screenWidth;
    float screenHeight;
};

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

float3 rotateQuat(float3 v, float4 q)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void unpackTBN(uint np, uint tp, out float3 normal, out float4 tangent)
{
    int3 npv = int3((np >> 0) & 1023, (np >> 10) & 1023, (np >> 20) & 1023);
    normal = float3(npv) / 511.0 - 1.0;
    float2 tpDecoded = float2((tp >> 0) & 255, (tp >> 8) & 255) / 127.0 - 1.0;
    float3 t = float3(tpDecoded, 1.0 - abs(tpDecoded.x) - abs(tpDecoded.y));
    float tVal = max(-t.z, 0.0);
    t.xy += float2(t.x >= 0 ? -tVal : tVal, t.y >= 0 ? -tVal : tVal);
    tangent.xyz = normalize(t);
    tangent.w = (np & (1 << 30)) != 0 ? -1.0 : 1.0;
}

struct MeshVertexOut
{
    float4 posH : SV_Position;
    nointerpolation uint drawId : DRAW_ID;
    nointerpolation uint meshletId : MESHLET_ID;
    float2 uv : TEXCRD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float3 wpos : WPOS;
};

[outputtopology("triangle")]
[numthreads(MESH_WGSIZE, 1, 1)]
void meshMain(
    in uint ti : SV_GroupIndex,
    in uint3 groupID : SV_GroupID,
    OutputVertices<MeshVertexOut, MESH_MAXVTX> verts,
    OutputIndices<uint3, MESH_MAXTRI> triangles)
{
    uint ci = gClusterIndices[groupID.x];
    if (ci == 0xFFFFFFFF)
    {
        SetMeshOutputCounts(0, 0);
        return;
    }

    NiagaraMeshTaskCommand command = gTaskCommands[ci & 0xFFFFFF];
    uint mi = command.taskOffset + (ci >> 24);

    NiagaraMeshDraw meshDraw = gDraws[command.drawId];

    uint vertexCount = gMeshlets[mi].vertexCount;
    uint triangleCount = gMeshlets[mi].triangleCount;

    SetMeshOutputCounts(vertexCount, triangleCount);

    uint dataOffset = gMeshlets[mi].dataOffset;
    uint baseVertex = gMeshlets[mi].baseVertex;
    bool shortRefs = gMeshlets[mi].shortRefs == 1;
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
        float3 position = float3(v.vx, v.vy, v.vz);
        float2 texcoord = float2(v.tu, v.tv);

        float3 normal;
        float4 tangent;
        unpackTBN(v.np, v.tp, normal, tangent);

        normal = rotateQuat(normal, meshDraw.orientation);
        tangent.xyz = rotateQuat(tangent.xyz, meshDraw.orientation);

        float3 wpos = rotateQuat(position, meshDraw.orientation) * meshDraw.scale + meshDraw.position;
        float4 clip = mul(gGlobals.projection, mul(gGlobals.cullData.view, float4(wpos, 1)));

        MeshVertexOut outVtx;
        outVtx.posH = clip;
        outVtx.drawId = command.drawId;
        outVtx.meshletId = mi;
        outVtx.uv = texcoord;
        outVtx.normal = normal;
        outVtx.tangent = tangent;
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

/**
 * Niagara pixel shader - G-Buffer + meshlet ID output.
 */
struct NiagaraMaterial
{
    int albedoTexture;
    int normalTexture;
    int specularTexture;
    int emissiveTexture;
    float4 diffuseFactor;
    float4 specularFactor;
    float3 emissiveFactor;
};

StructuredBuffer<NiagaraMaterial> gMaterials : register(t6);
Texture2D gTextures[] : register(t7);
SamplerState gSampler : register(s0);

float2 encodeOctPS(float3 v)
{
    float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    float2 s = float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
    return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * s) : p;
}

float3 fromSrgbPS(float3 c)
{
    return pow(c, float3(2.2, 2.2, 2.2));
}

float3 toSrgbPS(float3 c)
{
    return pow(c, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

float gradientNoisePS(float2 uv)
{
    return frac(52.9829189 * frac(dot(uv, float2(0.06711056, 0.00583715))));
}

struct PSOut
{
    float4 gbuffer0 : SV_TARGET0;
    float4 gbuffer1 : SV_TARGET1;
    uint meshletId : SV_TARGET2;
};

PSOut psMain(MeshVertexOut inData)
{
    NiagaraMeshDraw meshDraw = gDraws[inData.drawId];
    NiagaraMaterial material = gMaterials[meshDraw.materialIndex];

    float deband = gradientNoisePS(inData.posH.xy) * 2 - 1;

    float4 albedo = material.diffuseFactor;
    if (material.albedoTexture > 0)
        albedo *= fromSrgbPS(gTextures[material.albedoTexture].Sample(gSampler, inData.uv));

    float3 nmap = float3(0, 0, 1);
    if (material.normalTexture > 0)
        nmap = gTextures[material.normalTexture].Sample(gSampler, inData.uv).rgb * 2 - 1;

    float4 specgloss = material.specularFactor;
    if (material.specularTexture > 0)
        specgloss *= fromSrgbPS(gTextures[material.specularTexture].Sample(gSampler, inData.uv));

    float3 emissive = material.emissiveFactor;
    if (material.emissiveTexture > 0)
        emissive *= fromSrgbPS(gTextures[material.emissiveTexture].Sample(gSampler, inData.uv)).rgb;

    float3 bitangent = cross(inData.normal, inData.tangent.xyz) * inData.tangent.w;
    float3 nrm = normalize(nmap.r * inData.tangent.xyz + nmap.g * bitangent + nmap.b * inData.normal);

    float emissivef = dot(emissive, float3(0.3, 0.6, 0.1)) / (dot(albedo.rgb, float3(0.3, 0.6, 0.1)) + 1e-3);

    PSOut outData;
    outData.gbuffer0 = float4(toSrgbPS(albedo.rgb), log2(1 + emissivef) / 5);
    outData.gbuffer1 = float4(encodeOctPS(nrm) * 0.5 + 0.5 + deband * (0.5 / 1023), specgloss.a, 0.0);
    outData.meshletId = inData.meshletId;

    return outData;
}
