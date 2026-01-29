#pragma once
#include "USDHelpers.h"
#include "USDUtils.h"
#include "Scene/SceneBuilder.h"

BEGIN_DISABLE_USD_WARNINGS
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usd/object.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdGeom/tokens.h>
#include <pxr/usd/usdSkel/tokens.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
END_DISABLE_USD_WARNINGS

#include <string>
#include <functional>

using namespace pxr;

namespace Falcor
{
inline SceneBuilder::Node makeNode(const std::string& name, NodeID parentId = NodeID::Invalid())
{
    return SceneBuilder::Node{name, float4x4::identity(), float4x4::identity(), float4x4::identity(), parentId};
}

inline SceneBuilder::Node makeNode(
    const std::string& name,
    const float4x4& xform,
    const float4x4& bindTransform,
    NodeID parentId = NodeID::Invalid()
)
{
    return SceneBuilder::Node{name, xform, bindTransform, float4x4::identity(), parentId};
}

using AttributeFrequency = SceneBuilder::Mesh::AttributeFrequency;

inline size_t computeElementCount(AttributeFrequency freq, size_t faceCount, size_t vertexCount)
{
    if (freq == AttributeFrequency::Constant)
    {
        return 1;
    }
    else if (freq == AttributeFrequency::Uniform)
    {
        return faceCount;
    }
    else if (freq == AttributeFrequency::Vertex)
    {
        return vertexCount;
    }
    else if (freq == AttributeFrequency::FaceVarying)
    {
        return 3 * faceCount;
    }
    else
    {
        logError("Unsupported primvar interpolation mode {}.", (uint32_t)freq);
        return 0;
    }
}

// Compute the count of per-face elements, based on interpolation type
inline size_t computePerFaceElementCount(AttributeFrequency freq, size_t faceCount)
{
    if (freq == AttributeFrequency::Uniform)
    {
        return faceCount;
    }
    else if (freq == AttributeFrequency::FaceVarying)
    {
        return 3 * faceCount;
    }
    // Everything else is indexed (vertex, varying), or constant
    return 0;
}

inline AttributeFrequency convertInterpolation(const TfToken& mode)
{
    if (mode == UsdGeomTokens->constant)
    {
        return AttributeFrequency::Constant;
    }
    else if (mode == UsdGeomTokens->uniform)
    {
        return AttributeFrequency::Uniform;
    }
    else if (mode == UsdGeomTokens->vertex || mode == UsdGeomTokens->varying)
    {
        // For our purposes, vertex and varying are synonymous.
        return AttributeFrequency::Vertex;
    }
    else if (mode == UsdGeomTokens->faceVarying)
    {
        return AttributeFrequency::FaceVarying;
    }
    else
    {
        logError("Unknown vertex interpolation mode '{}'.", mode.GetString());
        return AttributeFrequency::None;
    }
}
} // namespace Falcor
