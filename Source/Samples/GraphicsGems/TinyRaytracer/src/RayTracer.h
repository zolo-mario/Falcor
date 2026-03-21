#pragma once

#include "Falcor.h"

#include <vector>

namespace tinyrt
{

struct Ray
{
    Falcor::float3 origin;
    Falcor::float3 dir;
};

struct Material
{
    Falcor::float3 diffuseColor = {1.f, 1.f, 1.f};
    float specularStrength = 0.f;
    float specularExponent = 50.f;
    /// Weight for recursive reflection (0 = none, 1 = mirror-like).
    float reflectivity = 0.f;
    /// 1 = opaque air boundary; >1 (e.g. 1.5) enables refraction path.
    float refractiveIndex = 1.f;
    bool checkerboard = false;
};

struct Sphere
{
    Falcor::float3 center;
    float radius = 1.f;
    Material material;
};

/// Horizontal infinite plane y = constant (Y-up world). Avoids the giant-sphere floor looking like vertical walls from the side.
struct PlaneY
{
    float y = 0.f;
    Material material;
};

struct Light
{
    Falcor::float3 position;
    float intensity = 1.f;
};

/// ssloy/tinyraytracer-style scene: analytic spheres, Phong-ish shading, shadow rays, reflection/refraction.
class RayTracerScene
{
public:
    void buildScene(int preset);

    int getPreset() const { return mPreset; }

    Falcor::float3 castRay(const Ray& ray, int depth, int maxDepth) const;

    std::vector<Sphere> spheres;
    std::vector<PlaneY> planes;
    std::vector<Light> lights;

private:
    struct Hit
    {
        float t = 1e30f;
        int sphereIndex = -1;
        int planeIndex = -1;
    };

    bool intersect(const Ray& ray, Hit& outHit) const;
    static bool raySphere(const Ray& ray, const Sphere& s, float& t);
    static bool rayPlaneY(const Ray& ray, float planeY, float& t);

    int mPreset = 0;
};

} // namespace tinyrt
