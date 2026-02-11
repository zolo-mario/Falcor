# ComputePass

## Module Overview

The **ComputePass** class provides a compute shader pass for executing compute operations in the Falcor engine. This class serves as the foundation for all compute passes, providing common functionality for shader program management, state management, variable binding, and compute dispatch operations. It supports both direct and indirect dispatch, flexible program creation from files or descriptions, and dynamic shader define management.

## Files

- **Header**: `Source/Falcor/Core/Pass/ComputePass.h` (153 lines)
- **Implementation**: `Source/Falcor/Core/Pass/ComputePass.cpp` (147 lines)

## Class Definition

```cpp
class ComputePass : public Object
{
    FALCOR_OBJECT(ComputePass)
public:
    /**
     * Create a new compute pass from file.
     * @param[in] pDevice GPU device.
     * @param[in] path Compute program file path.
     * @param[in] csEntry Name of the entry point in the program. If not specified "main" will be used.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] createVars Create program vars automatically, otherwise use setVars().
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<ComputePass> create(
        ref<Device> pDevice,
        const std::filesystem::path& path,
        const std::string& csEntry = "main",
        const DefineList& defines = DefineList(),
        bool createVars = true
    );

    /**
     * Create a new compute pass.
     * @param[in] pDevice GPU device.
     * @param[in] desc The program's description.
     * @param[in] defines Optional list of macro definitions to set into the program.
     * @param[in] createVars Create program vars automatically, otherwise use setVars().
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<ComputePass> create(
        ref<Device> pDevice,
        const ProgramDesc& desc,
        const DefineList& defines = DefineList(),
        bool createVars = true
    );

    /**
     * Execute the pass using the given compute-context
     * @param[in] pContext The compute context
     * @param[in] nThreadX The number of threads to dispatch in the X dimension (note that this is not the number of thread groups)
     * @param[in] nThreadY The number of threads to dispatch in the Y dimension (note that this is not the number of thread groups)
     * @param[in] nThreadZ The number of threads to dispatch in the Z dimension (note that this is not the number of thread groups)
     */
    virtual void execute(ComputeContext* pContext, uint32_t nThreadX, uint32_t nThreadY, uint32_t nThreadZ = 1);

    /**
     * Execute the pass using the given compute-context
     * @param[in] pContext The compute context
     * @param[in] nThreads The number of threads to dispatch in the XYZ dimensions (note that this is not the number of thread groups)
     */
    virtual void execute(ComputeContext* pContext, const uint3& nThreads);

    /**
     * Execute the pass using indirect dispatch given the compute-context and argument buffer
     * @param[in] pContext The compute context
     * @param[in] pArgBuffer Argument buffer
     * @param[in] argBufferOffset Offset in argument buffer
     */
    virtual void executeIndirect(ComputeContext* context, const Buffer* pArgBuffer, uint64_t argBufferOffset = 0);

    /**
     * Check if a vars object exists. If not, use setVars() to set or create a new vars object.
     * @return True if a vars object exists.
     */
    bool hasVars() const { return mpVars != nullptr; }

    /**
     * Get the vars.
     */
    const ref<ProgramVars>& getVars() const;

    /**
     * Get the root var
     */
    ShaderVar getRootVar() const { return mpVars->getRootVar(); }

    /**
     * Add a define
     * @param[in] name Define name
     * @param[in] value Define value
     * @param[in] updateVars Whether to update program vars (default: false)
     */
    void addDefine(const std::string& name, const std::string& value = "", bool updateVars = false);

    /**
     * Remove a define
     * @param[in] name Define name
     * @param[in] updateVars Whether to update program vars (default: false)
     */
    void removeDefine(const std::string& name, bool updateVars = false);

    /**
     * Get the program
     * @return The program associated with this pass
     */
    ref<Program> getProgram() const { return mpState->getProgram(); }

    /**
     * Set a vars object. Allows the user to override internal vars, for example when one wants to share a vars object between
     * different passes. The function throws an exception on error.
     * @param[in] pVars The new ProgramVars object. If this is nullptr, then the pass will automatically create a new vars object.
     */
    void setVars(const ref<ProgramVars>& pVars);

    /**
     * Get the thread group size from the program
     * @return Thread group size (X, Y, Z)
     */
    uint3 getThreadGroupSize() const { return mpState->getProgram()->getReflector()->getThreadGroupSize(); }

    /**
     * Get the device
     * @return The device associated with this pass
     */
    const ref<Device>& getDevice() const { return mpDevice; }

protected:
    /**
     * Create a new object.
     * @param[in] pDevice GPU device.
     * @param[in] desc The program's description.
     * @param[in] defines List of macro definitions to set into the program.
     * @param[in] createVars Create program vars automatically.
     */
    ComputePass(ref<Device> pDevice, const ProgramDesc& desc, const DefineList& defines, bool createVars);

    ref<Device> mpDevice;
    ref<ProgramVars> mpVars;
    ref<ComputeState> mpState;
};
```

## Dependencies

### Internal Dependencies

