#pragma once
#include "Utils/HostDeviceShared.slangh"

BEGIN_NAMESPACE_FALCOR

enum class SceneDebuggerMode : uint32_t
{
    FlatShaded,
    TriangleDensity,
    // Geometry
    HitType,
    InstanceID,
    MaterialID,
    GeometryID,
    BlasID,
    PrimitiveID,
    InstancedGeometry,
    MaterialType,
    MeshletID,
    // Shading data
    FaceNormal,
    ShadingNormal,
    ShadingTangent,
    ShadingBitangent,
    FrontFacingFlag,
    BackfacingShadingNormal,
    TexCoords,
    // Material properties
    BSDFProperties,
};

FALCOR_ENUM_INFO(
    SceneDebuggerMode,
    {
        { SceneDebuggerMode::FlatShaded, "FlatShaded" },
        { SceneDebuggerMode::TriangleDensity, "TriangleDensity" },
        // Geometry
        { SceneDebuggerMode::HitType, "HitType" },
        { SceneDebuggerMode::InstanceID, "InstanceID" },
        { SceneDebuggerMode::MaterialID, "MaterialID" },
        { SceneDebuggerMode::GeometryID, "GeometryID" },
        { SceneDebuggerMode::BlasID, "BlasID" },
        { SceneDebuggerMode::PrimitiveID, "PrimitiveID" },
        { SceneDebuggerMode::InstancedGeometry, "InstancedGeometry" },
        { SceneDebuggerMode::MaterialType, "MaterialType" },
        { SceneDebuggerMode::MeshletID, "MeshletID" },
        // Shading data
        { SceneDebuggerMode::FaceNormal, "FaceNormal" },
        { SceneDebuggerMode::ShadingNormal, "ShadingNormal" },
        { SceneDebuggerMode::ShadingTangent, "ShadingTangent" },
        { SceneDebuggerMode::ShadingBitangent, "ShadingBitangent" },
        { SceneDebuggerMode::FrontFacingFlag, "FrontFacingFlag" },
        { SceneDebuggerMode::BackfacingShadingNormal, "BackfacingShadingNormal" },
        { SceneDebuggerMode::TexCoords, "TexCoords" },
        // Material properties
        { SceneDebuggerMode::BSDFProperties, "BSDFProperties" },
    }
);
FALCOR_ENUM_REGISTER(SceneDebuggerMode);

enum class SceneDebuggerBSDFProperty : uint32_t
{
    Emission,
    Roughness,
    GuideNormal,
    DiffuseReflectionAlbedo,
    DiffuseTransmissionAlbedo,
    SpecularReflectionAlbedo,
    SpecularTransmissionAlbedo,
    SpecularReflectance,
    IsTransmissive,
};

FALCOR_ENUM_INFO(
    SceneDebuggerBSDFProperty,
    {
        { SceneDebuggerBSDFProperty::Emission, "Emission" },
        { SceneDebuggerBSDFProperty::Roughness, "Roughness" },
        { SceneDebuggerBSDFProperty::GuideNormal, "GuideNormal" },
        { SceneDebuggerBSDFProperty::DiffuseReflectionAlbedo, "DiffuseReflectionAlbedo" },
        { SceneDebuggerBSDFProperty::DiffuseTransmissionAlbedo, "DiffuseTransmissionAlbedo" },
        { SceneDebuggerBSDFProperty::SpecularReflectionAlbedo, "SpecularReflectionAlbedo" },
        { SceneDebuggerBSDFProperty::SpecularTransmissionAlbedo, "SpecularTransmissionAlbedo" },
        { SceneDebuggerBSDFProperty::SpecularReflectance, "SpecularReflectance" },
        { SceneDebuggerBSDFProperty::IsTransmissive, "IsTransmissive" },
    }
);
FALCOR_ENUM_REGISTER(SceneDebuggerBSDFProperty);

struct SceneDebuggerParams
{
    uint mode = (uint)SceneDebuggerMode::FaceNormal; ///< Current visualization mode. See SceneDebuggerMode.
    uint2 frameDim = { 0, 0 };
    uint frameCount = 0;

    uint bsdfProperty = 0; ///< Current BSDF property. See SceneDebuggerBSDFProperty.
    uint bsdfIndex = 0;    ///< Current BSDF layer index.

    uint2 selectedPixel = { 0, 0 }; ///< The currently selected pixel for readback.
    int flipSign = false;           ///< Flip sign before visualization.
    int remapRange = true;          ///< Remap valid range to [0,1] before output.
    int clamp = true;               ///< Clamp pixel values to [0,1] before output.

    int showVolumes = true;         ///< Show volumes.
    float volumeDensityScale = 1.f; ///< Volume density scale factor.
    int useVBuffer = false;

    int profileSecondaryRays = false;       ///< Trace secondary rays for profiling purposes.
    int profileSecondaryLoadHit = false;    ///< Load HitInfo for secondary rays, otherwise just visibility rays.
    float profileSecondaryConeAngle = 90.f; ///< Cone angle (degress) for randomly distributed ray directions.

    float2 triangleDensityLogRange = { -16.f, 16.f };
};

struct PixelData
{
    static constexpr uint kInvalidID = 0xffffffff;

    // Geometry
    uint hitType;
    uint instanceID;
    uint materialID;
    uint geometryID;
    uint blasID;

    // ShadingData
    float3 posW;
    float3 V;
    float3 N;
    float3 T;
    float3 B;
    float2 uv;
    float3 faceN;
    float4 tangentW;
    uint frontFacing;
    float curveRadius;
};

enum class InstanceInfoFlags : uint32_t
{
    IsInstanced = 0x1,
};

struct InstanceInfo
{
    uint flags = 0; ///< Flags as a combination of 'InstanceInfoFlags' flags.
};

// Meshlet data structures
struct GpuMeshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
    float3 boundCenter;
    float boundRadius;
};

struct MeshletData
{
    uint meshletCount;
    uint totalTriangles;
    uint originalTriangleCount;
    uint _pad0;
};

END_NAMESPACE_FALCOR
