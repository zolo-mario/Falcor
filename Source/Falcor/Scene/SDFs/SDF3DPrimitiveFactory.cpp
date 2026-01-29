#include "SDF3DPrimitiveFactory.h"

using namespace Falcor;

SDF3DPrimitive SDF3DPrimitiveFactory::initCommon(SDF3DShapeType shapeType, const float3& shapeData, float blobbing, float operationSmoothing, SDFOperationType operationType, const Transform& transform)
{
    SDF3DPrimitive primitive = {};
    primitive.shapeType = shapeType;
    primitive.shapeData = shapeData;
    primitive.operationType = operationType;
    primitive.shapeBlobbing = blobbing;
    primitive.operationSmoothing = operationSmoothing;
    primitive.translation = transform.getTranslation();
    primitive.invRotationScale = inverse(float3x3(transform.getMatrix()));
    return primitive;
}

AABB SDF3DPrimitiveFactory::computeAABB(const SDF3DPrimitive& primitive)
{
    AABB aabb;
    float rounding = primitive.shapeBlobbing + float(uint(primitive.operationType) >= uint(SDFOperationType::SmoothUnion)) * primitive.operationSmoothing;

    switch (primitive.shapeType)
    {
    case SDF3DShapeType::Sphere:
    {
        float radius = primitive.shapeData.x + rounding;
        aabb.include(float3(radius, radius, radius));
        aabb.include(float3(-radius, radius, radius));
        aabb.include(float3(radius, -radius, radius));
        aabb.include(float3(radius, radius, -radius));
        aabb.include(float3(-radius, -radius, radius));
        aabb.include(float3(radius, -radius, -radius));
        aabb.include(float3(-radius, radius, -radius));
        aabb.include(float3(-radius, -radius, -radius));
    }
    break;
    case  SDF3DShapeType::Ellipsoid:
    case  SDF3DShapeType::Box:
    {
        float3 halfExtents = primitive.shapeData + float3(rounding);
        aabb.include(float3(halfExtents.x, halfExtents.y, halfExtents.z));
        aabb.include(float3(-halfExtents.x, halfExtents.y, halfExtents.z));
        aabb.include(float3(halfExtents.x, -halfExtents.y, halfExtents.z));
        aabb.include(float3(halfExtents.x, halfExtents.y, -halfExtents.z));
        aabb.include(float3(-halfExtents.x, -halfExtents.y, halfExtents.z));
        aabb.include(float3(halfExtents.x, -halfExtents.y, -halfExtents.z));
        aabb.include(float3(-halfExtents.x, halfExtents.y, -halfExtents.z));
        aabb.include(float3(-halfExtents.x, -halfExtents.y, -halfExtents.z));
    }
    break;
    case  SDF3DShapeType::Torus:
    {
        float smallRadius = rounding;
        float bigRadius = primitive.shapeData.x + rounding;
        aabb.include(float3(bigRadius, smallRadius, bigRadius));
        aabb.include(float3(-bigRadius, smallRadius, bigRadius));
        aabb.include(float3(bigRadius, -smallRadius, bigRadius));
        aabb.include(float3(bigRadius, smallRadius, -bigRadius));
        aabb.include(float3(-bigRadius, -smallRadius, bigRadius));
        aabb.include(float3(bigRadius, -smallRadius, -bigRadius));
        aabb.include(float3(-bigRadius, smallRadius, -bigRadius));
        aabb.include(float3(-bigRadius, -smallRadius, -bigRadius));
    }
    break;
    case  SDF3DShapeType::Cone:
    {
        float tanAngle = primitive.shapeData.x;
        float height = primitive.shapeData.y;
        float radius = tanAngle * height + rounding;
        height += rounding;
        aabb.include(float3(radius, height, radius));
        aabb.include(float3(-radius, height, radius));
        aabb.include(float3(radius, -rounding, radius));
        aabb.include(float3(radius, height, -radius));
        aabb.include(float3(-radius, -rounding, radius));
        aabb.include(float3(radius, -rounding, -radius));
        aabb.include(float3(-radius, height, -radius));
        aabb.include(float3(-radius, -rounding, -radius));
    }
    break;
    case  SDF3DShapeType::Capsule:
    {
        float halfLen = primitive.shapeData.x + rounding;
        aabb.include(float3(rounding, halfLen, rounding));
        aabb.include(float3(-rounding, halfLen, rounding));
        aabb.include(float3(rounding, -halfLen, rounding));
        aabb.include(float3(rounding, halfLen, -rounding));
        aabb.include(float3(-rounding, -halfLen, rounding));
        aabb.include(float3(rounding, -halfLen, -rounding));
        aabb.include(float3(-rounding, halfLen, -rounding));
        aabb.include(float3(-rounding, -halfLen, -rounding));
    }
    break;
    default:
        FALCOR_THROW("SDF Primitive has unknown primitive type");
    }

    float4x4 translate = math::matrixFromTranslation(primitive.translation);
    float4x4 rotScale = inverse(transpose(primitive.invRotationScale));
    return aabb.transform(mul(translate, rotScale));
}
