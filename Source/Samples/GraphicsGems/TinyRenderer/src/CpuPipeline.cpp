#include "CpuPipeline.h"
#include "TriangleRaster.h"

#include "Utils/Math/MatrixMath.h"

#include <algorithm>

using namespace Falcor;

namespace tiny
{

void CpuFramebuffer::resize(uint32_t w, uint32_t h)
{
    width = w;
    height = h;
    color.resize(size_t(w) * size_t(h) * 4);
    depth.resize(size_t(w) * size_t(h));
}

void CpuFramebuffer::clear(const float3& rgb, float depthClear)
{
    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            size_t i = (size_t(y) * width + x) * 4;
            color[i + 0] = (uint8_t)std::clamp(rgb.x * 255.f, 0.f, 255.f);
            color[i + 1] = (uint8_t)std::clamp(rgb.y * 255.f, 0.f, 255.f);
            color[i + 2] = (uint8_t)std::clamp(rgb.z * 255.f, 0.f, 255.f);
            color[i + 3] = 255;
            depth[y * width + x] = depthClear;
        }
    }
}

bool CpuFramebuffer::setPixel(int x, int y, const float3& rgb, float z)
{
    if (x < 0 || y < 0 || x >= (int)width || y >= (int)height)
        return false;
    size_t di = size_t(y) * width + size_t(x);
    if (z >= depth[di])
        return false;
    depth[di] = z;
    size_t i = di * 4;
    color[i + 0] = (uint8_t)std::clamp(rgb.x * 255.f, 0.f, 255.f);
    color[i + 1] = (uint8_t)std::clamp(rgb.y * 255.f, 0.f, 255.f);
    color[i + 2] = (uint8_t)std::clamp(rgb.z * 255.f, 0.f, 255.f);
    color[i + 3] = 255;
    return true;
}

VertexOut LambertCpuShader::vertex(const TriangleMesh::Vertex& v) const
{
    VertexOut o;
    o.clip = math::mul(mvp, float4(v.position, 1.f));
    o.worldNormal = math::normalize(math::transformVector(model, v.normal));
    return o;
}

void LambertCpuShader::fragment(const float3& interpolatedNormal, float3& outRgb) const
{
    float3 n = math::normalize(interpolatedNormal);
    float3 lightDir = math::normalize(float3(0.35f, 0.65f, 0.65f));
    float ndl = std::max(0.f, dot(n, lightDir));
    float c = 0.1f + 0.9f * ndl;
    outRgb = float3(c, c, c);
}

void drawTriangle(CpuFramebuffer& fb, const VertexOut& v0, const VertexOut& v1, const VertexOut& v2, const LambertCpuShader& shader)
{
    // Only skip if the whole triangle is behind the camera (all w <= 0).
    if (v0.clip.w <= 1e-5f && v1.clip.w <= 1e-5f && v2.clip.w <= 1e-5f)
        return;

    float3 ndc0 = v0.clip.xyz() / v0.clip.w;
    float3 ndc1 = v1.clip.xyz() / v1.clip.w;
    float3 ndc2 = v2.clip.xyz() / v2.clip.w;

    float w = float(fb.width);
    float h = float(fb.height);
    // NDC x,y in [-1,1], y up in NDC; screen y grows downward.
    float sx0 = (ndc0.x * 0.5f + 0.5f) * w;
    float sy0 = (1.f - (ndc0.y * 0.5f + 0.5f)) * h;
    float sx1 = (ndc1.x * 0.5f + 0.5f) * w;
    float sy1 = (1.f - (ndc1.y * 0.5f + 0.5f)) * h;
    float sx2 = (ndc2.x * 0.5f + 0.5f) * w;
    float sy2 = (1.f - (ndc2.y * 0.5f + 0.5f)) * h;

    float2 p0(sx0, sy0);
    float2 p1(sx1, sy1);
    float2 p2(sx2, sy2);

    float z0 = ndc0.z;
    float z1 = ndc1.z;
    float z2 = ndc2.z;

    float minx = std::min({p0.x, p1.x, p2.x});
    float miny = std::min({p0.y, p1.y, p2.y});
    float maxx = std::max({p0.x, p1.x, p2.x});
    float maxy = std::max({p0.y, p1.y, p2.y});

    int xmin = (int)std::floor(minx);
    int ymin = (int)std::floor(miny);
    int xmax = (int)std::ceil(maxx);
    int ymax = (int)std::ceil(maxy);

    xmin = std::max(0, xmin);
    ymin = std::max(0, ymin);
    xmax = std::min((int)fb.width - 1, xmax);
    ymax = std::min((int)fb.height - 1, ymax);

    for (int y = ymin; y <= ymax; ++y)
    {
        for (int x = xmin; x <= xmax; ++x)
        {
            float2 pix(float(x) + 0.5f, float(y) + 0.5f);
            float3 bc = barycentric2D(p0, p1, p2, pix);
            if (bc.x < 0.f || bc.y < 0.f || bc.z < 0.f)
                continue;
            float z = bc.x * z0 + bc.y * z1 + bc.z * z2;
            float3 n = v0.worldNormal * bc.x + v1.worldNormal * bc.y + v2.worldNormal * bc.z;
            float3 rgb;
            shader.fragment(n, rgb);
            fb.setPixel(x, y, rgb, z);
        }
    }
}

void drawMesh(CpuFramebuffer& fb, const TriangleMesh* pMesh, const LambertCpuShader& shader)
{
    if (!pMesh)
        return;
    const auto& verts = pMesh->getVertices();
    const auto& idx = pMesh->getIndices();
    for (size_t i = 0; i + 2 < idx.size(); i += 3)
    {
        VertexOut o0 = shader.vertex(verts[idx[i]]);
        VertexOut o1 = shader.vertex(verts[idx[i + 1]]);
        VertexOut o2 = shader.vertex(verts[idx[i + 2]]);
        drawTriangle(fb, o0, o1, o2, shader);
    }
}

} // namespace tiny
