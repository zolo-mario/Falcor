# FullScreenPass

## Module Overview

The **FullScreenPass** class provides a full-screen rendering pass for executing pixel shaders across the entire viewport in the Falcor engine. This class extends BaseGraphicsPass to provide specialized functionality for full-screen quad rendering, including shared vertex buffer and VAO management, viewport mask support for multi-projection passes, and automatic viewport/scissor state management. It uses a shared cache to efficiently manage vertex data across multiple pass instances.

## Files

- **Header**: `Source/Falcor/Core/Pass/FullScreenPass.h` (89 lines)
- **Implementation**: `Source/Falcor/Core/Pass/FullScreenPass.cpp` (126 lines)

## Class Definition

```cpp
class FullScreenPass : public BaseGraphicsPass
{
public:
    struct SharedData;

    virtual ~FullScreenPass();

    /**
     * Create a new fullscreen pass from file.
     * @param[in] pDevice GPU device.
     * @param[in] path Pixel shader file path. This method expects a pixel shader named "main()" in the file.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] viewportMask Optional value to initialize viewport mask with. Useful for multi-projection passes.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<FullScreenPass> create(
        ref<Device> pDevice,
        const std::filesystem::path& path,
        const DefineList& defines = DefineList(),
        uint32_t viewportMask = 0
    );

    /**
     * Create a new fullscreen pass.
     * @param[in] pDevice GPU device.
     * @param[in] desc The program description.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] viewportMask Optional value to initialize viewport mask with. Useful for multi-projection passes.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<FullScreenPass> create(
        ref<Device> pDevice,
        const ProgramDesc& desc,
        const DefineList& defines = DefineList(),
        uint32_t viewportMask = 0
    );

    /**
     * Execute the pass using an FBO
     * @param[in] pRenderContext The render context.
     * @param[in] pFbo The target FBO
     * @param[in] autoSetVpSc If true, the pass will set the viewports and scissors to match the FBO size. If you want to override the VP or
     * SC, get the state by calling `getState()`, bind the SC and VP yourself and set this arg to false
     */
    virtual void execute(RenderContext* pRenderContext, const ref<Fbo>& pFbo, bool autoSetVpSc = true) const;

protected:
    FullScreenPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);

private:
    std::shared_ptr<SharedData> mpSharedData;
};

struct FullScreenPass::SharedData
{
    ref<Buffer> pVertexBuffer;
    ref<Vao> pVao;
    uint64_t objectCount = 0;

    SharedData(ref<Device> pDevice);
};
```

## Dependencies

### Internal Dependencies

- **BaseGraphicsPass**: Base class for graphics pass functionality
- **Object**: Base class for reference counting (inherited from BaseGraphicsPass)
- **Device**: Required for device access and resource creation
- **Program**: Required for shader program management (inherited from BaseGraphicsPass)
- **GraphicsState**: Required for graphics state management (inherited from BaseGraphicsPass)
- **ProgramVars**: Required for program variable management (inherited from BaseGraphicsPass)
- **Buffer**: Required for vertex buffer
- **Vao**: Required for vertex array object
- **RenderContext**: Required for rendering execution
- **Fbo**: Required for framebuffer object
- **DepthStencilState**: Required for depth stencil state
- **VertexLayout**: Required for vertex layout
- **VertexBufferLayout**: Required for vertex buffer layout
- **SharedCache**: Required for shared data management

### External Dependencies

- **std::filesystem**: Standard library filesystem for file paths
- **std::string**: Standard library string container
- **std::atomic**: Standard library atomic operations (inherited from Object)
- **std::shared_ptr**: Standard library shared pointer
- **std::size**: Standard library size function

## Cross-Platform Considerations

- **Cross-Platform**: Works on all platforms supported by Falcor
- **No Platform-Specific Code**: Pure C++ implementation
- **Standard Library**: Uses only standard library containers
- **Filesystem**: Uses std::filesystem for cross-platform file paths
- **No Conditional Compilation**: No platform-specific conditional compilation

## Usage Patterns

