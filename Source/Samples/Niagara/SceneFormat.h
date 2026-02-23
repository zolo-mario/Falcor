#pragma once

#include "Falcor.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Falcor
{

namespace NiagaraFormat
{

struct alignas(8) Meshlet
{
    uint16_t center[3];
    uint16_t radius;
    int8_t cone_axis[3];
    int8_t cone_cutoff;
    uint32_t dataOffset;
    uint32_t baseVertex;
    uint8_t vertexCount;
    uint8_t triangleCount;
    uint8_t shortRefs;
    uint8_t padding;
};

struct alignas(16) Material
{
    int albedoTexture;
    int normalTexture;
    int specularTexture;
    int emissiveTexture;
    float4 diffuseFactor;
    float4 specularFactor;
    float3 emissiveFactor;
};

struct alignas(16) MeshDraw
{
    float3 position;
    float scale;
    quatf orientation;
    uint32_t meshIndex;
    uint32_t meshletVisibilityOffset;
    uint32_t postPass;
    uint32_t materialIndex;
};

struct Vertex
{
    uint16_t vx, vy, vz;
    uint16_t tp;
    uint32_t np;
    uint16_t tu, tv;
};

struct MeshLod
{
    uint32_t indexOffset;
    uint32_t indexCount;
    uint32_t meshletOffset;
    uint32_t meshletCount;
    float error;
};

struct alignas(16) Mesh
{
    float3 center;
    float radius;
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t lodCount;
    MeshLod lods[8];
};

struct Geometry
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Meshlet> meshlets;
    std::vector<uint32_t> meshletdata;
    std::vector<uint16_t> meshletvtx0;
    std::vector<Mesh> meshes;
};

struct Camera
{
    float3 position;
    quatf orientation;
    float fovY;
    float znear;
    float4x4 viewMatrix;
};

struct NiagaraSceneFormat
{
    Geometry geometry;
    std::vector<Material> materials;
    std::vector<MeshDraw> draws;
    std::vector<std::string> texturePaths;
    Camera camera;
    float3 sunDirection;
};

}

}
