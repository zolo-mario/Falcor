# Core - Core System Components

## Module State Machine

**Status**: Complete

## Dependency Graph

### Internal Components (All Complete)

- [x] **AssetResolver** - Asset path resolution system
- [x] **Testbed** - Main application framework
- [x] **Version** - Version management
- [x] **Window** - Window and input management

### External Dependencies

- **Utils/Scripting** - Python bindings
- **Utils/Math** - Vector types (uint2, float2)
- **Platform/OS** - OS services
- **Platform/PlatformHandles** - Platform-specific handles
- **Core/Object** - Base object class
- **Core/API** - Graphics API (Device, Swapchain, Formats)
- **Utils/Logger** - Logging utilities
- **Utils/UI** - GUI and input types
- **Utils/Timing** - Clock and FrameRate
- **Utils/Settings** - Settings management

## Module Overview

The Core module provides fundamental system components for the Falcor framework, including window management, asset resolution, application lifecycle, and version tracking. It serves as the foundation layer that all other Falcor modules depend upon.

## Component Specifications

### AssetResolver

**File**: [`AssetResolver.h`](Source/Falcor/Core/AssetResolver.h:1), [`AssetResolver.cpp`](Source/Falcor/Core/AssetResolver.cpp:1)

**Purpose**: Provides a flexible asset path resolution system with category-based search paths.

**Key Features**:
- Category-based asset resolution (Any, Scene, Texture)
- Search path management with priority ordering (First/Last)
- Pattern-based file resolution using regex
- Canonical path generation
- Global default resolver instance

**Core Methods**:
- [`resolvePath()`](Source/Falcor/Core/AssetResolver.h:103) - Resolve single file path
- [`resolvePathPattern()`](Source/Falcor/Core/AssetResolver.h:118) - Resolve multiple files matching pattern
- [`addSearchPath()`](Source/Falcor/Core/AssetResolver.h:134) - Add search path with priority
- [`getDefaultResolver()`](Source/Falcor/Core/AssetResolver.h:141) - Get global resolver instance

**Resolution Logic** (from [`AssetResolver.cpp:40-61`](Source/Falcor/Core/AssetResolver.cpp:40)):
1. Check if path is absolute and exists → return canonical form
2. Check if path is relative to working directory and exists → return canonical form
3. Try resolving for specified category's search paths
4. If not found, try resolving for `AssetCategory::Any` search paths
5. Return empty path if resolution fails

**Data Structures**:
```cpp
struct SearchContext {
    std::vector<std::filesystem::path> searchPaths;
    std::filesystem::path resolvePath(const std::filesystem::path& path) const;
    std::vector<std::filesystem::path> resolvePathPattern(
        const std::filesystem::path& path,
        const std::regex& regex,
        bool firstMatchOnly
    ) const;
    void addSearchPath(const std::filesystem::path& path, SearchPathPriority priority);
};
```

**Enums**:
- [`AssetCategory`](Source/Falcor/Core/AssetResolver.h:41) - Any, Scene, Texture
- [`SearchPathPriority`](Source/Falcor/Core/AssetResolver.h:60) - First, Last

### Testbed

**File**: [`Testbed.h`](Source/Falcor/Core/Testbed.h:1), [`Testbed.cpp`](Source/Falcor/Core/Testbed.cpp:1)

**Purpose**: Main application framework available through Python API, providing a complete rendering testbed with scene management, render graph execution, and UI integration.

**Key Features**:
- Window and swapchain management
- Scene loading and management
- Render graph creation and execution
- Python UI integration
- Profiling and FPS display
- Keyboard/mouse/gamepad input handling
- Frame capture to file
- Asset import tracking

**Core Methods**:
- [`run()`](Source/Falcor/Core/Testbed.h:81) - Run main loop
- [`frame()`](Source/Falcor/Core/Testbed.h:92) - Render single frame
- [`loadScene()`](Source/Falcor/Core/Testbed.h:98) - Load scene from file
- [`loadSceneFromString()`](Source/Falcor/Core/Testbed.h:101) - Load scene from string
- [`createRenderGraph()`](Source/Falcor/Core/Testbed.h:111) - Create new render graph
- [`loadRenderGraph()`](Source/Falcor/Core/Testbed.h:114) - Load render graph from file
- [`setRenderGraph()`](Source/Falcor/Core/Testbed.h:117) - Set active render graph
- [`captureOutput()`](Source/Falcor/Core/Testbed.h:129) - Capture render graph output to file
- [`resizeFrameBuffer()`](Source/Falcor/Core/Testbed.h:95) - Resize frame buffer