### Creating a Full-Screen Pass from File

```cpp
ref<Device> pDevice = ...;
std::filesystem::path path = "shaders/fullscreen.slang";
DefineList defines;

ref<FullScreenPass> pPass = FullScreenPass::create(pDevice, path, defines);
```

### Creating a Full-Screen Pass from Description

```cpp
ref<Device> pDevice = ...;
ProgramDesc desc;
desc.addShaderLibrary("shaders/fullscreen.slang").psEntry("main");
DefineList defines;

ref<FullScreenPass> pPass = FullScreenPass::create(pDevice, desc, defines);
```

### Creating a Full-Screen Pass with Viewport Mask

```cpp
ref<Device> pDevice = ...;
std::filesystem::path path = "shaders/fullscreen.slang";
DefineList defines;
uint32_t viewportMask = 0x3; // Enable viewports 0 and 1

ref<FullScreenPass> pPass = FullScreenPass::create(pDevice, path, defines, viewportMask);
```

### Executing a Full-Screen Pass

```cpp
RenderContext* pRenderContext = ...;
ref<Fbo> pFbo = ...;

pPass->execute(pRenderContext, pFbo);
```

### Executing a Full-Screen Pass with Manual Viewport/Scissor

```cpp
RenderContext* pRenderContext = ...;
ref<Fbo> pFbo = ...;

// Set custom viewport and scissor
ref<GraphicsState> pState = pPass->getState();
pState->setViewport(0, viewport);
pState->setScissor(0, scissor);

pPass->execute(pRenderContext, pFbo, false);
```

### Accessing Pass Components

```cpp
ref<Program> pProgram = pPass->getProgram();
const ref<GraphicsState>& pState = pPass->getState();
const ref<ProgramVars>& pVars = pPass->getVars();
ShaderVar rootVar = pPass->getRootVar();
```

### Adding Shader Defines

```cpp
pPass->addDefine("ENABLE_TONEMAPPING", "1");
pPass->addDefine("GAMMA", "2.2");
pPass->addDefine("EXPOSURE", "1.0", true);
```

### Setting Program Variables

```cpp
ref<ProgramVars> pSharedVars = ...;
pPass->setVars(pSharedVars);
```

### Accessing Root Variable

```cpp
ShaderVar rootVar = pPass->getRootVar();
rootVar["inputTexture"] = pInputTexture;
rootVar["params"].setBlob(&params, sizeof(params));
```

## Summary

**FullScreenPass** is a specialized graphics pass for full-screen rendering that provides:

### FullScreenPass
- Excellent cache locality (all members in single cache line)
- O(N) time complexity for creation (N = number of shader stages × defines)
- O(1) time complexity for execution
- Shared vertex buffer and VAO via SharedCache
- Triangle strip topology for efficient rendering (4 vertices = 2 triangles)
- Viewport mask support for multi-projection passes
- Automatic viewport and scissor state management
- Depth test disabled for full-screen rendering
- Flexible creation from file or description
- 48 bytes total size (excluding virtual table, reference count storage, and SharedData allocation)
- 8-byte natural alignment
- Inherits padding from BaseGraphicsPass

### SharedData
- Excellent cache locality (all members in single cache line)
- O(1) time complexity for creation
- Shared across multiple FullScreenPass instances
- Thread-safe access via SharedCache
- 24 bytes total size (excluding buffer and VAO allocations)
- 8-byte natural alignment
- No padding (perfectly aligned)

### Vertex Data
- Excellent cache locality (all vertices in single cache line)
- 16 bytes per vertex (8 bytes position + 8 bytes texcoord)
- 64 bytes total for 4 vertices
- Triangle strip topology
- Normalized device coordinates (-1 to 1)
- Texture coordinates (0 to 1)

The class implements an efficient and flexible full-screen rendering pass with excellent cache locality, constant-time operations, comprehensive shader program management, shared vertex data for memory efficiency, and support for multi-projection passes via viewport mask, providing a solid foundation for all full-screen rendering operations in the Falcor engine.
