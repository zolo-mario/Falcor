import MeshletTypes;

// ============ Resource Bindings ============
Texture2D<uint> gVBuffer;           // R32_UINT Visibility Buffer
RWTexture2D<float4> gOutput;        // RGBA output

cbuffer VisualizeCB
{
    uint2 gFrameDim;
    uint gVisualizeMode;  // 0=MeshletID, 1=TriangleID, 2=Combined
    uint gFrameCount;
};

// ============ Main ============
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadID.xy;

    if (any(pixel >= gFrameDim))
        return;

    uint packed = gVBuffer[pixel];

    // Background color
    if (!isValidVBufferValue(packed))
    {
        gOutput[pixel] = float4(0.05f, 0.05f, 0.08f, 1.0f);
        return;
    }

    uint meshletID = unpackMeshletID(packed);
    uint primitiveID = unpackPrimitiveID(packed);

    float3 color;

    switch (gVisualizeMode)
    {
        case 0: // Meshlet ID
            color = meshletIdToColor(meshletID);
            break;

        case 1: // Triangle ID (within meshlet)
            color = triangleIdToColor(0, primitiveID);
            break;

        case 2: // Combined (unique per triangle globally)
        default:
            color = triangleIdToColor(meshletID, primitiveID);
            break;
    }

    gOutput[pixel] = float4(color, 1.0f);
}