**Frame Execution Flow** (from [`Testbed.cpp:72-140`](Source/Falcor/Core/Testbed.cpp:72)):
1. Update clock and frame rate
2. Poll window events
3. Clear frame buffer
4. Compile render graph (if exists)
5. Update scene (if exists)
6. Execute render graph
7. Blit render graph output to frame buffer
8. Render UI
9. Copy framebuffer to swapchain
10. Present swapchain

**Options Structure**:
```cpp
struct Options {
    ref<Device> pDevice = nullptr;
    Device::Desc deviceDesc;
    Window::Desc windowDesc;
    bool createWindow = false;
    bool showFPS = true;
    ResourceFormat colorFormat = ResourceFormat::BGRA8UnormSrgb;
    ResourceFormat depthFormat = ResourceFormat::D32Float;
};
```

**Input Handling**:
- Implements [`Window::ICallbacks`](Source/Falcor/Core/Testbed.h:51) interface
- Keyboard shortcuts: ESC (exit), F1 (help), F2 (toggle UI), F5 (reload shaders), P (toggle profiler)
- Supports custom keyboard, mouse, and window size change callbacks
- Gamepad input support with connection/disconnection events

**Python Integration**:
- Extensive Python bindings via [`FALCOR_SCRIPT_BINDING`](Source/Falcor/Core/Testbed.cpp:654)
- Exposes all major functionality to Python scripts
- Provides access to scene, render graph, device, profiler, and UI

### Version

**File**: [`Version.h`](Source/Falcor/Core/Version.h:1), [`Version.cpp`](Source/Falcor/Core/Version.cpp:1)

**Purpose**: Version information management with git integration.

**Version Constants**:
- `FALCOR_MAJOR_VERSION` = 8
- `FALCOR_MINOR_VERSION` = 0

**Functions**:
- [`getVersionString()`](Source/Falcor/Core/Version.h:39) - Returns "8.0"
- [`getLongVersionString()`](Source/Falcor/Core/Version.cpp:42) - Returns "8.0 (commit: <hash>, branch: <branch>[, contains uncommitted changes])"

**Git Integration**:
- Reads from `git_version.h` (generated by CMake)
- Includes commit hash, branch name, and dirty state
- Gracefully handles missing git installation

### Window

**File**: [`Window.h`](Source/Falcor/Core/Window.h:1), [`Window.cpp`](Source/Falcor/Core/Window.cpp:1)

**Purpose**: Cross-platform window management using GLFW, providing input handling and event callbacks.

**Key Features**:
- GLFW-based window creation and management
- Keyboard, mouse, and gamepad input handling
- Window resize and mode management (Normal, Minimized, Fullscreen)
- File drag-and-drop support
- Cross-platform (Windows, Linux)
- Gamepad support with custom mappings for NVIDIA controllers

**Core Methods**:
- [`create()`](Source/Falcor/Core/Window.h:96) - Create window with callbacks
- [`resize()`](Source/Falcor/Core/Window.h:120) - Resize window
- [`msgLoop()`](Source/Falcor/Core/Window.h:125) - Start message loop
- [`pollForEvents()`](Source/Falcor/Core/Window.h:130) - Force event polling
- [`handleGamepadInput()`](Source/Falcor/Core/Window.h:135) - Handle gamepad input
- [`setWindowPos()`](Source/Falcor/Core/Window.h:140) - Set window position
- [`setWindowTitle()`](Source/Falcor/Core/Window.h:145) - Set window title
- [`setWindowIcon()`](Source/Falcor/Core/Window.h:150) - Set window icon

**Window Modes**:
- [`WindowMode::Normal`](Source/Falcor/Core/Window.h:57) - Standard windowed mode
- [`WindowMode::Minimized`](Source/Falcor/Core/Window.h:58) - Minimized window
- [`WindowMode::Fullscreen`](Source/Falcor/Core/Window.h:59) - Fullscreen mode

**ICallbacks Interface**:
```cpp
class ICallbacks {
    virtual void handleWindowSizeChange() = 0;
    virtual void handleRenderFrame() = 0;
    virtual void handleKeyboardEvent(const KeyboardEvent& keyEvent) = 0;
    virtual void handleMouseEvent(const MouseEvent& mouseEvent) = 0;
    virtual void handleGamepadEvent(const GamepadEvent& gamepadEvent) = 0;
    virtual void handleGamepadState(const GamepadState& gamepadState) = 0;
    virtual void handleDroppedFile(const std::filesystem::path& path) = 0;
};
```

