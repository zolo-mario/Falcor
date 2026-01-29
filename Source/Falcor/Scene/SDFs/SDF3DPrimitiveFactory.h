#pragma once
#include "SDF3DPrimitiveCommon.slang"
#include "Core/Macros.h"
#include "Utils/Math/AABB.h"
#include "Utils/Math/Vector.h"
#include "Utils/SDF/SDFOperationType.slang"
#include "Scene/Transform.h"

namespace Falcor
{
    class FALCOR_API SDF3DPrimitiveFactory
    {
    public:
        static SDF3DPrimitive initCommon(SDF3DShapeType shapeType, const float3& shapeData, float blobbing, float operationSmoothing, SDFOperationType operationType, const Transform& transform);

        static AABB computeAABB(const SDF3DPrimitive& primitive);
    };
}
