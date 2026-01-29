#pragma once
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Scene/SceneIDs.h"
#include "Utils/Math/Matrix.h"
#include <memory>

namespace Falcor
{
    /** Represents an object that has a transform which can be animated using a scene graph node.
    */
    class FALCOR_API Animatable : public Object
    {
        FALCOR_OBJECT(Animatable)
    public:
        virtual ~Animatable() {}

        /** Set if object has animation data.
        */
        void setHasAnimation(bool hasAnimation) { mHasAnimation = hasAnimation; }

        /** Returns true if object has animation data.
        */
        bool hasAnimation() const { return mHasAnimation; }

        /** Enable/disable object animation.
        */
        void setIsAnimated(bool isAnimated) { mIsAnimated = isAnimated; }

        /** Returns true if object animation is enabled.
        */
        bool isAnimated() const { return mIsAnimated; }

        /** Sets the node ID of the animated scene graph node.
        */
        void setNodeID(NodeID nodeID) { mNodeID = nodeID; }

        /** Gets the node ID of the animated scene graph node.
        */
        NodeID getNodeID() const { return mNodeID; }

        /** Update the transform of the animatable object.
        */
        virtual void updateFromAnimation(const float4x4& transform) = 0;

    protected:
        bool mHasAnimation = false;
        bool mIsAnimated = true;
        NodeID mNodeID{ NodeID::Invalid() };
    };
}
