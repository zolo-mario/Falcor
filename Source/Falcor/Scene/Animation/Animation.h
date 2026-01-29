#pragma once
#include "Core/Macros.h"
#include "Core/Object.h"
#include "Scene/SceneIDs.h"
#include "Utils/Math/Vector.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Math/Quaternion.h"
#include "Utils/UI/Gui.h"
#include <fstd/span.h>
#include <memory>
#include <string>
#include <vector>

namespace Falcor
{
    class AnimationController;

    class FALCOR_API Animation : public Object
    {
        FALCOR_OBJECT(Animation)
    public:
        enum class InterpolationMode
        {
            Linear,
            Hermite,
        };

        enum class Behavior
        {
            Constant,
            Linear,
            Cycle,
            Oscillate,
        };

        struct Keyframe
        {
            double time = 0;
            float3 translation = float3(0, 0, 0);
            float3 scaling = float3(1, 1, 1);
            quatf rotation = quatf::identity();
        };

        static ref<Animation> create(std::string_view name, NodeID nodeID, double duration) { return make_ref<Animation>(name, nodeID, duration); }

        /** Create a new animation.
            \param[in] name Animation name.
            \param[in] nodeID ID of the animated node.
            \param[in] Animation duration in seconds.
        */
        Animation(std::string_view name, NodeID nodeID, double duration);

        /** Get the animation name.
        */
        const std::string& getName() const { return mName; }

        /** Get the animated node.
        */
        NodeID getNodeID() const { return mNodeID; }

        /** Set the animated node.
        */
        void setNodeID(NodeID id) { mNodeID = id; }

        /** Get the animation duration in seconds.
        */
        double getDuration() const { return mDuration; }

        /** Get the animation's behavior before the first keyframe.
        */
        Behavior getPreInfinityBehavior() const { return mPreInfinityBehavior; }

        /** Set the animation's behavior before the first keyframe.
        */
        void setPreInfinityBehavior(Behavior behavior) { mPreInfinityBehavior = behavior; }

        /** Get the animation's behavior after the last keyframe.
        */
        Behavior getPostInfinityBehavior() const { return mPostInfinityBehavior; }

        /** Set the animation's behavior after the last keyframe.
        */
        void setPostInfinityBehavior(Behavior behavior) { mPostInfinityBehavior = behavior; }

        /** Get the interpolation mode.
        */
        InterpolationMode getInterpolationMode() const { return mInterpolationMode; }

        /** Set the interpolation mode.
        */
        void setInterpolationMode(InterpolationMode interpolationMode) { mInterpolationMode = interpolationMode; }

        /** Return true if warping is enabled.
        */
        bool isWarpingEnabled() const { return mEnableWarping; }

        /** Enable/disable warping.
        */
        void setEnableWarping(bool enableWarping) { mEnableWarping = enableWarping; }

        /** Add a keyframe.
            If there's already a keyframe at the requested time, this call will override the existing frame.
            \param[in] keyframe Keyframe.
        */
        void addKeyframe(const Keyframe& keyframe);

        /** Get the keyframe at the specified time.
            If the keyframe doesn't exists, the function will throw an exception. If you don't want to handle exceptions, call doesKeyframeExist() first.
            \param[in] time Time of the keyframe.
            \return Returns the keyframe.
        */
        const Keyframe& getKeyframe(double time) const;

        /** Gets all the keyframes in the animation.
            \return Returns span of keyframes.
        */
        fstd::span<const Keyframe> getKeyframes() const { return mKeyframes; }

        /** Check if a keyframe exists at the specified time.
            \param[in] time Time of the keyframe.
            \return Returns true if keyframe exists.
        */
        bool doesKeyframeExists(double time) const;

        /** Compute the animation.
            \param time The current time in seconds. This can be larger then the animation time, in which case the animation will loop.
            \return Returns the animation's transform matrix for the specified time.
        */
        float4x4 animate(double currentTime);

        /* Render the UI.
        */
        void renderUI(Gui::Widgets& widget);

    private:
        Keyframe interpolate(InterpolationMode mode, double time) const;
        double calcSampleTime(double currentTime);

        std::string mName;
        NodeID mNodeID;
        double mDuration; // Includes any time before the first keyframe. May be Assimp or FBX specific.

        Behavior mPreInfinityBehavior = Behavior::Constant; // How the animation behaves before the first keyframe
        Behavior mPostInfinityBehavior = Behavior::Constant; // How the animation behaves after the last keyframe

        InterpolationMode mInterpolationMode = InterpolationMode::Linear;
        bool mEnableWarping = false;

        std::vector<Keyframe> mKeyframes;
        mutable size_t mCachedFrameIndex = 0;

        friend class SceneCache;
    };
}
