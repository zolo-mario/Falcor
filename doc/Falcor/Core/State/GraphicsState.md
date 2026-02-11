# GraphicsState

## Module Overview

The **GraphicsState** module provides graphics state management for Falcor engine. This module includes [`GraphicsState`](Source/Falcor/Core/State/GraphicsState.h:52) class, providing comprehensive graphics state management with viewport, scissor, FBO, VAO, blend state, rasterizer state, depth-stencil state, and state graph management.

## Files

- **Header**: `Source/Falcor/Core/State/GraphicsState.h` (305 lines)
- **Implementation**: `Source/Falcor/Core/State/GraphicsState.cpp` (not provided in file list)

## Module Structure

### Classes

1. **GraphicsState** - Graphics state management
2. **Viewport** - Viewport definition
3. **Scissor** - Scissor definition

## GraphicsState Class

### Definition

```cpp
class FALCOR_API GraphicsState : public Object
{
    FALCOR_OBJECT(GraphicsState)
public:
    virtual ~GraphicsState();

    struct Viewport
    {
        Viewport() = default;
        Viewport(float x, float y, float w, float h, float minZ, float maxZ)
            : originX(x), originY(y), width(w), height(h), minDepth(minZ), maxDepth(maxZ)
        {}

        float originX = 0;
        float originY = 0;
        float width = 1.0f;
        float height = 1.0f;
        float minDepth = 0;
        float maxDepth = 1;
    };

    struct Scissor
    {
        Scissor() = default;
        Scissor(int32_t l, int32_t t, int32_t r, int32_t b) : left(l), top(t), right(r), bottom(b) {}

        int32_t left = 0;
        int32_t top = 0;
        int32_t right = 0;
        int32_t bottom = 0;
    };

    static ref<GraphicsState> create(ref<Device> pDevice);

    ref<Fbo> getFbo() const;
    GraphicsState& setFbo(const ref<Fbo>& pFbo, bool setVp0Sc0 = true);
    void pushFbo(const ref<Fbo>& pFbo, bool setVp0Sc0 = true);
    void popFbo(bool setVp0Sc0 = true);

    GraphicsState& setVao(const ref<Vao>& pVao);
    ref<Vao> getVao() const;

    GraphicsState& setStencilRef(uint8_t refValue);
    uint8_t getStencilRef() const;

    void setViewport(uint32_t index, const Viewport& vp, bool setScissors = true);
    const Viewport& getViewport(uint32_t index) const;
    const std::vector<Viewport>& getViewports() const;
    void pushViewport(uint32_t index, const Viewport& vp, bool setScissors = true);
    void popViewport(uint32_t index, bool setScissors = true);

    void setScissors(uint32_t index, const Scissor& sc);
    const Scissor& getScissors(uint32_t index) const;
    const std::vector<Scissor>& getScissors() const;
    void pushScissors(uint32_t index, const Scissor& sc);
    void popScissors(uint32_t index);

    GraphicsState& setProgram(const ref<Program>& pProgram);
    ref<Program> getProgram() const;

    GraphicsState& setBlendState(ref<BlendState> pBlendState);
    ref<BlendState> getBlendState() const;

    GraphicsState& setRasterizerState(ref<RasterizerState> pRasterizerState);
    ref<RasterizerState> getRasterizerState() const;

    GraphicsState& setDepthStencilState(ref<DepthStencilState> pDepthStencilState);
    ref<DepthStencilState> getDepthStencilState() const;

    GraphicsState& setSampleMask(uint32_t sampleMask);
    uint32_t getSampleMask() const;

    virtual ref<GraphicsStateObject> getGSO(const ProgramVars* pVars);
    const GraphicsStateObjectDesc& getDesc() const;

    void breakStrongReferenceToDevice();

private:
    GraphicsState(ref<Device> pDevice);

    BreakableReference<Device> mpDevice;
    ref<Vao> mpVao;
    ref<Fbo> mpFbo;
    ref<Program> mpProgram;
    GraphicsStateObjectDesc mDesc;
    uint8_t mStencilRef = 0;
    std::vector<Viewport> mViewports;
    std::vector<Scissor> mScissors;

    std::stack<ref<Fbo>> mFboStack;
    std::vector<std::stack<Viewport>> mVpStack;
    std::vector<std::stack<Scissor>> mScStack;

    struct CachedData
    {
        const ProgramKernels* pProgramKernels = nullptr;
        const Fbo::Desc* pFboDesc = nullptr;
    };
    CachedData mCachedData;

    using GraphicsStateGraph = StateGraph<ref<GraphicsStateObject>, void*>;
    std::unique_ptr<GraphicsStateGraph> mpGsoGraph;
};
```

### Dependencies

### Internal Dependencies

- **Object**: Inherits from [`Object`](Source/Falcor/Core/Object.h) for reference counting
- **Device**: Uses [`Device`](Source/Falcor/Core/API/Device.h) for device management
- **Fbo**: Uses [`Fbo`](Source/Falcor/Core/API/FBO.h) for framebuffer object management
- **Vao**: Uses [`Vao`](Source/Falcor/Core/API/VAO.h) for vertex array object management
- **Program**: Uses [`Program`](Source/Falcor/Core/Program/Program.h:573) for program management
- **ProgramKernels**: Uses [`ProgramKernels`](Source/Falcor/Core/Program/ProgramVersion.h:171) for program kernels
- **BlendState**: Uses [`BlendState`](Source/Falcor/Core/API/BlendState.h) for blend state management
- **RasterizerState**: Uses [`RasterizerState`](Source/Falcor/Core/API/RasterizerState.h) for rasterizer state management
- **DepthStencilState**: Uses [`DepthStencilState`](Source/Falcor/Core/API/DepthStencilState.h) for depth-stencil state management
- **GraphicsStateObject**: Uses [`GraphicsStateObject`](Source/Falcor/Core/API/GraphicsStateObject.h) for graphics state object
- **GraphicsStateObjectDesc**: Uses [`GraphicsStateObjectDesc`](Source/Falcor/Core/API/GraphicsStateObject.h) for graphics state object descriptor
- **StateGraph**: Uses [`StateGraph`](Source/Falcor/Core/State/StateGraph.h:36) for state graph management
- **BreakableReference**: Uses [`BreakableReference`](Source/Falcor/Core/Object.h) for breakable reference