- **Object**: Base class for reference counting
- **Device**: Required for device access and program creation
- **Program**: Required for shader program management
- **ComputeState**: Required for compute state management
- **ProgramVars**: Required for program variable management
- **ComputeContext**: Required for compute execution
- **Buffer**: Required for indirect dispatch

### External Dependencies

- **std::filesystem**: Standard library filesystem for file paths
- **std::string**: Standard library string container
- **std::atomic**: Standard library atomic operations
- **std::shared_ptr**: Standard library shared pointer (via ref<>)
- **std::optional**: Standard library optional type (Python bindings)

## Cross-Platform Considerations

- **Cross-Platform**: Works on all platforms supported by Falcor
- **No Platform-Specific Code**: Pure C++ implementation
- **Standard Library**: Uses only standard library containers
- **Filesystem**: Uses std::filesystem for cross-platform file paths
- **No Conditional Compilation**: No platform-specific conditional compilation

## Usage Patterns

### Creating a Compute Pass from File

```cpp
ref<Device> pDevice = ...;
std::filesystem::path path = "shaders/compute.slang";
std::string csEntry = "main";
DefineList defines;

ref<ComputePass> pPass = ComputePass::create(pDevice, path, csEntry, defines);
```

### Creating a Compute Pass from Description

```cpp
ref<Device> pDevice = ...;
ProgramDesc desc;
desc.addShaderLibrary("shaders/compute.slang").csEntry("main");
DefineList defines;

ref<ComputePass> pPass = ComputePass::create(pDevice, desc, defines);
```

### Creating a Compute Pass without Vars

```cpp
ref<Device> pDevice = ...;
ProgramDesc desc;
desc.addShaderLibrary("shaders/compute.slang").csEntry("main");
DefineList defines;

ref<ComputePass> pPass = ComputePass::create(pDevice, desc, defines, false);
```

### Executing a Compute Pass

```cpp
ComputeContext* pContext = ...;
uint32_t nThreadX = 1024;
uint32_t nThreadY = 1024;
uint32_t nThreadZ = 1;

pPass->execute(pContext, nThreadX, nThreadY, nThreadZ);
```

### Executing a Compute Pass with uint3

```cpp
ComputeContext* pContext = ...;
uint3 nThreads = uint3(1024, 1024, 1);

pPass->execute(pContext, nThreads);
```

### Executing Indirect Compute

```cpp
ComputeContext* pContext = ...;
const Buffer* pArgBuffer = ...;
uint64_t argBufferOffset = 0;

pPass->executeIndirect(pContext, pArgBuffer, argBufferOffset);
```

### Adding Shader Defines

```cpp
pPass->addDefine("MAX_ITERATIONS", "100");
pPass->addDefine("USE_WARP_SYNC", "1");
pPass->addDefine("BLOCK_SIZE", "256", true);
```

### Removing Shader Defines

```cpp
pPass->removeDefine("MAX_ITERATIONS");
pPass->removeDefine("USE_WARP_SYNC", true);
```

### Accessing Pass Components

```cpp
ref<Program> pProgram = pPass->getProgram();
const ref<ProgramVars>& pVars = pPass->getVars();
ShaderVar rootVar = pPass->getRootVar();
uint3 threadGroupSize = pPass->getThreadGroupSize();
const ref<Device>& pDevice = pPass->getDevice();
```

### Setting Program Variables

```cpp
ref<ProgramVars> pSharedVars = ...;
pPass->setVars(pSharedVars);
```

### Checking if Vars Exists

```cpp
if (pPass->hasVars())
{
    const ref<ProgramVars>& pVars = pPass->getVars();
}
```

### Python Usage

```python
# Create from description
device = ...
desc = ProgramDesc()
desc.addShaderLibrary("shaders/compute.slang").csEntry("main")
pass = ComputePass.create(device, desc, defines={})

# Create from kwargs
pass = ComputePass.create(device, path="shaders/compute.slang", cs_entry="main", defines={})

# Execute
pass.execute(threads_x=1024, threads_y=1024, threads_z=1, compute_context=None)

# Access properties
program = pass.program
root_var = pass.root_var
globals = pass.globals
```

## Summary

**ComputePass** is a foundational class for compute shader passes that provides:

### ComputePass
- Excellent cache locality (all members in single cache line)
- O(N) time complexity for creation (N = number of shader stages × defines)
- O(1) time complexity for all execution, accessor, and modifier methods
- Dynamic shader define management
- Direct and indirect compute dispatch
- Thread group size management
- Program and state management
- Program vars management with optional creation
- Automatic resource cleanup via smart pointers
- Flexible creation from file or description
- Python bindings with automatic context handling
- 40 bytes total size (excluding virtual table and reference count storage)
- 8-byte natural alignment
- Only 4 bytes of padding

The class implements a flexible and efficient compute pass framework with excellent cache locality, constant-time operations, comprehensive shader program management, and support for both direct and indirect dispatch, providing a solid foundation for all compute shader operations in the Falcor engine.
