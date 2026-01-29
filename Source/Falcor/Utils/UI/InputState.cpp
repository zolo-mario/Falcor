#include "InputState.h"
#include "Core/Error.h"

namespace Falcor
{
bool InputState::isModifierDown(Input::Modifier mod) const
{
    return getModifierState(mCurrentKeyState, mod);
}

bool InputState::isModifierPressed(Input::Modifier mod) const
{
    return getModifierState(mCurrentKeyState, mod) == true && getModifierState(mPreviousKeyState, mod) == false;
}

bool InputState::isModifierReleased(Input::Modifier mod) const
{
    return getModifierState(mCurrentKeyState, mod) == false && getModifierState(mPreviousKeyState, mod) == true;
}

void InputState::onKeyEvent(const KeyboardEvent& keyEvent)
{
    // Update the stored key state.
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed || keyEvent.type == KeyboardEvent::Type::KeyReleased)
    {
        mCurrentKeyState[(size_t)keyEvent.key] = keyEvent.type == KeyboardEvent::Type::KeyPressed;
    }
}

void InputState::onMouseEvent(const MouseEvent& mouseEvent)
{
    // Update the stored mouse state.
    if (mouseEvent.type == MouseEvent::Type::ButtonDown || mouseEvent.type == MouseEvent::Type::ButtonUp)
    {
        mCurrentMouseState[(size_t)mouseEvent.button] = mouseEvent.type == MouseEvent::Type::ButtonDown;
    }
    else if (mouseEvent.type == MouseEvent::Type::Move)
    {
        mMouseMoving = true;
    }
}

void InputState::endFrame()
{
    mPreviousKeyState = mCurrentKeyState;
    mPreviousMouseState = mCurrentMouseState;

    mMouseMoving = false;
}

bool InputState::getModifierState(const KeyStates& states, Input::Modifier mod) const
{
    switch (mod)
    {
    case Input::Modifier::Shift:
        return states[(size_t)Input::Key::LeftShift] || states[(size_t)Input::Key::RightShift];
    case Input::Modifier::Ctrl:
        return states[(size_t)Input::Key::LeftControl] || states[(size_t)Input::Key::RightControl];
    case Input::Modifier::Alt:
        return states[(size_t)Input::Key::LeftAlt] || states[(size_t)Input::Key::RightAlt];
    default:
        FALCOR_UNREACHABLE();
        return false;
    }
}
} // namespace Falcor