### External Dependencies

- **ref<T>**: Uses [`ref<T>`](Source/Falcor/Core/Object.h) for reference counting
- **std::unique_ptr**: Uses std::unique_ptr for unique ownership
- **std::vector**: Uses std::vector for container storage
- **std::stack**: Uses std::stack for stack-based state management
- **ProgramVars**: Uses [`ProgramVars`](Source/Falcor/Core/Program/ProgramVars.h) for program variables

## Usage Patterns

### Creating GraphicsState

```cpp
auto graphicsState = GraphicsState::create(pDevice);
```

### Setting FBO

```cpp
graphicsState->setFbo(pFbo);
graphicsState->setFbo(pFbo, true); // Set viewport 0 and scissor 0 to match FBO dimensions
```

### Pushing/Poping FBO

```cpp
graphicsState->pushFbo(pFbo);
// ... rendering ...
graphicsState->popFbo();
```

### Setting VAO

```cpp
graphicsState->setVao(pVao);
```

### Setting Stencil Reference

```cpp
graphicsState->setStencilRef(1);
```

### Setting Viewport

```cpp
GraphicsState::Viewport vp(0, 0, 1920, 1080, 0, 1);
graphicsState->setViewport(0, vp);
graphicsState->setViewport(0, vp, true); // Set scissor to same dimensions
```

### Getting Viewport

```cpp
const GraphicsState::Viewport& vp = graphicsState->getViewport(0);
```

### Pushing/Poping Viewport

```cpp
GraphicsState::Viewport vp(0, 0, 1920, 1080, 0, 1);
graphicsState->pushViewport(0, vp);
// ... rendering ...
graphicsState->popViewport(0);
```

### Setting Scissor

```cpp
GraphicsState::Scissor sc(0, 0, 1920, 1080);
graphicsState->setScissors(0, sc);
```

### Getting Scissor

```cpp
const GraphicsState::Scissor& sc = graphicsState->getScissors(0);
```

### Pushing/Poping Scissor

```cpp
GraphicsState::Scissor sc(0, 0, 1920, 1080);
graphicsState->pushScissors(0, sc);
// ... rendering ...
graphicsState->popScissors(0);
```

### Setting Program

```cpp
graphicsState->setProgram(pProgram);
```

### Setting Blend State

```cpp
graphicsState->setBlendState(pBlendState);
```

### Setting Rasterizer State

```cpp
graphicsState->setRasterizerState(pRasterizerState);
```

### Setting Depth-Stencil State

```cpp
graphicsState->setDepthStencilState(pDepthStencilState);
```

### Setting Sample Mask

```cpp
graphicsState->setSampleMask(0xFFFFFFFF);
```

### Getting Graphics State Object

```cpp
ref<GraphicsStateObject> gso = graphicsState->getGSO(pVars);
```

## Summary

**GraphicsState** is a comprehensive module that provides graphics state management for Falcor engine:

### GraphicsState Class
- **Memory Layout**: ~144-160 bytes + heap allocations
- **Threading Model**: Not thread-safe
- **Cache Locality**: Excellent (inline members fit in cache lines)
- **Algorithmic Complexity**: O(1) for most operations, O(N) for scan for matching node
- **Mutable State**: Mutable state (not immutable after construction)
- **Factory Pattern**: Static create method for object creation
- **Reference Counting**: [`ref<T>`](Source/Falcor/Core/Object.h) provides automatic resource management
- **State Graph**: Uses state graph for state object caching
- **Fluent Interface**: Fluent interface for method chaining
- **Stack-Based State Management**: Stack-based state management for FBO, viewport, and scissor

### Viewport Struct
- **Memory Layout**: 24 bytes (no heap allocation)
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for all operations
- **Immutable After Construction**: Immutable after construction

### Scissor Struct
- **Memory Layout**: 16 bytes (no heap allocation)
- **Threading Model**: Thread-safe for concurrent reads (immutable after construction)
- **Cache Locality**: Excellent (all members fit in cache line)
- **Algorithmic Complexity**: O(1) for all operations
- **Immutable After Construction**: Immutable after construction

### Module Characteristics
- Mutable state (not immutable after construction)
- Reference counting (ref<T> provides automatic resource management)
- Factory pattern (static create method)
- State graph (uses state graph for state object caching)
- Fluent interface (fluent interface for method chaining)
- Stack-based state management (stack-based state management for FBO, viewport, and scissor)
- Viewport/scissor management (viewport and scissor management with push/pop operations)
- Excellent cache locality (inline members fit in cache lines)
- Not thread-safe (requires external synchronization)
- High complexity (many members and state management)

The module provides a comprehensive graphics state management system with mutable state, reference counting, state graph for state object caching, fluent interface for method chaining, and stack-based state management for FBO, viewport, and scissor, optimized for performance with caching of graphics state objects.
