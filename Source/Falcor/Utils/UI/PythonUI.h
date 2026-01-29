#pragma once

#include "Core/Object.h"

#include <imgui.h>

#include <string>
#include <string_view>
#include <functional>

namespace Falcor
{
namespace python_ui
{

/// Base class for Python UI widgets.
/// Widgets own their children.
class Widget : public Object
{
    FALCOR_OBJECT(Widget)
public:
    Widget(Widget* parent) : m_parent(parent)
    {
        if (m_parent)
            m_parent->m_children.push_back(ref<Widget>(this));
    }

    virtual ~Widget() {}

    Widget* get_parent() { return m_parent; }
    const Widget* get_parent() const { return m_parent; }
    void set_parent(Widget* parent) { m_parent = parent; }

    const std::vector<ref<Widget>>& get_children() const { return m_children; }

    bool get_visible() const { return m_visible; }
    void set_visible(bool visible) { m_visible = visible; }

    bool get_enabled() const { return m_enabled; }
    void set_enabled(bool enabled) { m_enabled = enabled; }

    virtual void render()
    {
        if (m_visible)
            for (const auto& child : m_children)
                child->render();
    }

protected:
    Widget* m_parent;
    std::vector<ref<Widget>> m_children;
    bool m_visible{true};
    bool m_enabled{true};
};

/// This is the main widget that represents the screen.
/// It is intended to be used as the parent for \c Window widgets.
class Screen : public Widget
{
    FALCOR_OBJECT(Screen)
public:
    Screen() : Widget(nullptr) {}

    virtual void render() override { Widget::render(); }
};

// The widgets are implemented in the C++ file as they are only exposed to Python.

} // namespace python_ui
} // namespace Falcor
