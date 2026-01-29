#pragma once
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Math/Quaternion.h"

namespace Falcor
{
    /** Helper to create transformation matrices based on translation,
        rotation and scaling. These operations are applied in order of
        scaling, rotation, and translation last.
    */
    class FALCOR_API Transform
    {
    public:
        enum class CompositionOrder
        {
            Unknown = 0,
            ScaleRotateTranslate,
            ScaleTranslateRotate,
            RotateScaleTranslate,
            RotateTranslateScale,
            TranslateRotateScale,
            TranslateScaleRotate,
            Default = ScaleRotateTranslate
        };

        Transform();

        const float3& getTranslation() const { return mTranslation; }
        void setTranslation(const float3& translation);

        const float3& getScaling() const { return mScaling; }
        void setScaling(const float3& scaling);

        const quatf& getRotation() const { return mRotation; }
        void setRotation(const quatf& rotation);

        float3 getRotationEuler() const;
        void setRotationEuler(const float3& angles);

        float3 getRotationEulerDeg() const;
        void setRotationEulerDeg(const float3& angles);

        void lookAt(const float3& position, const float3& target, const float3& up);

        CompositionOrder getCompositionOrder() const { return mCompositionOrder; } ;
        void setCompositionOrder(const CompositionOrder& order) { mCompositionOrder = order; mDirty = true; }

        const float4x4& getMatrix() const;

        bool operator==(const Transform& other) const;
        bool operator!=(const Transform& other) const { return !((*this) == other); }

        static CompositionOrder getInverseOrder(const CompositionOrder& order);

    private:
        float3 mTranslation = float3(0.f);
        float3 mScaling = float3(1.f);
        quatf mRotation = quatf::identity();
        CompositionOrder mCompositionOrder = CompositionOrder::Default;

        mutable bool mDirty = true;
        mutable float4x4 mMatrix;

        friend class SceneCache;
    };
}
