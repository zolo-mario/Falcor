import MeshletTypes;

StructuredBuffer<GpuMeshlet> gMeshlets;
StructuredBuffer<MeshletSceneData> gSceneData;
ConstantBuffer<FrustumData> gFrustum;

RWStructuredBuffer<uint> gVisibleMeshletIDs;
RWStructuredBuffer<DrawIndexedIndirectArgs> gIndirectArgs;

bool frustumCullSphere(float3 center, float radius)
{
    [unroll]
    for (int i = 0; i < 6; i++)
    {
        float4 plane = gFrustum.planes[i];
        float dist = dot(plane.xyz, center) + plane.w;

        if (dist < -radius)
            return false;
    }
    return true;
}

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint meshletID = dispatchThreadID.x;

    if (meshletID >= gSceneData[0].meshletCount)
        return;

    GpuMeshlet meshlet = gMeshlets[meshletID];

    bool visible = frustumCullSphere(meshlet.boundCenter, meshlet.boundRadius);

    if (visible)
    {
        uint outIndex = gVisibleMeshletIDs.IncrementCounter();
        gVisibleMeshletIDs[outIndex] = meshletID;

        DrawIndexedIndirectArgs args;
        args.IndexCountPerInstance = meshlet.indexCount;
        args.InstanceCount = 1;
        args.StartIndexLocation = meshlet.indexStart;
        args.BaseVertexLocation = 0;
        args.StartInstanceLocation = 0;

        gIndirectArgs[outIndex] = args;
    }
}
