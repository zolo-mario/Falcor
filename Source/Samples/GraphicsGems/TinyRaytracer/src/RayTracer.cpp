#include "RayTracer.h"

#include "Utils/Math/VectorMath.h"

#include <algorithm>
#include <cmath>

namespace tinyrt
{

using namespace Falcor::math;

namespace
{

using namespace Falcor;

float3 skyBackground(const float3& dir)
{
    const float t = 0.5f * (dir.y + 1.f);
    return lerp(float3(1.f), float3(0.3f, 0.5f, 1.f), t);
}

bool refractDir(const float3& I, const float3& N, float eta, float3& out)
{
    const float cosi = -dot(N, I);
    const float cost2 = 1.f - eta * eta * (1.f - cosi * cosi);
    if (cost2 < 0.f)
        return false;
    out = eta * I + (eta * cosi - std::sqrt(cost2)) * N;
    return true;
}

float schlick(float cos, float etaI, float etaT)
{
    float r0 = (etaI - etaT) / (etaI + etaT);
    r0 = r0 * r0;
    return r0 + (1.f - r0) * std::pow(1.f - cos, 5.f);
}

} // namespace

bool RayTracerScene::raySphere(const Ray& ray, const Sphere& s, float& t)
{
    const float3 oc = ray.origin - s.center;
    const float b = dot(oc, ray.dir);
    const float c = dot(oc, oc) - s.radius * s.radius;
    const float disc = b * b - c;
    if (disc < 0.f)
        return false;
    const float sd = std::sqrt(disc);
    const float t0 = -b - sd;
    const float t1 = -b + sd;
    if (t0 > 1e-4f)
        t = t0;
    else if (t1 > 1e-4f)
        t = t1;
    else
        return false;
    return true;
}

bool RayTracerScene::rayPlaneY(const Ray& ray, float planeY, float& t)
{
    if (std::abs(ray.dir.y) < 1e-8f)
        return false;
    t = (planeY - ray.origin.y) / ray.dir.y;
    return t > 1e-4f;
}

bool RayTracerScene::intersect(const Ray& ray, Hit& outHit) const
{
    outHit.t = 1e30f;
    outHit.sphereIndex = -1;
    outHit.planeIndex = -1;
    for (int i = 0; i < (int)planes.size(); ++i)
    {
        float t;
        if (rayPlaneY(ray, planes[(size_t)i].y, t) && t < outHit.t)
        {
            outHit.t = t;
            outHit.planeIndex = i;
            outHit.sphereIndex = -1;
        }
    }
    for (int i = 0; i < (int)spheres.size(); ++i)
    {
        float t;
        if (raySphere(ray, spheres[(size_t)i], t) && t < outHit.t)
        {
            outHit.t = t;
            outHit.sphereIndex = i;
            outHit.planeIndex = -1;
        }
    }
    return outHit.sphereIndex >= 0 || outHit.planeIndex >= 0;
}

void RayTracerScene::buildScene(int preset)
{
    mPreset = preset;
    spheres.clear();
    planes.clear();
    lights.clear();

    lights.push_back({float3(-20.f, 20.f, 20.f), 1.5f});
    lights.push_back({float3(30.f, 50.f, -25.f), 1.8f});

    if (preset == 1)
    {
        // Minimal: three colored spheres + floor.
        Material ivory;
        ivory.diffuseColor = float3(0.4f, 0.4f, 0.3f);
        ivory.specularStrength = 0.6f;
        ivory.specularExponent = 50.f;
        Material red;
        red.diffuseColor = float3(0.3f, 0.1f, 0.1f);
        red.specularStrength = 0.3f;
        red.specularExponent = 10.f;
        Material green;
        green.diffuseColor = float3(0.1f, 0.3f, 0.1f);
        green.specularStrength = 0.3f;
        green.specularExponent = 10.f;

        Material floorMat;
        floorMat.checkerboard = true;

        planes.push_back({-2.f, floorMat});
        spheres.push_back({float3(0.f, -0.8f, -3.f), 1.2f, ivory});
        spheres.push_back({float3(-1.4f, -1.f, -2.5f), 0.8f, red});
        spheres.push_back({float3(1.4f, -1.f, -2.5f), 0.8f, green});
        return;
    }

    // Preset 0: tinyraytracer-like (floor + ivory + red + mirror + glass).
    Material floorMat;
    floorMat.checkerboard = true;

    Material ivory;
    ivory.diffuseColor = float3(0.4f, 0.4f, 0.3f);
    ivory.specularStrength = 0.6f;
    ivory.specularExponent = 50.f;

    Material red;
    red.diffuseColor = float3(0.3f, 0.1f, 0.1f);
    red.specularStrength = 0.3f;
    red.specularExponent = 10.f;

    Material mirror;
    mirror.diffuseColor = float3(1.f, 1.f, 1.f);
    mirror.specularStrength = 0.8f;
    mirror.specularExponent = 1425.f;
    mirror.reflectivity = 0.85f;

    Material glass;
    glass.diffuseColor = float3(0.f, 0.f, 0.f);
    glass.specularStrength = 0.9f;
    glass.specularExponent = 125.f;
    glass.reflectivity = 0.15f;
    glass.refractiveIndex = 1.5f;

    planes.push_back({-2.f, floorMat});
    spheres.push_back({float3(0.f, -0.75f, -3.5f), 1.05f, ivory});
    spheres.push_back({float3(-1.4f, -0.95f, -2.8f), 0.65f, red});
    spheres.push_back({float3(1.35f, -0.95f, -2.8f), 0.65f, mirror});
    spheres.push_back({float3(0.f, 0.15f, -3.2f), 0.75f, glass});
}

float3 RayTracerScene::castRay(const Ray& ray, int depth, int maxDepth) const
{
    if (depth > maxDepth)
        return skyBackground(ray.dir);

    Hit hit;
    if (!intersect(ray, hit))
        return skyBackground(ray.dir);

    const float3 hitPoint = ray.origin + ray.dir * hit.t;

    float3 Ngeom;
    float3 N;
    bool inside = false;
    const Material* pMat = nullptr;

    if (hit.planeIndex >= 0)
    {
        const PlaneY& pl = planes[(size_t)hit.planeIndex];
        pMat = &pl.material;
        N = float3(0.f, 1.f, 0.f);
        if (dot(ray.dir, N) > 0.f)
            N = -N;
        Ngeom = N;
    }
    else
    {
        const Sphere& sp = spheres[(size_t)hit.sphereIndex];
        pMat = &sp.material;
        Ngeom = (hitPoint - sp.center) * (1.f / sp.radius);
        inside = dot(ray.dir, Ngeom) > 0.f;
        N = inside ? -Ngeom : Ngeom;
        N = normalize(N);
    }

    const Material& m = *pMat;

    float3 diffuseColor = m.diffuseColor;
    if (m.checkerboard)
    {
        const float checker =
            std::fmod(std::floor(hitPoint.x * 0.1f) + std::floor(hitPoint.z * 0.1f), 2.f);
        diffuseColor = (checker > 0.5f) ? float3(0.3f, 0.3f, 0.3f) : float3(0.3f, 0.2f, 0.1f);
    }

    float diffuseAcc = 0.f;
    float specAcc = 0.f;
    for (const auto& L : lights)
    {
        const float3 lightDir = normalize(L.position - hitPoint);
        const float lightDist = length(L.position - hitPoint);
        const float3 shadowOrig = hitPoint + N * 1e-3f;
        Ray shadowRay{shadowOrig, lightDir};
        Hit sh;
        if (intersect(shadowRay, sh) && sh.t < lightDist - 1e-3f)
            continue;
        const float diff = std::max(0.f, dot(lightDir, N));
        diffuseAcc += L.intensity * diff;
        const float3 reflLight = reflect(-lightDir, N);
        const float spec = std::pow(std::max(0.f, dot(reflLight, -ray.dir)), m.specularExponent);
        specAcc += L.intensity * spec;
    }

    float3 shade = diffuseColor * diffuseAcc + float3(1.f) * (specAcc * m.specularStrength);

    const bool glass = m.refractiveIndex > 1.01f;

    float3 reflectCol{0.f};
    float3 refractCol{0.f};
    if (depth < maxDepth)
    {
        const float3 reflectDir = normalize(reflect(ray.dir, N));
        const float3 reflectOrig = hitPoint + N * 1e-3f;
        reflectCol = castRay(Ray{reflectOrig, reflectDir}, depth + 1, maxDepth);

        if (glass && hit.sphereIndex >= 0)
        {
            const float eta = inside ? m.refractiveIndex : (1.f / m.refractiveIndex);
            const float3 nRefr = inside ? -Ngeom : Ngeom;
            float3 refrDir;
            if (refractDir(ray.dir, nRefr, eta, refrDir))
            {
                refrDir = normalize(refrDir);
                const float3 refrOrig = inside ? hitPoint + Ngeom * 1e-3f : hitPoint - Ngeom * 1e-3f;
                refractCol = castRay(Ray{refrOrig, refrDir}, depth + 1, maxDepth);
            }
            else
                refractCol = reflectCol;

            const float cosi = std::clamp(std::abs(dot(Ngeom, ray.dir)), 0.f, 1.f);
            const float etaI = inside ? m.refractiveIndex : 1.f;
            const float etaT = inside ? 1.f : m.refractiveIndex;
            const float kr = schlick(cosi, etaI, etaT);
            return shade * 0.15f + reflectCol * kr + refractCol * (1.f - kr);
        }

        if (m.reflectivity > 0.01f)
            shade = shade * (1.f - m.reflectivity) + reflectCol * m.reflectivity;
    }

    return shade;
}

} // namespace tinyrt
