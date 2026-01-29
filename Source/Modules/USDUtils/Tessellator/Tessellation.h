#include "Core/Error.h"

#include "USDUtils/USDUtils.h"
#include "USDUtils/USDHelpers.h"

BEGIN_DISABLE_USD_WARNINGS
#include <pxr/usd/usdGeom/mesh.h>
END_DISABLE_USD_WARNINGS

namespace Falcor
{

/**
 * Mesh topology information.
 */
struct MeshTopology
{
    MeshTopology() {}
    MeshTopology(pxr::TfToken scheme, pxr::TfToken orient, VtIntArray& faceCounts, VtIntArray& faceIndices)
        : scheme(scheme), orient(orient), faceCounts(faceCounts), faceIndices(faceIndices)
    {}
    pxr::TfToken scheme;         ///< Subdivision scheme, "none", "catmullClark", etc.
    pxr::TfToken orient;         ///< Orientation, nominally "leftHanded" or "rightHanded"
    pxr::VtIntArray faceCounts;  ///< Per-face number of vertices.
    pxr::VtIntArray faceIndices; ///< Per-face-vertex indices.
    pxr::VtIntArray holeIndices; ///< Indices of hole faces (sorted, per USD spec).

    uint32_t getNumFaces() const { return faceCounts.size(); }

    VtVec3iArray getTriangleIndices() const
    {
        FALCOR_ASSERT((faceIndices.size() % 3) == 0);

        VtVec3iArray ret;
        for (uint32_t i = 0; i < faceIndices.size(); i += 3)
        {
            ret.push_back(GfVec3i(faceIndices[i + 0], faceIndices[i + 1], faceIndices[i + 2]));
        }
        return ret;
    }
};

/**
 * A Basic mesh, as represented using USD datatypes.
 */
struct UsdMeshData
{
    MeshTopology topology;     ///< Topology
    pxr::VtVec3fArray points;  ///< Vertex positions
    pxr::VtVec3fArray normals; ///< Shading normals
    pxr::VtVec2fArray uvs;     ///< Texture coordinates
    pxr::TfToken normalInterp; ///< Normal interpolation mode (none, vertex, varying, faceVarying)
    pxr::TfToken uvInterp;     ///< Texture coordinate interpolatoin mode (none, vertex, varying, faceVarying)
};

/**
 * @brief Tessellate a UsdMeshData into triangles
 *
 * @param[in] geomMesh UsdGeomMesh being tessellated; used to extract subdivision and tessellation attributes.
 * @param[in] baseMesh Base mesh to tessellate.
 * @param[in] maxRefinementLevel Maximum subdivision refinement level. Zero indicates no subdivision.
 * @param[out] coarseFaceIndices Index of base face from which each output triangle derives.
 * @return UsdMeshData containing tessellated results; points will be zero-length on failure.
 */
UsdMeshData tessellate(
    const pxr::UsdGeomMesh& geomMesh,
    const UsdMeshData& baseMesh,
    uint32_t maxRefinementLevel,
    pxr::VtIntArray& coarseFaceIndices
);
} // namespace Falcor
