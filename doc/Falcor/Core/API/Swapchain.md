# Swapchain

## Overview
[`Swapchain`](Source/Falcor/Core/API/Swapchain.h:38) is a swapchain management class that provides a mechanism for presenting rendered images to the screen. It manages a set of back buffer images and handles presentation and synchronization with the window system.

## Source Files
- Header: [`Source/Falcor/Core/API/Swapchain.h`](Source/Falcor/Core/API/Swapchain.h) (91 lines)
- Implementation: [`Source/Falcor/Core/API/Swapchain.cpp`](Source/Falcor/Core/API/Swapchain.cpp) (121 lines)

## Class Hierarchy
```
Object (base class)
  └── Swapchain
```

## Dependencies

### Direct Dependencies
- [`fwd.h`](Source/Falcor/Core/API/fwd.h) - Forward declarations
- [`Texture.h`](Source/Falcor/Core/API/Texture.h) - Texture types
- [`Formats.h`](Source/Falcor/Core/API/Formats.h) - Format types
- [`Core/Macros.h`](Source/Falcor/Core/Macros.h) - Macros
- [`Core/Object.h`](Source/Falcor/Core/Object.h) - Base class
- [`Device.h`](Source/Falcor/Core/API/Device.h) - Device (implementation)
- [`GFXAPI.h`](Source/Falcor/Core/API/GFXAPI.h) - GFX API (implementation)
- [`GFXHelpers.h`](Source/Falcor/Core/API/GFXHelpers.h) - GFX helpers (implementation)

### Indirect Dependencies
- Object base class (from Core/Object.h)
- Device class (from Device.h)
- Texture class (from Texture.h)
- ResourceFormat enum (from Formats.h)
- Slang gfx interfaces (gfx::ISwapchain, gfx::ISwapchain::Desc, gfx::ITextureResource)
- WindowHandle type (from Handles.h)

## Known Issues and Limitations

1. **Thread Safety**: Not thread-safe for concurrent access
2. **No Bounds Checking**: No bounds checking for image count
3. **No Validation**: No validation of window handle
4. **No Error Handling**: No error handling for present failures
5. **No Frame Rate Control**: No control over presentation frame rate
6. **No Tear-Free Support**: No explicit tear-free presentation support
7. **No HDR Support**: No explicit HDR support
8. **No Multi-Monitor Support**: No support for multiple monitors
9. **No Color Space Control**: No control over color space
10. **No Presentation Mode Control**: No control over presentation mode (immediate, mailbox, etc.)

## Usage Example

```cpp
// Create a swapchain
Swapchain::Desc desc = {};
desc.format = ResourceFormat::RGBA8Unorm;
desc.width = 1920;
desc.height = 1080;
desc.imageCount = 3;
desc.enableVSync = true;

WindowHandle windowHandle = <get window handle>;
ref<Swapchain> pSwapchain = Swapchain::create(pDevice, desc, windowHandle);

// Get the next image to render to
int imageIndex = pSwapchain->acquireNextImage();
if (imageIndex >= 0)
{
    const ref<Texture>& pBackBuffer = pSwapchain->getImage(imageIndex);
    
    // Render to back buffer
    // <render to pBackBuffer>
    
    // Present the rendered image
    pSwapchain->present();
}

// Resize the swapchain
pSwapchain->resize(2560, 1440);

// Check if window is occluded
bool occluded = pSwapchain->isOccluded();

// Toggle full screen mode
pSwapchain->setFullScreenMode(true);

// Get swapchain descriptor
const Swapchain::Desc& swapchainDesc = pSwapchain->getDesc();
```

## Conclusion

Swapchain provides a straightforward and efficient swapchain management mechanism. The implementation is clean with excellent cache locality but lacks thread safety and advanced presentation features.

**Strengths**:
- Clean and simple API for swapchain management
- Excellent cache locality (members fit in a few cache lines)
- Lightweight present and acquire operations
- Automatic resource management using smart pointers
- Support for resize operations
- Support for windowed and fullscreen modes
- Support for vertical synchronization (VSync)
- Cross-platform support (Windows and Linux)
- Automatic image management

**Weaknesses**:
- Not thread-safe for concurrent access
- No bounds checking for image count
- No validation of window handle
- No error handling for present failures
- No control over presentation frame rate
- No explicit tear-free presentation support
- No explicit HDR support
- No support for multiple monitors
- No control over color space
- No control over presentation mode (immediate, mailbox, etc.)

**Recommendations**:
1. Add thread safety using mutex or atomic operations
2. Add bounds checking for image count
3. Add validation for window handle
4. Add error handling for present failures
5. Consider adding control over presentation frame rate
6. Consider adding explicit tear-free presentation support
7. Consider adding HDR support
8. Consider adding multi-monitor support
9. Consider adding color space control
10. Consider adding presentation mode control (immediate, mailbox, etc.)
11. Consider pre-allocating vector capacity to avoid reallocations
12. Consider using a fixed-size array for images if count is known at compile time
