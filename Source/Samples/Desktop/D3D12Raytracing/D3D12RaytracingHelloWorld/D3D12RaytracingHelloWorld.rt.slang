/**
 * D3D12RaytracingHelloWorld - Migrated from DirectX-Graphics-Samples
 * Screen-space raytracing of a single triangle with barycentric coloring.
 */

import Utils.Attributes;

// Match D3D12 RaytracingHlslCompat.h
struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};

// Root resources - gRtScene is a parameter block bound by host
struct RtScene
{
    [root] RaytracingAccelerationStructure rtAccel;
};
ParameterBlock<RtScene> gRtScene;

RWTexture2D<float4> gOutput;
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB;

struct RayPayload
{
    float4 color;
};

bool IsInsideViewport(float2 p, Viewport viewport)
{
    return (p.x >= viewport.left && p.x <= viewport.right)
        && (p.y >= viewport.top && p.y <= viewport.bottom);
}

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    uint2 launchDim = DispatchRaysDimensions().xy;
    float2 lerpValues = float2(launchIndex) / float2(launchDim);

    // Orthographic projection since we're raytracing in screen space.
    float3 rayDir = float3(0, 0, 1);
    float3 origin = float3(
        lerp(g_rayGenCB.viewport.left, g_rayGenCB.viewport.right, lerpValues.x),
        lerp(g_rayGenCB.viewport.top, g_rayGenCB.viewport.bottom, lerpValues.y),
        0.0f);

    if (IsInsideViewport(origin.xy, g_rayGenCB.stencil))
    {
        // Trace the ray.
        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = rayDir;
        ray.TMin = 0.001f;
        ray.TMax = 10000.0f;

        RayPayload payload = { float4(0, 0, 0, 0) };
        TraceRay(gRtScene.rtAccel, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

        gOutput[launchIndex] = payload.color;
    }
    else
    {
        // Render interpolated DispatchRaysIndex outside the stencil window
        gOutput[launchIndex] = float4(lerpValues, 0, 1);
    }
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}
