import Scene.RaytracingInline;
import VBufferRT;

ConstantBuffer<VBufferRT> gVBufferRT;

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    if (any(pixel >= gVBufferRT.frameDim))
        return;

    GpuTimer timer;
    gVBufferRT.beginTime(timer);

    const Ray ray = gVBufferRT.generateRay(pixel);

    SceneRayQuery<VBufferRT::kUseAlphaTest> sceneRayQuery;
    float hitT;
    const HitInfo hit = sceneRayQuery.traceRay(ray, hitT, VBufferRT::kRayFlags);
    if (hit.isValid())
    {
        gVBufferRT.writeHit(pixel, ray.origin, ray.dir, hit);
    }
    else
    {
        gVBufferRT.writeMiss(pixel, ray.origin, ray.dir);
    }

    gVBufferRT.writeAux(pixel, ray);

    gVBufferRT.endTime(pixel, timer);
}
