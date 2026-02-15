// Match D3D12 Pipeline State Cache: QuadVertexShader + UberPixelShader (all 9 post effects)
// Effect indices: PostBlit=2, PostInvert=3, PostGrayScale=4, PostEdgeDetect=5, PostBlur=6,
// PostWarp=7, PostPixelate=8, PostDistort=9, PostWave=10

struct VSInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_tex : register(t0);
SamplerState g_samp : register(s0);

cbuffer EffectCB : register(b0)
{
    uint effectIndex;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    result.position = input.position;
    result.uv = input.uv;
    return result;
}

// Effect implementations - match D3D12 sample 1:1
float4 Blit(float2 uv)
{
    return g_tex.Sample(g_samp, uv);
}

float4 InvertPixel(float2 uv)
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f) - g_tex.Sample(g_samp, uv);
}

float4 GrayScale(float2 uv)
{
    float3 luminance = float3(0.21f, 0.72f, 0.07f);
    float3 color = g_tex.Sample(g_samp, uv).xyz;
    float output_val = dot(luminance, color);
    return float4(output_val, output_val, output_val, 1.0f);
}

static const float edgeThreshold = 0.015f;
static const float edgeSize = 0.001f;
static const float2 samplePoints[9] = {
    float2(-edgeSize, edgeSize),
    float2(0.0f, edgeSize),
    float2(edgeSize, edgeSize),
    float2(-edgeSize, 0.0f),
    float2(0.0f, 0.0f),
    float2(edgeSize, edgeSize),
    float2(-edgeSize, -edgeSize),
    float2(0.0f, -edgeSize),
    float2(edgeSize, -edgeSize),
};

float4 EdgeDetect(float2 uv)
{
    float xKernel[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    float yKernel[9] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
    float x = 0;
    float y = 0;
    for (int i = 0; i < 9; i++)
    {
        float4 color = g_tex.Sample(g_samp, uv + samplePoints[i]);
        x += color.x * xKernel[i];
        y += color.x * yKernel[i];
    }
    if ((x * x + y * y) < edgeThreshold)
        return g_tex.Sample(g_samp, uv + samplePoints[4]);
    else
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

static const float weights[15] = {
    0.1061154f, 0.102850571f, 0.102850571f, 0.09364651f, 0.09364651f,
    0.0801001f, 0.0801001f, 0.06436224f, 0.06436224f, 0.0485831723f,
    0.0485831723f, 0.0344506279f, 0.0344506279f, 0.0229490642f, 0.0229490642f,
};
static const float2 blurOffsets[15] = {
    float2(0.0f, 0.0f),
    float2(0.00375f, 0.006250001f), float2(-0.00375f, -0.006250001f),
    float2(0.00875f, 0.01458333f), float2(-0.00875f, -0.01458333f),
    float2(0.01375f, 0.02291667f), float2(-0.01375f, -0.02291667f),
    float2(0.01875f, 0.03125f), float2(-0.01875f, -0.03125f),
    float2(0.02375f, 0.03958334f), float2(-0.02375f, -0.03958334f),
    float2(0.02875f, 0.04791667f), float2(-0.02875f, -0.04791667f),
    float2(0.03375f, 0.05625f), float2(-0.03375f, -0.05625f),
};

float4 Blur(float2 uv)
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 15; i++)
        color += g_tex.Sample(g_samp, uv + blurOffsets[i]) * weights[i];
    return color;
}

float4 Warp(float2 uv)
{
    float2 dir = uv - float2(0.5f, 0.5f);
    float len = length(dir) * 6.0f;
    float2 newCoord = uv + dir * sin(pow(2.0f, len));
    return g_tex.Sample(g_samp, newCoord);
}

float4 Pixelate(float2 uv)
{
    uint2 var = uint2(uv.x * 100, uv.y * 100);
    uv = float2((float)var.x / 100, (float)var.y / 100);
    return g_tex.Sample(g_samp, uv);
}

float4 Distort(float2 uv)
{
    float pixelDistance = distance(float2(0.5f, 0.5f), uv);
    float weight = 1.0f + pixelDistance;
    float x = (uv.x - 0.5f) + 0.5f;
    float y = (uv.y - 0.5f) + 0.5f;
    x = pow(abs(x), weight);
    y = pow(abs(y), weight);
    return g_tex.Sample(g_samp, float2(x, y));
}

float4 Wave(float2 uv)
{
    float angle = distance(float2(0.5f, 0.5f), uv) * 50.0f;
    uv.x = 0.5f + (uv.x - 0.5f) * cos(angle) - (uv.y - 0.5f) * sin(angle);
    uv.y = 0.5f + (uv.x - 0.5f) * sin(angle) + (uv.y - 0.5f) * cos(angle);
    return g_tex.Sample(g_samp, uv);
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    [branch] switch (effectIndex)
    {
        case 2: color = Blit(input.uv); break;
        case 3: color = InvertPixel(input.uv); break;
        case 4: color = GrayScale(input.uv); break;
        case 5: color = EdgeDetect(input.uv); break;
        case 6: color = Blur(input.uv); break;
        case 7: color = Warp(input.uv); break;
        case 8: color = Pixelate(input.uv); break;
        case 9: color = Distort(input.uv); break;
        case 10: color = Wave(input.uv); break;
        default: break;
    }
    return color;
}
