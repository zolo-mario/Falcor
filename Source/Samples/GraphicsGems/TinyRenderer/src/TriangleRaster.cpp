#include "TriangleRaster.h"
#include "Utils/Math/VectorMath.h"

namespace tiny
{

Falcor::float3 barycentric2D(Falcor::float2 a, Falcor::float2 b, Falcor::float2 c, Falcor::float2 p)
{
    // Cross-product formulation (tinyrenderer)
    Falcor::float3 s[2];
    for (int i = 2; i--;)
    {
        s[i][0] = c[i] - a[i];
        s[i][1] = b[i] - a[i];
        s[i][2] = a[i] - p[i];
    }
    Falcor::float3 u = cross(s[0], s[1]);
    if (std::abs(u.z) > 1e-5f)
        return Falcor::float3(1.f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
    return Falcor::float3(-1.f, 1.f, 1.f);
}

} // namespace tiny
