import Scene.Raster;
import Utils.Sampling.TinyUniformSampleGenerator;
import Rendering.Lights.LightHelpers;

VSOut vsMain(VSIn vIn)
{
    return defaultVS(vIn);
}

float4 psMain(VSOut vsOut, uint triangleIndex: SV_PrimitiveID) : SV_TARGET
{
    let lod = ImplicitLodTextureSampler();
    if (alphaTest(vsOut, triangleIndex, lod))
        discard;

    float3 viewDir = normalize(gScene.camera.getPosition() - vsOut.posW);
    ShadingData sd = prepareShadingData(vsOut, triangleIndex, viewDir);

    // Create material instance.
    let mi = gScene.materials.getMaterialInstance(sd, lod);

    float3 color = mi.getProperties(sd).emission;

    uint2 pixelPos = (uint2)vsOut.posH.xy; 
    uint3 launchIndex = uint3(pixelPos, 0);
    TinyUniformSampleGenerator sg = TinyUniformSampleGenerator(launchIndex.xy, 0);

    // Direct lighting from analytic light sources
    for (int i = 0; i < gScene.getLightCount(); i++)
    {
        AnalyticLightSample ls;
        evalLightApproximate(sd.posW, gScene.getLight(i), ls);
        color += mi.eval(sd, ls.dir, sg) * ls.Li;
    }

    return float4(color, 1.f);
}