**Input Handling**:
- Keyboard: Key press/release/repeat events with modifier flags
- Mouse: Button down/up, move, wheel events with position tracking
- Gamepad: Connection/disconnection events, button events, state polling
- File drag-and-drop: Handle dropped file paths

**Gamepad Support** (from [`Window.cpp:557-657`](Source/Falcor/Core/Window.cpp:557)):
- Automatic gamepad detection and connection tracking
- Custom mappings for NVIDIA controllers
- Button event synthesis (button down/up)
- Continuous state polling for analog inputs
- Support for up to 16 gamepads (GLFW_JOYSTICK_LAST)

**Platform-Specific Handles**:
- Windows: `HWND` via `glfwGetWin32Window()`
- Linux: X11 Display and Window via `glfwGetX11Display()` and `glfwGetX11Window()`

## Architecture Patterns

### Singleton Pattern
- [`AssetResolver::getDefaultResolver()`](Source/Falcor/Core/AssetResolver.cpp:101) - Global asset resolver instance

### Callback Pattern
- [`Window::ICallbacks`](Source/Falcor/Core/Window.h:78) - Event-driven architecture for window and input events

### RAII Pattern
- GLFW initialization/shutdown tied to window creation/destruction
- Reference counting for GLFW lifetime management

### Factory Pattern
- [`Window::create()`](Source/Falcor/Core/Window.h:96) - Factory method for window creation
- [`Testbed::create()`](Source/Falcor/Core/Testbed.h:72) - Factory method for testbed creation

## Technical Details

### Asset Resolution Algorithm

The asset resolver uses a hierarchical search strategy:
1. Absolute path check (fast path)
2. Working directory relative check
3. Category-specific search paths
4. Fallback to `AssetCategory::Any` search paths

This allows for flexible asset organization while maintaining performance through early exit conditions.

### Frame Loop Architecture

The Testbed implements a frame loop that:
- Separates rendering from event handling
- Supports both interactive and batch modes
- Maintains consistent timing via [`Clock`](Source/Falcor/Core/Testbed.h:108) and [`FrameRate`](Source/Falcor/Core/Testbed.h:194)
- Provides hooks for custom input handling
- Integrates profiling via [`Profiler`](Source/Falcor/Core/Testbed.h:186)

### Input Processing Pipeline

Input flows through multiple layers:
1. GLFW callbacks (platform layer)
2. [`ApiCallbacks`](Source/Falcor/Core/Window.cpp:40) translation (platform abstraction)
3. Event type conversion (GLFW → Falcor)
4. [`Window::ICallbacks`](Source/Falcor/Core/Window.h:78) dispatch (application layer)
5. Custom callback handlers (user layer)

This multi-layer approach provides clean separation of concerns and extensibility.

## Dependencies

### Internal Dependencies
- None (Core is the foundation layer)

### External Dependencies
- **GLFW** - Window and input management
- **std::filesystem** - Path manipulation (C++17)
- **pybind11** - Python bindings
- **fmt** - String formatting

## Integration Points

### Scene Loading
Testbed integrates with [`SceneBuilder`](Source/Falcor/Scene/SceneBuilder.h:1) to load scenes from files or strings.

### Render Graph Execution
Testbed manages render graph lifecycle including compilation, execution, and output handling.

### UI Integration
Testbed provides UI rendering through [`Gui`](Source/Falcor/Utils/UI/Gui.h:1) and [`python_ui::Screen`](Source/Falcor/Utils/UI/PythonUI.h:1).

### Profiling
Testbed integrates with [`Profiler`](Source/Falcor/Utils/Timing/Profiler.h:1) for performance tracking and visualization.

## Performance Considerations

### Asset Resolution
- Early exit on absolute paths avoids unnecessary searches
- Search path ordering allows for asset override strategies
- Pattern matching uses regex for flexibility (potential performance cost)

### Window Management
- GLFW provides efficient event polling
- Gamepad state polling only occurs when gamepad is connected
- Window resize events trigger minimal updates

### Frame Loop
- Efficient event handling via `pollForEvents()`
- Separate UI rendering pass for clarity
- Profiling can be toggled at runtime to avoid overhead

## Progress Log

- **2026-01-07T18:45:23Z**: Core module analysis completed. All four components (AssetResolver, Testbed, Version, Window) analyzed and documented. Module marked as Complete.

## Next Steps

Proceed to analyze the RenderGraph module to understand the rendering pipeline architecture.
