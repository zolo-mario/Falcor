#pragma once
#include "Camera.h"
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include "Utils/Math/Matrix.h"
#include "Utils/Timing/CpuTimer.h"
#include <bitset>
#include <memory>

namespace Falcor
{
    struct MouseEvent;
    struct KeyboardEvent;
    struct GamepadState;

    /** Camera controller interface. Camera controllers should inherit from this object.
    */
    class FALCOR_API CameraController
    {
    public:
        enum class UpDirection
        {
            XPos, XNeg, YPos, YNeg, ZPos, ZNeg,
        };

        virtual ~CameraController() = default;

        /** Handle mouse events
        */
        virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }

        /** Handle keyboard events
        */
        virtual bool onKeyEvent(const KeyboardEvent& keyboardEvent) { return false; }

        /** Handle gamepad state.
        */
        virtual bool onGamepadState(const GamepadState& gamepadState) { return false; }

        /** Update the camera position and orientation.
            \return Whether the camera was updated/changed
        */
        virtual bool update() = 0;

        /** Set the world up-direction.
        */
        void setUpDirection(UpDirection upDirection) { mUpDirection = upDirection; }

        /** Get the world up-direction.
        */
        UpDirection getUpDirection() const { return mUpDirection; }

        /** Set the camera's speed
            \param[in] Speed Camera speed. Measured in WorldUnits per second.
        */
        void setCameraSpeed(float speed) { mSpeed = speed; }

        /** Get the camera's speed.
        */
        float getCameraSpeed() const { return mSpeed; }

        /** Reset the key, mouse, and gamepad states to release all buttons.
        */
        virtual void resetInputState() {};

        /** Set the camera's bounds
            \param[in] aabb Camera bound AABB; position will be clipped to the interior.
        */
        void setCameraBounds(const AABB& aabb) { mBounds = aabb; }

    protected:
        CameraController(const ref<Camera>& pCamera) : mpCamera(pCamera) {}

        float3 getUpVector() const;

        ref<Camera> mpCamera;
        UpDirection mUpDirection = UpDirection::YPos;
        float mSpeed = 1;
        AABB mBounds;
    };

    /** An orbiter camera controller. Orbits around a given point.
        To control the camera:
        * Left mouse click + movement will orbit around the model.
        * Mouse wheel zooms in/out.
    */
    class FALCOR_API OrbiterCameraController : public CameraController
    {
    public:
        OrbiterCameraController(const ref<Camera>& pCamera) : CameraController(pCamera) {}

        /** Handle mouse events
        */
        bool onMouseEvent(const MouseEvent& mouseEvent) override;

        /** Set the model parameters
            \param[in] Center The model's center. This is the position in which the camera will orbit around.
            \param[in] Radius The model's radius. Used to determin the speed of movement when zooming in/out.
            \param[in] InitialDistanceInRadius The initial distance of the camera from the model, measured in the model's radius.
        */
        void setModelParams(const float3& center, float radius, float initialDistanceInRadius);

        /** Update the camera position and orientation.
            \return Whether the camera was updated/changed
        */
        bool update() override;

        /** Reset the mouse state to release all mouse buttons.
        */
        void resetInputState() override;

    private:
        float3 mModelCenter;
        float mModelRadius;
        float mCameraDistance;
        bool mbDirty;

        float3x3 mRotation = float3x3::identity();
        float3 mLastVector;
        bool mIsLeftButtonDown = false;
        bool mShouldRotate = false;
    };

    /** First person camera controller.
        If b6DoF is false, camera will behave like a regular FPS camera. If b6DoF is true, camera will be able to roll as well.
        Controls:
        - W/S/A/D to move forward/backward/strafe left/strafe right.
        - Q/E to move down/up.
        - Left mouse button + mouse movement to rotate camera.
        - Right mouse button + mouse movement to roll camera (for 6DoF camera controller only).
        - Shift for faster movement.
        - Ctrl for slower movement.
    */
    template<bool b6DoF>
    class FALCOR_API FirstPersonCameraControllerCommon : public CameraController
    {
    public:
        FirstPersonCameraControllerCommon(const ref<Camera>& pCamera);

        /** Handle mouse events
        */
        bool onMouseEvent(const MouseEvent& mouseEvent) override;

        /** Handle keyboard events
        */
        bool onKeyEvent(const KeyboardEvent& keyboardEvent) override;

        /* Handle gamepad state.
        */
        bool onGamepadState(const GamepadState& gamepadState) override;

        /** Update the camera position and orientation.
            \return Whether the camera was updated/changed
        */
        bool update() override;

        /** Reset the key, mouse, and gamepad states to release all buttons.
        */
        void resetInputState() override;

    private:
        bool mIsLeftButtonDown = false;
        bool mIsRightButtonDown = false;
        bool mShouldRotate = false;

        float2 mLastMousePos;
        float2 mMouseDelta;

        bool mGamepadPresent = false;
        float2 mGamepadLeftStick;
        float2 mGamepadRightStick;
        float mGamepadLeftTrigger;
        float mGamepadRightTrigger;

        CpuTimer mTimer;

        enum Direction
        {
            Forward,
            Backward,
            Right,
            Left,
            Up,
            Down,
            Count
        };

        std::bitset<Direction::Count> mMovement;

        float mSpeedModifier = 1.0f;
    };

    using FirstPersonCameraController = FirstPersonCameraControllerCommon<false>;
    using SixDoFCameraController = FirstPersonCameraControllerCommon<true>;
}
