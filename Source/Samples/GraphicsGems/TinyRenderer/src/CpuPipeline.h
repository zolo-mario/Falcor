#pragma once

#include <cfloat>

#include "Falcor.h"
#include "Scene/TriangleMesh.h"

namespace tiny
{

/// CPU-side color + depth buffer (RGBA8). Depth: smaller NDC z = closer; clear far with FLT_MAX so z==1 still draws.
struct CpuFramebuffer
{
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> color;
    std::vector<float> depth;

    void resize(uint32_t w, uint32_t h);
    void clear(const Falcor::float3& rgb, float depthClear = FLT_MAX);
    bool setPixel(int x, int y, const Falcor::float3& rgb, float z);
};

/// Vertex shader output / fragment shader input (world-space normal for shading).
struct VertexOut
{
    Falcor::float4 clip;
    Falcor::float3 worldNormal;
};

/// Lambert shading on CPU (tinyrenderer-style stages: vertex → raster → fragment).
class LambertCpuShader
{
public:
    Falcor::float4x4 model = Falcor::float4x4::identity();
    Falcor::float4x4 mvp = Falcor::float4x4::identity();

    VertexOut vertex(const Falcor::TriangleMesh::Vertex& v) const;
    void fragment(const Falcor::float3& interpolatedNormal, Falcor::float3& outRgb) const;
};

/// Rasterize one triangle (clip → viewport → 2D barycentric → depth → fragment).
void drawTriangle(CpuFramebuffer& fb, const VertexOut& v0, const VertexOut& v1, const VertexOut& v2, const LambertCpuShader& shader);

/// Draw entire mesh (indexed triangles).
void drawMesh(CpuFramebuffer& fb, const Falcor::TriangleMesh* pMesh, const LambertCpuShader& shader);

} // namespace tiny
