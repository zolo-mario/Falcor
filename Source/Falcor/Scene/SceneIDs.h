#pragma once
#include "Utils/ObjectID.h"
#include <cstdint>

// Scene1 IDs are now defined in scene1 namespace, so places that need to distinguish multiple MeshIDs can.
namespace Falcor::scene1
{
    enum class SceneObjectKind
    {
        kNode,        ///< NodeID, but also for MatrixID for animation.
        kMesh,        ///< MeshID, also curves that tesselate into triangle mesh.
        kCurve,       ///< CurveID
        kCurveOrMesh, ///< Used when the ID in curves is aliased based on tessellation mode.
        kSdfDesc,     ///< The user-facing ID.
        kSdfGrid,     ///< The internal ID, can get deduplicated.
        kMaterial,
        kLight,
        kCamera,
        kVolume,
        kGlobalGeometry, ///< The linearized global ID, current in order: mest, curve, sdf, custom. Not to be confused with geometryID in curves, which is "either Mesh or Curve, depending on tessellation mode".
    };


    using NodeID = ObjectID<SceneObjectKind, SceneObjectKind::kNode, uint32_t>;
    using MeshID = ObjectID<SceneObjectKind, SceneObjectKind::kMesh, uint32_t>;
    using CurveID = ObjectID<SceneObjectKind, SceneObjectKind::kCurve, uint32_t>;
    using CurveOrMeshID = ObjectID<SceneObjectKind, SceneObjectKind::kCurveOrMesh, uint32_t>;
    using SdfDescID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfDesc, uint32_t>;
    using SdfGridID = ObjectID<SceneObjectKind, SceneObjectKind::kSdfGrid, uint32_t>;
    using MaterialID = ObjectID<SceneObjectKind, SceneObjectKind::kMaterial, uint32_t>;
    using LightID = ObjectID<SceneObjectKind, SceneObjectKind::kLight, uint32_t>;
    using CameraID = ObjectID<SceneObjectKind, SceneObjectKind::kCamera, uint32_t>;
    using VolumeID = ObjectID<SceneObjectKind, SceneObjectKind::kVolume, uint32_t>;
    using GlobalGeometryID = ObjectID<SceneObjectKind, SceneObjectKind::kGlobalGeometry, uint32_t>;
}

// scene1 is by default included in Falcor namespace to keep ensure that everything works are when IDs were directly in Falcor namespace
namespace Falcor
{
using namespace scene1;
}
