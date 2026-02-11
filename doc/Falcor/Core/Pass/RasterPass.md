# RasterPass

## Module Overview

The **RasterPass** class provides a rasterization pass for rendering geometry in the Falcor engine. This class extends BaseGraphicsPass to provide specialized functionality for rasterization operations, including both ordered (non-indexed) and indexed draw calls. It supports flexible program creation from files or descriptions, with optional default vertex shader support.

## Files

- **Header**: `Source/Falcor/Core/Pass/RasterPass.h` (87 lines)
- **Implementation**: `Source/Falcor/Core/Pass/RasterPass.cpp` (64 lines)

## Class Definition

```cpp
class RasterPass : public BaseGraphicsPass
{
public:
    /**
     * Create a new object.
     * @param[in] pDevice GPU device.
     * @param[in] path Program file path.
     * @param[in] vsEntry Vertex shader entry point. If this string is empty (""), it will use a default vertex shader which transforms and
     * outputs all default vertex attributes.
     * @param[in] psEntry Pixel shader entry point
     * @param[in] programDefines Optional list of macro definitions to set into the program. The macro definitions will be set on all shader
     * stages.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<RasterPass> create(
        ref<Device> pDevice,
        const std::filesystem::path& path,
        const std::string& vsEntry,
        const std::string& psEntry,
        const DefineList& defines = DefineList()
    );

    /**
     * Create a new object.
     * @param[in] pDevice GPU device.
     * @param[in] progDesc The program description.
     * @param[in] programDefines Optional list of macro definitions to set into the program. The macro definitions will be set on all shader
     * stages.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<RasterPass> create(ref<Device> pDevice, const ProgramDesc& desc, const DefineList& defines = DefineList());

    /**
     * Ordered draw call.
     * @param[in] pRenderContext The render context.
     * @param[in] vertexCount Number of vertices to draw
     * @param[in] startVertexLocation The location of the first vertex to read from vertex buffers (offset in vertices)
     */
    void draw(RenderContext* pRenderContext, uint32_t vertexCount, uint32_t startVertexLocation);

    /**
     * Indexed draw call.
     * @param[in] pRenderContext The render context.
     * @param[in] indexCount Number of indices to draw
     * @param[in] startIndexLocation The location of the first index to read from the index buffer (offset in indices)
     * @param[in] baseVertexLocation A value which is added to each index before reading a vertex from vertex buffer
     */
    void drawIndexed(RenderContext* pRenderContext, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);

protected:
    RasterPass(ref<Device> pDevice, const ProgramDesc& progDesc, const DefineList& programDefines);
};
```

## Dependencies

### Internal Dependencies

- **BaseGraphicsPass**: Base class for graphics pass functionality
- **Object**: Base class for reference counting (inherited from BaseGraphicsPass)
- **Device**: Required for device access and program creation (inherited from BaseGraphicsPass)
- **Program**: Required for shader program management (inherited from BaseGraphicsPass)
- **GraphicsState**: Required for graphics state management (inherited from BaseGraphicsPass)
- **ProgramVars**: Required for program variable management (inherited from BaseGraphicsPass)
- **RenderContext**: Required for rendering execution

### External Dependencies

- **std::filesystem**: Standard library filesystem for file paths
- **std::string**: Standard library string container
- **std::atomic**: Standard library atomic operations (inherited from Object)
- **std::shared_ptr**: Standard library shared pointer (via ref<>)

## Cross-Platform Considerations

- **Cross-Platform**: Works on all platforms supported by Falcor
- **No Platform-Specific Code**: Pure C++ implementation
- **Standard Library**: Uses only standard library containers
- **Filesystem**: Uses std::filesystem for cross-platform file paths
- **No Conditional Compilation**: No platform-specific conditional compilation

## Usage Patterns

### Creating a Raster Pass from File

```cpp
ref<Device> pDevice = ...;
std::filesystem::path path = "shaders/raster.slang";
std::string vsEntry = "vsMain";
std::string psEntry = "psMain";
DefineList defines;

ref<RasterPass> pPass = RasterPass::create(pDevice, path, vsEntry, psEntry, defines);
```

### Creating a Raster Pass with Default Vertex Shader

```cpp
ref<Device> pDevice = ...;
std::filesystem::path path = "shaders/raster.slang";
std::string vsEntry = ""; // Use default vertex shader
std::string psEntry = "psMain";
DefineList defines;

ref<RasterPass> pPass = RasterPass::create(pDevice, path, vsEntry, psEntry, defines);
```

### Creating a Raster Pass from Description

```cpp
ref<Device> pDevice = ...;
ProgramDesc desc;
desc.addShaderLibrary("shaders/raster.slang").vsEntry("vsMain").psEntry("psMain");
DefineList defines;

ref<RasterPass> pPass = RasterPass::create(pDevice, desc, defines);
```

### Drawing with Ordered Draw Call

```cpp
RenderContext* pRenderContext = ...;
uint32_t vertexCount = 1000;
uint32_t startVertexLocation = 0;

pPass->draw(pRenderContext, vertexCount, startVertexLocation);
```

### Drawing with Indexed Draw Call

```cpp
RenderContext* pRenderContext = ...;
uint32_t indexCount = 1500;
uint32_t startIndexLocation = 0;
int32_t baseVertexLocation = 0;

pPass->drawIndexed(pRenderContext, indexCount, startIndexLocation, baseVertexLocation);
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
pPass->addDefine("MAX_LIGHTS", "16");
pPass->addDefine("ENABLE_SHADOWS", "1");
pPass->addDefine("USE_PCF", "1", true);
```

### Setting Program Variables

```cpp
ref<ProgramVars> pSharedVars = ...;
pPass->setVars(pSharedVars);
```

### Setting Graphics State

```cpp
ref<GraphicsState> pState = pPass->getState();

// Set VAO
pState->setVao(pVao);

// Set FBO
pState->setFbo(pFbo);

// Set blend state
pState->setBlendState(pBlendState);

// Set rasterizer state
pState->setRasterizerState(pRasterizerState);

// Set depth stencil state
pState->setDepthStencilState(pDepthStencilState);
```

## Summary

**RasterPass** is a specialized graphics pass for rasterization that provides:

### RasterPass
- Excellent cache locality (all members in single cache line, inherited from BaseGraphicsPass)
- O(N) time complexity for creation (N = number of shader stages × defines)
- O(1) time complexity for draw operations
- Ordered draw call for non-indexed rendering
- Indexed draw call for indexed rendering
- Vertex and index offset support
- Base vertex offset support
- Default vertex shader support
- Flexible creation from file or description
- No additional member variables (pure wrapper around BaseGraphicsPass)
- ~40 bytes total size (excluding virtual table and reference count storage)
- 8-byte natural alignment (inherited from BaseGraphicsPass)
- Only 4 bytes of padding (inherited from BaseGraphicsPass)

The class implements a simple and efficient rasterization pass with excellent cache locality, constant-time operations, comprehensive shader program management, and support for both ordered and indexed draw calls, providing a minimal wrapper around BaseGraphicsPass for rasterization operations in the Falcor engine.
